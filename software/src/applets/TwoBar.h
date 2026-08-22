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

  // Piano-roll grid constants (one bar per row, 32nd-note cells)
  static constexpr uint16_t BAR_TICKS       = 3840;
  static constexpr uint16_t TICKS_PER_CELL  = 120;  // 32nd note
  static constexpr int      GRID_COLS       = 32;   // cells per bar
  static constexpr int      GRID_COL_W      = 2;    // pixels per cell
  static constexpr int      GRID_W          = GRID_COLS * GRID_COL_W; // = 64
  static constexpr int      GRID_BAR1_Y     = 34;   // row bottom stays at y=43
  static constexpr int      GRID_BAR2_Y     = 54;   // row bottom stays at y=63
  static constexpr int      GRID_ROW_H      = 10;
  static constexpr int      GRID_NOTE_H     = 10;

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

    // Gate indicator: hollow square = off, filled square = on
    const int sx = 53, sy = 15, ss = 9;
    gfxLine(sx,      sy,      sx+ss-1, sy);
    gfxLine(sx,      sy+ss-1, sx+ss-1, sy+ss-1);
    gfxLine(sx,      sy,      sx,      sy+ss-1);
    gfxLine(sx+ss-1, sy,      sx+ss-1, sy+ss-1);
    if (gate_state_) { gfxRect(sx+1, sy+1, ss-2, ss-2); }

    DrawClipGrid();
    DrawPlayhead();
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

  // Piano-roll of the active clip: bar 1 on top row, bar 2 on bottom.
  // Each cell is one 32nd-note (2 px wide). Notes with off-grid startTicks
  // (~0.4% of the library) round to the nearest 32nd.
  void DrawClipGrid() {
    const two_bar::ClipDefinition& clip = *active_clip_;
    for (uint16_t i = 0; i < clip.eventCount; i++) {
      uint16_t st  = clip.events[i].startTick;
      uint16_t dur = clip.events[i].durationTicks;
      uint8_t  bar = (uint8_t)(st / BAR_TICKS);
      if (bar > 1) continue;
      uint16_t pos_in_bar = (uint16_t)(st - (uint32_t)bar * BAR_TICKS);
      int col = (pos_in_bar + TICKS_PER_CELL / 2) / TICKS_PER_CELL;
      if (col >= GRID_COLS) col = GRID_COLS - 1;
      int len = (dur + TICKS_PER_CELL - 1) / TICKS_PER_CELL;
      if (len < 1) len = 1;
      if (col + len > GRID_COLS) len = GRID_COLS - col;
      int y = (bar == 0) ? GRID_BAR1_Y : GRID_BAR2_Y;
      int x = col * GRID_COL_W;
      int w = len * GRID_COL_W - 1; // 1 px right-edge gap between consecutive notes
      if (w < 1) w = 1;
      gfxRect(x, y, w, GRID_NOTE_H);
    }
  }

  // Small downward-pointing triangle above the active bar row marks the playhead.
  void DrawPlayhead() {
    uint16_t pos        = (uint16_t)(mono_tick_ % LOOP_TICKS);
    uint8_t  bar        = (uint8_t)(pos / BAR_TICKS);
    uint16_t pos_in_bar = (uint16_t)(pos - (uint32_t)bar * BAR_TICKS);
    int head_x = (int)((uint32_t)pos_in_bar * GRID_W / BAR_TICKS);
    if (head_x >= GRID_W) head_x = GRID_W - 1;
    int y = (bar == 0) ? GRID_BAR1_Y : GRID_BAR2_Y;
    // 5x3 triangle whose tip pixel sits directly against the row top.
    gfxLine(head_x - 2, y - 3, head_x + 2, y - 3);
    gfxLine(head_x - 1, y - 2, head_x + 1, y - 2);
    gfxPixel(head_x, y - 1);
  }
};

