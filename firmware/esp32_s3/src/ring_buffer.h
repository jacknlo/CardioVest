// ============================================================================
//  CardioVest / CardioCore V1 - fixed-size frame ring buffer
// ----------------------------------------------------------------------------
//  Single-producer / single-consumer FIFO of fixed-size byte frames. The
//  producer is the acquisition path (drains the ADS1298 on DRDY); the consumer
//  is the transport path (BLE / microSD). Holds RAW samples only - no
//  interpretation of any kind happens here.
// ============================================================================
#pragma once
#include <stdint.h>
#include <string.h>
#include <atomic>

template <uint16_t FRAME_SIZE, uint16_t NUM_FRAMES>
class FrameRing {
 public:
  // Producer: copy one frame in. Returns false (and counts a drop) if full.
  // The release store on head_ publishes the memcpy'd frame to the consumer;
  // pop()'s acquire load pairs with it. This keeps the FIFO correct once the
  // producer (acquisition task/core) and consumer (transport task/core) are
  // split across cores, where `volatile` alone would not order the memcpy
  // against the index publish on the Xtensa LX7.
  bool push(const uint8_t* frame) {
    const uint16_t head = head_.load(std::memory_order_relaxed);
    const uint16_t next = static_cast<uint16_t>((head + 1) % NUM_FRAMES);
    if (next == tail_.load(std::memory_order_acquire)) {  // full: keep the oldest, drop the newest
      dropped_.fetch_add(1, std::memory_order_relaxed);
      return false;
    }
    memcpy(&buf_[static_cast<uint32_t>(head) * FRAME_SIZE], frame, FRAME_SIZE);
    head_.store(next, std::memory_order_release);
    return true;
  }

  // Consumer: copy one frame out. Returns false if empty.
  bool pop(uint8_t* out) {
    const uint16_t tail = tail_.load(std::memory_order_relaxed);
    if (tail == head_.load(std::memory_order_acquire)) return false;  // empty
    memcpy(out, &buf_[static_cast<uint32_t>(tail) * FRAME_SIZE], FRAME_SIZE);
    tail_.store(static_cast<uint16_t>((tail + 1) % NUM_FRAMES), std::memory_order_release);
    return true;
  }

  bool     empty()   const { return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire); }
  uint32_t dropped() const { return dropped_.load(std::memory_order_relaxed); }

 private:
  uint8_t                buf_[static_cast<uint32_t>(FRAME_SIZE) * NUM_FRAMES];
  std::atomic<uint16_t>  head_{0};
  std::atomic<uint16_t>  tail_{0};
  std::atomic<uint32_t>  dropped_{0};
};
