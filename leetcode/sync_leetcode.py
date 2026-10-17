#!/usr/bin/env python3
"""
Sync accepted LeetCode submissions via LeetCode's GraphQL API and commit each
solution using its original submission timestamp (GIT_AUTHOR_DATE / GIT_COMMITTER_DATE).

Usage:
    export LEETCODE_SESSION="<your-session-cookie>"
    python3 sync_leetcode.py
"""

import argparse
import datetime
import json
import os
import re
import shutil
import subprocess
import sys
import time
import urllib.request
from pathlib import Path

LEETCODE_GRAPHQL_URL = "https://leetcode.com/graphql"

LANG_EXTENSIONS = {
    "c": "c",
    "cpp": "cpp",
    "python": "py",
    "python3": "py",
    "swift": "swift",
    "java": "java",
    "javascript": "js",
    "typescript": "ts",
    "golang": "go",
    "rust": "rs",
    "bash": "sh",
    "mysql": "sql",
}

SUBMISSION_LIST_QUERY = """
query submissionList($offset: Int!, $limit: Int!) {
  submissionList(offset: $offset, limit: $limit) {
    hasNext
    submissions {
      id
      title
      titleSlug
      timestamp
      statusDisplay
      lang
    }
  }
}
"""

SUBMISSION_DETAIL_QUERY = """
query submissionDetails($submissionId: Int!) {
  submissionDetails(submissionId: $submissionId) {
    code
    timestamp
    statusCode
    question {
      questionFrontendId
      title
      titleSlug
      difficulty
    }
  }
}
"""


def format_c_allman(code: str) -> str:
    """Normalize CRLF -> LF, enforce if(x)/for(...)/while(x)/switch(x), and Allman braces."""
    code = code.replace("\r\n", "\n").replace("\r", "\n")
    # Remove space between control keyword and '(' and immediately after '('
    code = re.sub(r"\b(if|for|while|switch)\s*\(\s*", r"\1(", code)
    lines = code.split("\n")
    out_lines = []

    def strip_ctrl_closing_space(s: str) -> str:
        if re.match(r"^(?:else\s+)?(?:if|for|while|switch)\(", s):
            return re.sub(r"\s+\)$", ")", s)
        return s

    for line in lines:
        stripped = line.lstrip()
        indent = line[: len(line) - len(stripped)]
        # Convert "} else {" or "} else if(...) {"
        m_else_brace = re.match(r"^\}\s*(else(?:\s+if\(.*\))?)\s*\{\s*$", stripped)
        if m_else_brace:
            out_lines.append(f"{indent}}}")
            out_lines.append(f"{indent}{strip_ctrl_closing_space(m_else_brace.group(1))}")
            out_lines.append(f"{indent}{{")
            continue
        m_else_only = re.match(r"^\}\s*(else\b.*)$", stripped)
        if m_else_only:
            out_lines.append(f"{indent}}}")
            stripped = m_else_only.group(1)
        # Move trailing '{' on non-comment, non-struct-initializer lines to its own line
        if (
            stripped.endswith("{")
            and stripped != "{"
            and not stripped.startswith("//")
            and not stripped.startswith("/*")
            and not stripped.startswith("*")
            and "=" not in stripped
        ):
            before = strip_ctrl_closing_space(stripped[:-1].rstrip())
            out_lines.append(f"{indent}{before}")
            out_lines.append(f"{indent}{{")
        else:
            stripped = strip_ctrl_closing_space(stripped)
            out_lines.append(f"{indent}{stripped}" if stripped else "")
    return "\n".join(out_lines).rstrip() + "\n"


def graphql_request(session_cookie: str, query: str, variables: dict) -> dict:
    payload = json.dumps({"query": query, "variables": variables}).encode("utf-8")
    req = urllib.request.Request(
        LEETCODE_GRAPHQL_URL,
        data=payload,
        headers={
            "Content-Type": "application/json",
            "Cookie": f"LEETCODE_SESSION={session_cookie}",
            "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7)",
            "Referer": "https://leetcode.com",
        },
        method="POST",
    )
    with urllib.request.urlopen(req, timeout=20) as resp:
        return json.loads(resp.read().decode("utf-8"))


def fetch_accepted_submissions(session_cookie: str, max_submissions: int = 500) -> list:
    accepted = []
    offset = 0
    page_size = 20

    while offset < max_submissions:
        data = graphql_request(
            session_cookie,
            SUBMISSION_LIST_QUERY,
            {"offset": offset, "limit": page_size},
        )
        sub_list = (data.get("data") or {}).get("submissionList")
        if not sub_list:
            break

        submissions = sub_list.get("submissions") or []
        for sub in submissions:
            if sub.get("statusDisplay") == "Accepted":
                accepted.append(sub)

        if not sub_list.get("hasNext") or not submissions:
            break

        offset += page_size
        time.sleep(1.0)

    accepted.sort(key=lambda s: int(s["timestamp"]))
    return accepted


def main():
    parser = argparse.ArgumentParser(description="Sync accepted LeetCode submissions with backdated git commits.")
    parser.add_argument("--dry-run", action="store_true", help="Write solution files without committing to git.")
    parser.add_argument("--limit", type=int, default=500, help="Max recent submissions to scan (default: 500).")
    args = parser.parse_args()

    session_cookie = os.environ.get("LEETCODE_SESSION", "").strip()
    if not session_cookie:
        default_cookie_file = Path(os.path.expanduser("~/.gemini/config/.leetcode_session"))
        if default_cookie_file.is_file():
            session_cookie = default_cookie_file.read_text(encoding="utf-8").strip()
    if not session_cookie:
        print("Error: LEETCODE_SESSION environment variable is not set.", file=sys.stderr)
        print("Export your cookie from leetcode.com (or save to ~/.gemini/config/.leetcode_session) and re-run:", file=sys.stderr)
        print("  export LEETCODE_SESSION='...'", file=sys.stderr)
        sys.exit(1)

    leetcode_dir = Path(__file__).resolve().parent
    repo_root = leetcode_dir.parent
    state_file = leetcode_dir / ".synced_submissions.json"

    synced_ids = set()
    if state_file.exists():
        synced_ids = set(json.loads(state_file.read_text()))

    print("Fetching accepted submissions from LeetCode...")
    accepted = fetch_accepted_submissions(session_cookie, max_submissions=args.limit)
    print(f"Found {len(accepted)} accepted submissions.")

    cf_bin = (
        shutil.which("clang-format")
        or ("/opt/homebrew/bin/clang-format" if Path("/opt/homebrew/bin/clang-format").exists() else None)
    )

    new_count = 0
    for sub in accepted:
        sub_id = str(sub["id"])
        if sub_id in synced_ids:
            continue

        details_resp = graphql_request(
            session_cookie,
            SUBMISSION_DETAIL_QUERY,
            {"submissionId": int(sub_id)},
        )
        details = (details_resp.get("data") or {}).get("submissionDetails")
        if not details or not details.get("code"):
            continue

        question = details.get("question") or {}
        frontend_id = question.get("questionFrontendId", "0000").zfill(4)
        difficulty = question.get("difficulty", "")
        slug = sub["titleSlug"]
        title = sub["title"]
        lang = sub["lang"]
        ext = LANG_EXTENSIONS.get(lang, "txt")
        ts = int(sub["timestamp"])
        iso_date = datetime.datetime.fromtimestamp(ts, tz=datetime.timezone.utc).astimezone().strftime("%Y-%m-%d %H:%M:%S %z")

        problem_dir = leetcode_dir / f"{frontend_id}-{slug}"
        problem_dir.mkdir(parents=True, exist_ok=True)
        sol_file = problem_dir / f"solution.{ext}"

        header = (
            f"/* ============================================================================\n"
            f" * LeetCode #{int(frontend_id)}: {title} ({difficulty})\n"
            f" * URL:      https://leetcode.com/problems/{slug}/\n"
            f" * Language: {lang.upper()}\n"
            f" * ============================================================================ */\n\n"
        )
        body = details["code"]
        if ext in ("c", "cpp"):
            body = format_c_allman(body)
        sol_file.write_text(header + body, encoding="utf-8")
        if cf_bin and ext in ("c", "cpp"):
            subprocess.run([cf_bin, "-i", "--style=file", str(sol_file)], cwd=repo_root, check=True)

        if not args.dry_run:
            rel_path = sol_file.relative_to(repo_root)
            subprocess.run(["git", "add", str(rel_path)], cwd=repo_root, check=True)
            status = subprocess.run(
                ["git", "diff", "--cached", "--quiet"],
                cwd=repo_root,
            )
            if status.returncode != 0:
                env = os.environ.copy()
                env["GIT_AUTHOR_DATE"] = iso_date
                env["GIT_COMMITTER_DATE"] = iso_date
                msg = f"Solve LeetCode {int(frontend_id)}. {title} ({lang.upper()})"
                subprocess.run(["git", "commit", "-m", msg], cwd=repo_root, env=env, check=True)
                print(f"Committed [{iso_date}] {msg}")

        synced_ids.add(sub_id)
        state_file.write_text(json.dumps(sorted(synced_ids), indent=2) + "\n")
        new_count += 1
        time.sleep(1.0)

    print(f"Sync complete ({new_count} new submissions processed).")


if __name__ == "__main__":
    main()
