/**
 * @file dma_ring_buffer_driver.c
 * @brief Coherent Direct Memory Access (DMA) Circular Descriptor Ring Controller.
 *
 * Concepts Demonstrated:
 * 1. Hardware DMA descriptor ring layout and ownership handoff bit protocols.
 * 2. Memory barrier ordering (acquire/release semantics) across CPU and DMA peripheral.
 * 3. Producer-consumer circular ring index advancement and wrap-around.
 * 4. Hardware Doorbell notification and completion reaping.
 *
 * Academic Attribution:
 * Exercise design based on operating systems I/O architecture and network device drivers
 * (CS:APP Chapter 6, OSTEP Chapter 36, and Linux kernel Documentation/networking/device_drivers).
 * Retained strictly for independent educational self-study.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==============================================================================
 * DMA Descriptor & Controller Definitions
 * ============================================================================== */

/* Descriptor Ownership and Control Flags */
#define DMA_DESC_FLAG_EOP (1U << 0)    /* End of Packet */
#define DMA_DESC_FLAG_IOC (1U << 1)    /* Interrupt on Completion */
#define DMA_DESC_STATUS_OWN (1U << 15) /* 1 = Owned by Device; 0 = Owned by CPU */

/**
 * Hardware DMA Descriptor (16-byte aligned hardware format)
 */
typedef struct
{
  uint64_t buffer_addr; /* Memory address of data payload buffer */
  uint32_t length;      /* Length of data buffer in bytes */
  uint16_t flags;       /* Control flags (EOP, IOC) */
  uint16_t status;      /* Status and ownership bits */
} dma_descriptor_t;

/**
 * DMA Ring Controller State
 */
typedef struct
{
  dma_descriptor_t *ring;     /* Base pointer to descriptor array in coherent memory */
  size_t ring_size;           /* Number of descriptors in ring */
  uint32_t head;              /* Producer index: next descriptor to submit */
  uint32_t tail;              /* Consumer index: next descriptor to reap */
  volatile uint32_t doorbell; /* Hardware doorbell register for CPU-to-device kicks */
} dma_controller_t;

/* ==============================================================================
 * Driver Function Declarations & Skeletons
 * ============================================================================== */

/**
 * Initializes the DMA circular descriptor ring and controller state.
 *
 * Contract & Requirements:
 * - Validates controller pointer, descriptor memory pointer, and ring size (must be > 0).
 * - Initializes all descriptor entries to host ownership with cleared buffers and flags.
 * - Resets head, tail, and doorbell counters to zero.
 * - Returns 0 on success, or -1 on invalid arguments.
 *
 * @param ctrl Pointer to DMA controller state structure.
 * @param desc_ring_mem Pointer to pre-allocated descriptor ring memory.
 * @param ring_size Number of descriptors in the ring.
 * @return 0 on success, or -1 on error.
 */
int dma_ring_init(dma_controller_t *ctrl, void *desc_ring_mem, size_t ring_size)
{
  /* TODO: Author Implementation */
  if((ctrl == NULL) || (desc_ring_mem == NULL) || (ring_size <= 0))
  {
    return -1;
  }

  ctrl->head = 0;
  ctrl->tail = 0;
  ctrl->doorbell = 0;
  ctrl->ring = desc_ring_mem;
  ctrl->ring_size = ring_size;

  size_t count = 0;

  /*
   * // AI Review Feedback: Pointer indexing with void* arithmetic:
   * Notice that `desc_ring_mem` is passed as `void *`. In GNU C, `&desc_ring_mem[count]` computes
   * `(char *)desc_ring_mem + count` (byte-level offset) rather than indexing descriptors by
   * `sizeof(dma_descriptor_t)`. The test harness passed because `memset(raw_ring, 0, ...)`
   * zero-initialized the memory prior to calling init. In production drivers, indexing should
   * use the typed pointer `ctrl->ring[count]` or `((dma_descriptor_t *)desc_ring_mem)[count]`.
   */
  while(count != ring_size)
  {
    dma_descriptor_t *ptr = (dma_descriptor_t *)&desc_ring_mem[count];
    ptr->flags = 0x00;
    ptr->status = 0x00;
    count++;
  }
  return 0;
}

/**
 * Enqueues a packet buffer into the DMA descriptor ring and notifies hardware.
 *
 * Contract & Requirements:
 * - Validates controller pointer, buffer pointer, and non-zero length.
 * - Checks descriptor ring capacity; if ring is full, returns -1.
 * - Populates descriptor with buffer address, length, and transfer flags.
 * - Enforces memory ordering before handing descriptor ownership to the device.
 * - Advances producer head index with circular wrap-around.
 * - Increments doorbell register to notify device hardware.
 * - Returns 0 on success, or -1 on error / ring full.
 *
 * @param ctrl Pointer to DMA controller state structure.
 * @param packet_buffer Pointer to packet data payload.
 * @param length Length of packet payload in bytes.
 * @return 0 on success, or -1 on error or ring full.
 */
int dma_submit_tx_packet(dma_controller_t *ctrl, void *packet_buffer, size_t length)
{
  /* TODO: Author Implementation */
  if((ctrl == NULL) || (packet_buffer == NULL) || (length <= 0))
  {
    return -1;
  }

  /*
   * // AI Review Feedback: Concurrency & Producer Reentrancy:
   * In real-world network and storage drivers, `ctrl->head` is updated by packet submitters, while
   * `ctrl->tail` is updated by completion reapers (frequently executing in an ISR or bottom-half
   * tasklet). If multiple worker threads or kernel contexts submit packets concurrently, a spinlock
   * (e.g., `spin_lock_irqsave`) or lock-free atomic head reservation is required around
   * `dma_submit_tx_packet()` to prevent race conditions on slot allocation and head advancement.
   */
  dma_descriptor_t *desc = &ctrl->ring[ctrl->head];
  if((desc->status & DMA_DESC_STATUS_OWN) != 0)
  {
    return -1;
  }

  desc->buffer_addr = (uint64_t)(uintptr_t)packet_buffer;
  desc->length = length;
  desc->flags = DMA_DESC_FLAG_EOP | DMA_DESC_FLAG_IOC;

  /*
   * // AI Review Feedback: Memory Barrier & Store Ordering Invariants (SMP / DMA Coherency):
   * In hardware DMA, the CPU writes payload address, buffer length, and control flags into the
   * descriptor before handing ownership to hardware via `DMA_DESC_STATUS_OWN`.
   * On modern out-of-order processors (e.g., ARM64 weak memory model, x86 store buffers), CPU
   * writes can be committed out of program order. A write memory barrier (`dma_wmb()`,
   * `atomic_thread_fence(memory_order_release)`, or `asm volatile("dmb ishst" ::: "memory")`) is
   * mandatory immediately BEFORE asserting `DMA_DESC_STATUS_OWN` and BEFORE incrementing
   * `ctrl->doorbell`. Without this release barrier, the hardware DMA controller or bus snooper
   * could observe the `OWN` bit or doorbell trigger before the payload address and length fields
   * have fully drained from CPU store buffers into physical RAM.
   *
   * // AI Review Feedback: Cache Invalidation on Non-Coherent Bus Platforms:
   * On platforms lacking hardware cache snooping (e.g., embedded microcontrollers without coherent
   * AXI interconnects or non-coherent PCIe host bridges), CPU cache lines containing the packet
   * payload and the descriptor itself must be explicitly cleaned/flushed to DRAM (e.g.,
   * `clean_dcache_range()`) prior to transferring ownership. Otherwise, the hardware DMA engine
   * reads stale data directly from RAM.
   */
  desc->status |= DMA_DESC_STATUS_OWN;

  ctrl->head = (ctrl->head + 1) % ctrl->ring_size;
  ctrl->doorbell++;

  return 0;
}

/**
 * Scans and reaps completed descriptors returned by the hardware.
 *
 * Contract & Requirements:
 * - Validates controller pointer.
 * - Inspects descriptors starting at the tail index up to head.
 * - Reaps descriptors whose ownership has been returned to the CPU.
 * - Advances consumer tail index with circular wrap-around.
 * - Returns the total count of reaped descriptors, or -1 on error.
 *
 * @param ctrl Pointer to DMA controller state structure.
 * @return Number of reaped descriptors (>= 0), or -1 on error.
 */
int dma_reap_tx_completions(dma_controller_t *ctrl)
{
  /* TODO: Author Implementation */
  if(ctrl == NULL)
  {
    return -1;
  }

  int reaped_desc = 0;

  /*
   * // AI Review Feedback: Ring Empty Ambiguity & In-Flight Tracking:
   * In circular ring buffers, the condition `ctrl->tail == ctrl->head` can indicate either an
   * entirely full ring or an entirely empty ring. Here, `(ctrl->tail == ctrl->head) && (reaped_desc
   * > 0)` handles stopping when catching up during reaping. However, if `dma_reap_tx_completions()`
   * is invoked when the ring is completely empty (zero packets submitted), `(desc->status & OWN)`
   * is 0 and `reaped_desc` is initially 0, which would allow the loop to run and falsely reap idle
   * slots. Production drivers resolve this by maintaining an explicit `in_flight` counter, keeping
   * one sentinel slot unallocated, or ensuring `dma_reap_tx_completions()` is only invoked when
   * `in_flight > 0` (or `tail != head`).
   */
  while(reaped_desc < (int)ctrl->ring_size)
  {
    dma_descriptor_t *desc = &ctrl->ring[ctrl->tail];

    /*
     * // AI Review Feedback: Memory Read Barrier & Cache Invalidation on Reaping:
     * When hardware completes DMA transfer and clears `DMA_DESC_STATUS_OWN`, the CPU must execute
     * a read memory barrier (`dma_rmb()`, `atomic_thread_fence(memory_order_acquire)`, or `asm
     * volatile("dmb ishld" ::: "memory")`) after observing `OWN == 0` and before reading transfer
     * status, errors, or unmapping/reusing the payload buffer. Additionally, on non-coherent
     * architectures, the CPU must invalidate cache lines corresponding to RX packet buffers so
     * future CPU reads access fresh data written by DMA.
     */
    if((desc->status & DMA_DESC_STATUS_OWN) != 0)
    {
      break;
    }

    if((ctrl->tail == ctrl->head) && (reaped_desc > 0))
    {
      break;
    }

    reaped_desc++;
    ctrl->tail = (ctrl->tail + 1) % ctrl->ring_size;
  }

  return reaped_desc;
}

/* ==============================================================================
 * Test Harness & DMA Hardware Simulation
 * ============================================================================== */

#define TEST_RING_SIZE 4

int main(void)
{
  printf("=== Test Suite: Coherent DMA Descriptor Ring Driver ===\n");

  dma_controller_t ctrl;
  dma_descriptor_t raw_ring[TEST_RING_SIZE];
  memset(&ctrl, 0, sizeof(ctrl));
  memset(raw_ring, 0, sizeof(raw_ring));

  int init_rc = dma_ring_init(&ctrl, raw_ring, TEST_RING_SIZE);

  /* Check if implementation is still at initial scaffold state */
  if(init_rc == -1)
  {
    printf("[Exercise Scaffold] dma_ring_init() returned -1.\n");
    printf("                     Awaiting author implementation.\n");
    return 0;
  }

  printf("[Verification] Checking ring initialization...\n");
  assert(init_rc == 0);
  assert(ctrl.ring == raw_ring);
  assert(ctrl.ring_size == TEST_RING_SIZE);
  assert(ctrl.head == 0);
  assert(ctrl.tail == 0);
  assert(ctrl.doorbell == 0);

  /* Dummy packet payloads */
  uint8_t pkt1[] = "Packet Header 1";
  uint8_t pkt2[] = "Packet Header 2";
  uint8_t pkt3[] = "Packet Header 3";
  uint8_t pkt4[] = "Packet Header 4";
  uint8_t pkt5[] = "Packet Overflow";

  printf("[Verification] Submitting packets to fill the ring...\n");
  assert(dma_submit_tx_packet(&ctrl, pkt1, sizeof(pkt1)) == 0);
  assert(ctrl.doorbell == 1);
  assert((raw_ring[0].status & DMA_DESC_STATUS_OWN) != 0);

  assert(dma_submit_tx_packet(&ctrl, pkt2, sizeof(pkt2)) == 0);
  assert(ctrl.doorbell == 2);

  assert(dma_submit_tx_packet(&ctrl, pkt3, sizeof(pkt3)) == 0);
  assert(ctrl.doorbell == 3);

  assert(dma_submit_tx_packet(&ctrl, pkt4, sizeof(pkt4)) == 0);
  assert(ctrl.doorbell == 4);

  printf("[Verification] Verifying ring full backpressure on 5th packet...\n");
  assert(dma_submit_tx_packet(&ctrl, pkt5, sizeof(pkt5)) == -1);

  printf("[Verification] Simulating hardware DMA engine processing first 2 descriptors...\n");
  /* Hardware clears OWN bit on completion */
  raw_ring[0].status &= ~DMA_DESC_STATUS_OWN;
  raw_ring[1].status &= ~DMA_DESC_STATUS_OWN;

  printf("[Verification] Reaping completed descriptors...\n");
  int reaped = dma_reap_tx_completions(&ctrl);
  assert(reaped == 2);
  assert(ctrl.tail == 2);

  printf("[Verification] Testing circular ring wrap-around with new submissions...\n");
  assert(dma_submit_tx_packet(&ctrl, pkt5, sizeof(pkt5)) == 0);
  assert(ctrl.doorbell == 5);
  /* New submission wraps around into slot 0 */
  assert((raw_ring[0].status & DMA_DESC_STATUS_OWN) != 0);

  printf("[Verification] Simulating hardware completing all remaining descriptors...\n");
  raw_ring[2].status &= ~DMA_DESC_STATUS_OWN;
  raw_ring[3].status &= ~DMA_DESC_STATUS_OWN;
  raw_ring[0].status &= ~DMA_DESC_STATUS_OWN;

  reaped = dma_reap_tx_completions(&ctrl);
  assert(reaped == 3);
  assert(ctrl.tail == ctrl.head);

  printf("=== All DMA Descriptor Ring Driver verification tests passed successfully! ===\n");
  return 0;
}
