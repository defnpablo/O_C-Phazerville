#pragma once

#include "../HemisphereApplet.h"
#include "clips.h"

class TwoBar : public HemisphereApplet {
public:
  // Timing constants
  static constexpr uint16_t PPQN            = 960;
  static constexpr uint16_t LOOP_TICKS       = 7680;
  static constexpr uint16_t TICKS_PER_PULSE  = 240;  // 16th-note external clock
  static constexpr float    CV_RANGE_VOLTS   = 5.0f; // change to 10.0f for 10V hardware

  const char* applet_name() override {
    return "2Bar";
  }

  void Start() override {
    mono_tick_     = 0;
    gate_state_    = false;
    gate_off_mono_ = 0;
    tick_accum_    = 0;
    clock_target_  = 0;
    last_cv_clip_  = 255; // sentinel: first Changed() establishes baseline only
    pending_clip_  = 0;
    selecting_     = false;
    SelectClip(0);
  }

  void Reset() override {
    mono_tick_     = 0;
    gate_state_    = false;
    gate_off_mono_ = 0;
    tick_accum_    = 0;
    clock_target_  = 0;
    GateOut(0, false);
    // Step 6.5: skip any tick-0 events so the gate stays off after reset.
    // They fire normally when the loop wraps back to position 0.
    const two_bar::ClipDefinition& clip = *active_clip_;
    event_idx_ = 0;
    while (event_idx_ < clip.eventCount &&
           clip.events[event_idx_].startTick == 0) {
      event_idx_++;
    }
  }

  void Controller() override {
    if (Clock(1)) {
      Reset();
      return;
    }

    if (Changed(0)) UpdateClipFromCV();

    // Every Clock(0) drops mono_tick_ onto the exact musical position that
    // clock represents (clock_target_). Independent of accumulator state, so
    // the first two clocks (before cycle is known) still land correctly.
    if (Clock(0)) {
      last_clock_sys_tick_ = OC::CORE::ticks;
      JumpToClockTarget();
      clock_target_ += TICKS_PER_PULSE;
      tick_accum_ = 0;
    }

    uint32_t cycle = ClockCycleTicks(0);
    if (cycle > 0 && (OC::CORE::ticks - last_clock_sys_tick_) < 2 * cycle) {
      tick_accum_ += ((uint32_t)TICKS_PER_PULSE << 16) / cycle;
      uint16_t advance_by = (uint16_t)(tick_accum_ >> 16);
      if (advance_by > 0) {
        tick_accum_ &= 0xFFFF;
        // Never let the accumulator reach or cross the next clock target.
        if (mono_tick_ + advance_by >= clock_target_) {
          advance_by = (mono_tick_ < clock_target_)
                         ? (uint16_t)(clock_target_ - mono_tick_ - 1)
                         : 0;
        }
        if (advance_by > 0) Advance(advance_by);
      }
    }
    GateOut(0, gate_state_);
  }

  void View() override {
    gfxHeader("2Bar");

    // Row 1: clip number (1-indexed) blinks while a selection is pending
    bool blink = (OC::CORE::ticks >> 11) & 1;
    if (!selecting_ || blink) {
      gfxPrint(1, 15, "#");
      gfxPrint(8, 15, (int)((selecting_ ? pending_clip_ : clip_idx_) + 1));
    }

    // Gate indicator: hollow circle = off, solid square-in-circle = on
    const int sx = 53, sy = 15, ss = 9;
    gfxLine(sx,      sy,      sx+ss-1, sy);
    gfxLine(sx,      sy+ss-1, sx+ss-1, sy+ss-1);
    gfxLine(sx,      sy,      sx,      sy+ss-1);
    gfxLine(sx+ss-1, sy,      sx+ss-1, sy+ss-1);
    if (gate_state_) { gfxRect(sx+1, sy+1, ss-2, ss-2); }

    // Row 2: timeline bar with vertical playhead
    const int bar_y  = 45;
    const int bar_x0 = 1;
    const int bar_x1 = 60;
    gfxLine(bar_x0, bar_y, bar_x1, bar_y);

    uint16_t pos    = (uint16_t)(mono_tick_ % LOOP_TICKS);
    int      head_x = bar_x0 + (int)((uint32_t)pos * (bar_x1 - bar_x0) / LOOP_TICKS);
    gfxLine(head_x, bar_y - 4, head_x, bar_y + 4);
  }

  uint64_t OnDataRequest() override {
    return 0;
  }

  void OnDataReceive(uint64_t data) override {
    (void)data;
  }

  void OnButtonPress() override {
    if (selecting_) {
      SelectClip(pending_clip_);
      selecting_ = false;
      return;
    }
    HemisphereApplet::OnButtonPress();
  }

  void OnEncoderMove(int direction) override {
    if (!selecting_) {
      selecting_    = true;
      pending_clip_ = clip_idx_;
    }
    int next = (int)pending_clip_ + direction;
    next = constrain(next, 0, (int)two_bar::ClipLibraryCount - 1);
    pending_clip_ = (uint8_t)next;
  }

protected:
  void SetHelp() override {
    help[HELP_DIGITAL1] = "Clock";
    help[HELP_DIGITAL2] = "Reset";
    help[HELP_CV1] = "Clip #";
    help[HELP_CV2] = "";
    help[HELP_OUT1] = "Gate";
    help[HELP_OUT2] = "";
  }

private:
  const two_bar::ClipDefinition* active_clip_ = &two_bar::ClipLibrary[0];
  uint8_t  clip_idx_     = 0;
  uint8_t  last_cv_clip_ = 255;
  uint8_t  pending_clip_ = 0;   // encoder preview (confirmed on button press)
  bool     selecting_    = false;

  uint32_t last_clock_sys_tick_ = 0;
  uint32_t mono_tick_    = 0;
  uint32_t gate_off_mono_ = 0; // mono_tick_ value at which the gate turns off
  uint32_t tick_accum_   = 0; // Q16 fractional tick accumulator
  uint32_t clock_target_ = 0; // absolute mono_tick position for the current Clock(0)
  uint16_t event_idx_    = 0; // index of next event to check in active clip
  bool     gate_state_   = false;

  // CV input range scaled to CV_RANGE_VOLTS so the full voltage span covers all clips.
  // First call after start sets the baseline without selecting (startup settle guard).
  void UpdateClipFromCV() {
    int cv_max = (int)(CV_RANGE_VOLTS / 5.0f * HEMISPHERE_MAX_INPUT_CV);
    int cv_idx = constrain(
      Proportion(In(0), cv_max, (int)two_bar::ClipLibraryCount - 1),
      0, (int)two_bar::ClipLibraryCount - 1);
    if (last_cv_clip_ == 255) {
      last_cv_clip_ = (uint8_t)cv_idx;
    } else if ((uint8_t)cv_idx != last_cv_clip_) {
      last_cv_clip_ = (uint8_t)cv_idx;
      SelectClip((uint8_t)cv_idx);
    }
  }

  // Switch to a new clip at the current musical position without restarting.
  void SelectClip(uint8_t idx) {
    clip_idx_   = idx;
    active_clip_ = &two_bar::ClipLibrary[idx];

    // Seek event_idx_ to the first event not yet passed in the new clip
    uint16_t pos = (uint16_t)(mono_tick_ % LOOP_TICKS);
    const two_bar::ClipDefinition& clip = *active_clip_;
    event_idx_ = 0;
    while (event_idx_ < clip.eventCount &&
           clip.events[event_idx_].startTick < pos) {
      event_idx_++;
    }
    // Leave gate_state_ alone — let any in-flight gate expire naturally
  }

  // Advance mono_tick_ by n; check gate-off and fire any events crossed.
  void Advance(uint16_t n) {
    const two_bar::ClipDefinition& clip = *active_clip_;

    for (uint16_t i = 0; i < n; i++) {
      mono_tick_++;
      uint16_t pos = (uint16_t)(mono_tick_ % LOOP_TICKS);

      if (pos == 0) {
        event_idx_ = 0;
      }

      if (gate_state_ && mono_tick_ >= gate_off_mono_) {
        gate_state_ = false;
      }

      while (event_idx_ < clip.eventCount &&
             clip.events[event_idx_].startTick <= pos) {
        gate_state_    = true;
        gate_off_mono_ = mono_tick_ + clip.events[event_idx_].durationTicks;
        event_idx_++;
      }
    }
  }

  // Jump mono_tick_ forward to clock_target_ (the musical position for the
  // current Clock(0)), fire any events at that exact position, and handle
  // gate-off / loop-wrap that fall inside the skipped range.
  void JumpToClockTarget() {
    uint32_t target = clock_target_;
    if (target < mono_tick_) return; // safety: never rewind

    if (gate_state_ && target >= gate_off_mono_) gate_state_ = false;

    uint32_t old_loop = mono_tick_ / LOOP_TICKS;
    mono_tick_ = target;
    uint16_t pos = (uint16_t)(target % LOOP_TICKS);
    uint32_t new_loop = target / LOOP_TICKS;
    if (new_loop != old_loop) event_idx_ = 0;

    const two_bar::ClipDefinition& clip = *active_clip_;
    while (event_idx_ < clip.eventCount &&
           clip.events[event_idx_].startTick <= pos) {
      if (clip.events[event_idx_].startTick == pos) {
        gate_state_    = true;
        gate_off_mono_ = target + clip.events[event_idx_].durationTicks;
      }
      event_idx_++;
    }
  }
};

