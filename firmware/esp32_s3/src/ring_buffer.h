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

template <uint16_t FRAME_SIZE, uint16_t NUM_FRAMES>
class FrameRing {
 public:
  // Producer: copy one frame in. Returns false (and counts a drop) if full.
  bool push(const uint8_t* frame) {
    const uint16_t next = static_cast<uint16_t>((head_ + 1) % NUM_FRAMES);
    if (next == tail_) {  // full: keep the oldest, drop the newest
      dropped_++;
      return false;
    }
    memcpy(&buf_[static_cast<uint32_t>(head_) * FRAME_SIZE], frame, FRAME_SIZE);
    head_ = next;
    return true;
  }

  // Consumer: copy one frame out. Returns false if empty.
  bool pop(uint8_t* out) {
    if (tail_ == head_) return false;  // empty
    memcpy(out, &buf_[static_cast<uint32_t>(tail_) * FRAME_SIZE], FRAME_SIZE);
    tail_ = static_cast<uint16_t>((tail_ + 1) % NUM_FRAMES);
    return true;
  }

  bool     empty()   const { return head_ == tail_; }
  uint32_t dropped() const { return dropped_; }

 private:
  uint8_t           buf_[static_cast<uint32_t>(FRAME_SIZE) * NUM_FRAMES];
  volatile uint16_t head_    = 0;
  volatile uint16_t tail_    = 0;
  volatile uint32_t dropped_ = 0;
};
