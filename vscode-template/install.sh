#!/usr/bin/env bash
# ==============================================================================
# Script: install.sh
# Purpose: Import portable VS Code settings, C snippets, and Git preferences into
#          any fresh VS Code environment on macOS or Linux.
# ==============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

echo "=== Portable VS Code & Development Environment Installer ==="

# 1. Detect VS Code User Configuration Directory
if [[ "$OSTYPE" == "darwin"* ]]; then
  USER_CONFIG_DIR="${HOME}/Library/Application Support/Code/User"
elif [[ "$OSTYPE" == "linux"* ]]; then
  USER_CONFIG_DIR="${HOME}/.config/Code/User"
else
  echo "[WARNING] Unknown OS: $OSTYPE. Defaulting to Linux ~/.config/Code/User."
  USER_CONFIG_DIR="${HOME}/.config/Code/User"
fi

SNIPPETS_DIR="${USER_CONFIG_DIR}/snippets"
mkdir -p "${USER_CONFIG_DIR}"
mkdir -p "${SNIPPETS_DIR}"

# 2. Install VS Code Settings
SETTINGS_SRC="${REPO_ROOT}/.vscode/settings.json"
SETTINGS_DST="${USER_CONFIG_DIR}/settings.json"

if [[ -f "${SETTINGS_SRC}" ]]; then
  if [[ -f "${SETTINGS_DST}" ]]; then
    cp "${SETTINGS_DST}" "${SETTINGS_DST}.bak"
    echo "  [BACKUP] Existing settings backed up to: ${SETTINGS_DST}.bak"
  fi
  cp "${SETTINGS_SRC}" "${SETTINGS_DST}"
  echo "  [OK] Installed settings -> ${SETTINGS_DST}"
fi

# 3. Install VS Code C Snippets
SNIPPETS_SRC="${REPO_ROOT}/.vscode/c.code-snippets"
SNIPPETS_DST="${SNIPPETS_DIR}/c.json"

if [[ -f "${SNIPPETS_SRC}" ]]; then
  if [[ -f "${SNIPPETS_DST}" ]]; then
    cp "${SNIPPETS_DST}" "${SNIPPETS_DST}.bak"
    echo "  [BACKUP] Existing snippets backed up to: ${SNIPPETS_DST}.bak"
  fi
  cp "${SNIPPETS_SRC}" "${SNIPPETS_DST}"
  echo "  [OK] Installed C snippets -> ${SNIPPETS_DST}"
fi

# 4. Configure Local Git Commit Template
if [[ -f "${REPO_ROOT}/.gitmessage" ]]; then
  git -C "${REPO_ROOT}" config commit.template .gitmessage
  echo "  [OK] Configured Git commit.template -> .gitmessage"
fi

# 5. Configure Local Git Hooks (pre-commit & commit-msg)
HOOKS_DIR="${REPO_ROOT}/.git/hooks"
if [[ -d "${HOOKS_DIR}" ]]; then
  if [[ -f "${REPO_ROOT}/scripts/git-hooks/commit-msg" ]]; then
    cp "${REPO_ROOT}/scripts/git-hooks/commit-msg" "${HOOKS_DIR}/commit-msg"
    chmod +x "${HOOKS_DIR}/commit-msg"
    echo "  [OK] Installed commit-msg hook -> ${HOOKS_DIR}/commit-msg"
  fi
  if [[ -f "${REPO_ROOT}/scripts/git-hooks/pre-commit" ]]; then
    cp "${REPO_ROOT}/scripts/git-hooks/pre-commit" "${HOOKS_DIR}/pre-commit"
    chmod +x "${HOOKS_DIR}/pre-commit"
    echo "  [OK] Installed pre-commit hook -> ${HOOKS_DIR}/pre-commit"
  fi
fi

echo "=== Installation Complete! ==="
echo "Restart or reload VS Code to activate Allman braces, automatic indentation, and C snippets."
