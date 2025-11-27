#pragma once
#include "HSApplication.h"
#include "TrigSeq64.h"

class TrigSeq64App : public HSApplication, public SystemExclusiveHandler {
    friend size_t TrigSeq64_save(void *storage);
    friend size_t TrigSeq64_restore(const void *storage);
    
public:
    void Start() {
      sequencer_ = TrigSeq64();
    }

    void Resume() {
      // State is restored automatically via TrigSeq64_restore()
    }

    void Suspend() {
      // maybe save some state here
    }

    void Controller() {
      if (Clock(0)) {
        if (sequencer_.playhead_cursor() >= sequencer_.end_cursor()) {
          playhead_blink_counter_ = kBlinkDuration;
        }
        
        if (sequencer_.is_playhead_step_on()) {
          float prob = sequencer_.get_step_probability(sequencer_.playhead_cursor());
          int prob_percent = static_cast<int>(prob * 100.0f);
          if (random(1, 101) <= prob_percent) {
            ClockOut(0);
          }
        }
        
        sequencer_.advance_playhead();
      }

      if (Clock(1)) {
        sequencer_.reset_playhead();
      }
    }

    void View() {
      if (playhead_blink_counter_ > 0) {
        --playhead_blink_counter_;
      }
      
      DrawPages();
      DrawSteps();
      
      if (cursor_mode_ != CursorMode::PROBABILITY) {
        gfxFrame(0, 0, kDisplayWidth, kPageTabBottom);
      }
    }

    void OnSendSysEx() {
    }

    void OnReceiveSysEx() {
    }

    /////////////////////////////////////////////////////////////////
    // Control handlers
    /////////////////////////////////////////////////////////////////
    void OnLeftButtonPress() {
      sequencer_.toggle_step_cursor();
    }

    void OnLeftButtonLongPress() {
      // Cycle through modes: END_OF_SEQ -> START_OF_SEQ -> PROBABILITY -> END_OF_SEQ
      switch (cursor_mode_) {
        case CursorMode::END_OF_SEQ:
          cursor_mode_ = CursorMode::START_OF_SEQ;
          break;
        case CursorMode::START_OF_SEQ:
          cursor_mode_ = CursorMode::PROBABILITY;
          break;
        case CursorMode::PROBABILITY:
          cursor_mode_ = CursorMode::END_OF_SEQ;
          break;
      }
    }

    void OnLeftButtonRelease() {
      // Don't clear the flag here anymore, only on long press toggle
    }

    void OnRightButtonPress() {
      switch (cursor_mode_) {
        case CursorMode::END_OF_SEQ:
          // End cursor mode: set end cursor to page end
          sequencer_.set_end_cursor_to_page_end();
          break;
        case CursorMode::START_OF_SEQ:
          // Start cursor mode: reset start cursor to page start
          sequencer_.reset_start_cursor_to_page_start();
          break;
        case CursorMode::PROBABILITY:
          // Probability mode: reset current step to 100%
          sequencer_.reset_step_cursor_probability();
          break;
      }
    }

    void OnUpButtonPress() {
      sequencer_.retreat_page();
    }

    void OnUpButtonLongPress() {
      if (cursor_mode_ == CursorMode::PROBABILITY) {
        // Probability mode: set current page probabilities to 100%
        sequencer_.set_page_probabilities_max();
      } else {
        // End/Start cursor modes: clear page
        sequencer_.clear_page();
      }
    }

    void OnDownButtonPress() {
      sequencer_.advance_page();
    }

    void OnDownButtonLongPress() {
      if (cursor_mode_ == CursorMode::PROBABILITY) {
        // Probability mode: set current page probabilities to 10%
        sequencer_.set_page_probabilities_min();
      } else {
        // End/Start cursor modes: fill page
        sequencer_.fill_page();
      }
    }

    void OnLeftEncoderMove(int direction) {
      // Left encoder always moves step cursor (in both normal and probability mode)
      if (direction > 0) {
        sequencer_.advance_step_cursor();
      } else {
        sequencer_.retreat_step_cursor();
      }
    }

    void OnRightEncoderMove(int direction) {
      switch (cursor_mode_) {
        case CursorMode::END_OF_SEQ:
          // End cursor mode: move end cursor
          if (direction > 0) {
            sequencer_.advance_end_cursor();
          } else {
            sequencer_.retreat_end_cursor();
          }
          break;
        case CursorMode::START_OF_SEQ:
          // Start cursor mode: move start cursor
          if (direction > 0) {
            sequencer_.advance_start_cursor();
          } else {
            sequencer_.retreat_start_cursor();
          }
          break;
        case CursorMode::PROBABILITY:
          // Probability mode: adjust probability with right encoder
          if (direction > 0) {
            sequencer_.increase_step_cursor_probability();
          } else {
            sequencer_.decrease_step_cursor_probability();
          }
          break;
      }
    }

private:
    // Display layout constants
    static constexpr int kDisplayWidth = 128;
    static constexpr int kDisplayHeight = 64;
    
    // Page tab area
    static constexpr int kPageTabY = 1;
    static constexpr int kPageTabHeight = 16;
    static constexpr int kPageTabBottom = kPageTabY + kPageTabHeight + 1;  // 18
    static constexpr int kPageTabWidth = kDisplayWidth / TrigSeq64::PAGE_COUNT;  // 32
    static constexpr int kPageTabCenterY = kPageTabY + 8;
    
    // Marker rectangles (start/end indicators)
    static constexpr int kMarkerWidth = 4;
    static constexpr int kMarkerHalfHeight = 6;
    static constexpr int kMarkerInset = 2;
    
    // Page number position
    static constexpr int kPageNumberOffsetX = 7;
    static constexpr int kPageNumberOffsetY = 5;
    
    // Playhead triangle in tab
    static constexpr int kTabTriangleOffset = 14;
    static constexpr int kTabTriangleSize = 4;
    
    // Selection indicator
    static constexpr int kSelectionWidth = 12;
    static constexpr int kSelectionHeight = 2;
    
    // Step grid
    static constexpr int kStepsPerRow = 8;
    static constexpr int kStepSpacing = 16;
    static constexpr int kStepGridStartX = 8;
    static constexpr int kStepGridTopY = 32;
    static constexpr int kStepGridBottomY = 51;
    static constexpr int kStepBoxRadius = 6;
    
    // Timing
    static constexpr uint8_t kBlinkDuration = 6;
    
    // Storage
    static constexpr uint8_t kStorageVersion = 0xA2;
    
    // Cursor mode
    enum class CursorMode { END_OF_SEQ, START_OF_SEQ, PROBABILITY };
    
    // State
    TrigSeq64 sequencer_;
    CursorMode cursor_mode_ = CursorMode::END_OF_SEQ;
    uint8_t playhead_blink_counter_ = 0;
    
    size_t SaveToStorage(void *storage) {
        uint8_t *data = static_cast<uint8_t*>(storage);
        size_t offset = 0;
        
        data[offset++] = kStorageVersion;
        
        uint64_t steps_value = sequencer_.steps().to_ullong();
        memcpy(data + offset, &steps_value, sizeof(steps_value));
        offset += sizeof(steps_value);
        
        data[offset++] = sequencer_.page_cursor();
        data[offset++] = sequencer_.step_cursor();
        data[offset++] = sequencer_.playhead_cursor();
        data[offset++] = sequencer_.end_cursor();
        data[offset++] = sequencer_.start_cursor();
        
        // Save probabilities array
        for (size_t i = 0; i < TrigSeq64::MAX_STEPS; ++i) {
            float prob = sequencer_.get_step_probability(i);
            memcpy(data + offset, &prob, sizeof(float));
            offset += sizeof(float);
        }
        
        return offset;
    }
    
    size_t RestoreFromStorage(const void *storage) {
        const uint8_t *data = static_cast<const uint8_t*>(storage);
        size_t offset = 0;
        
        uint8_t version = data[offset++];
        if (version != kStorageVersion) {
            sequencer_ = TrigSeq64();
            return 0;
        }
        
        uint64_t steps_value;
        memcpy(&steps_value, data + offset, sizeof(steps_value));
        offset += sizeof(steps_value);
        std::bitset<TrigSeq64::MAX_STEPS> steps(steps_value);
        
        uint8_t page_cursor = data[offset++];
        uint8_t step_cursor = data[offset++];
        uint8_t playhead_cursor = data[offset++];
        uint8_t end_cursor = data[offset++];
        uint8_t start_cursor = data[offset++];
        
        // Restore probabilities array
        std::array<float, TrigSeq64::MAX_STEPS> probabilities;
        for (size_t i = 0; i < TrigSeq64::MAX_STEPS; ++i) {
            float prob;
            memcpy(&prob, data + offset, sizeof(float));
            offset += sizeof(float);
            probabilities[i] = prob;
        }
        
        sequencer_ = TrigSeq64(steps, playhead_cursor, step_cursor, start_cursor, end_cursor, page_cursor, probabilities);
        
        return offset;
    }

    void DrawPlayheadIndicator(int tab_x) {
        const int tri_x = tab_x + kPageTabWidth - kTabTriangleOffset;
        const int tri_y = kPageTabCenterY;
        gfxLine(tri_x, tri_y - kTabTriangleSize + 1, tri_x, tri_y + kTabTriangleSize - 1);
        gfxLine(tri_x, tri_y - kTabTriangleSize + 1, tri_x + kTabTriangleSize, tri_y);
        gfxLine(tri_x, tri_y + kTabTriangleSize - 1, tri_x + kTabTriangleSize, tri_y);
    }
    
    void DrawMarkerRect(int x, int y, bool filled) {
        const int top = y - kMarkerHalfHeight;
        const int bottom = y + kMarkerHalfHeight - 1;
        const int right = x + kMarkerWidth - 1;
        
        gfxLine(x, top, x, bottom);           // Left
        gfxLine(right, top, right, bottom);   // Right
        gfxLine(x, top, right, top);          // Top
        gfxLine(x, bottom, right, bottom);    // Bottom
        
        if (filled) {
            for (int fy = top + 1; fy < bottom; ++fy) {
                gfxLine(x + 1, fy, right - 1, fy);
            }
        }
    }

    void DrawPages() {
        const uint8_t start_page = sequencer_.start_cursor() / TrigSeq64::PAGE_LENGTH;
        const uint8_t end_page = sequencer_.end_cursor() / TrigSeq64::PAGE_LENGTH;
        const uint8_t playhead_page = sequencer_.get_playhead_page();
        const uint8_t selected_page = sequencer_.page_cursor();
        
        for (size_t page = 0; page < TrigSeq64::PAGE_COUNT; ++page) {
            const int tab_x = static_cast<int>(page) * kPageTabWidth;
            
            // Bottom border
            gfxLine(tab_x, kPageTabY + kPageTabHeight - 1,
                    tab_x + kPageTabWidth - 1, kPageTabY + kPageTabHeight - 1);
            
            // Vertical divider
            if (page > 0) {
                gfxLine(tab_x, kPageTabY, tab_x, kPageTabY + kPageTabHeight - 1);
            }
            
            // Start marker
            if (page == start_page) {
                DrawMarkerRect(tab_x + kMarkerInset, kPageTabCenterY,
                               cursor_mode_ == CursorMode::START_OF_SEQ);
            }
            
            // End marker
            if (page == end_page) {
                DrawMarkerRect(tab_x + kPageTabWidth - kMarkerInset - kMarkerWidth,
                               kPageTabCenterY,
                               cursor_mode_ == CursorMode::END_OF_SEQ);
            }
            
            // Page number
            gfxPrint(tab_x + (kPageTabWidth / 2) - kPageNumberOffsetX,
                     kPageTabY + kPageNumberOffsetY, page + 1);
            
            // Playhead indicator
            if (page == playhead_page) {
                DrawPlayheadIndicator(tab_x);
            }
            
            // Selection indicator
            if (page == selected_page) {
                const int rect_x = tab_x + (kPageTabWidth - kSelectionWidth) / 2;
                for (int i = 0; i < kSelectionHeight; ++i) {
                    gfxLine(rect_x, kPageTabY + i, rect_x + kSelectionWidth - 1, kPageTabY + i);
                }
            }
        }
    }
    
    void DrawCursorLine(int x, int y_start, int y_end, bool thick) {
        gfxLine(x, y_start, x, y_end);
        if (thick) {
            gfxLine(x - 1, y_start, x - 1, y_end);
        }
    }
    
    void DrawStepPlayhead(int x, int y, bool is_top_row) {
        constexpr int kTriSize = 4;
        if (is_top_row) {
            for (int dy = 0; dy < kTriSize; ++dy) {
                const int half_w = kTriSize - 1 - dy;
                gfxLine(x - half_w, kPageTabBottom + dy, x + half_w, kPageTabBottom + dy);
            }
        } else {
            const int tri_y = y + kStepBoxRadius + kStepBoxRadius;
            for (int dy = 0; dy < kTriSize; ++dy) {
                const int half_w = kTriSize - 1 - dy;
                gfxLine(x - half_w, tri_y - dy, x + half_w, tri_y - dy);
            }
        }
    }
    
    void DrawStep(int x, int y, int step_index, bool is_top_row) {
        const float prob = sequencer_.get_step_probability(step_index);
        
        // Probability display
        if (prob >= 0.999f) {
            gfxPrint(x - 2, y - 3, "1");
        } else {
            const int tenths = static_cast<int>(prob * 10.0f + 0.5f);
            gfxPrint(x - 5, y - 3, ".");
            gfxPrint(x - 2, y - 3, tenths);
        }
        
        // Step ON indicator
        if (sequencer_.steps()[step_index]) {
            const int size = (kStepBoxRadius - 1) * 2;
            gfxInvert(x - kStepBoxRadius + 1, y - kStepBoxRadius + 1, size, size);
        }
        
        // Step cursor frame
        if (step_index == sequencer_.step_cursor()) {
            const int size = (kStepBoxRadius + 1) * 2;
            gfxFrame(x - kStepBoxRadius - 1, y - kStepBoxRadius - 1, size, size);
        }
        
        // Playhead triangle
        if (step_index == sequencer_.playhead_cursor() && playhead_blink_counter_ == 0) {
            DrawStepPlayhead(x, y, is_top_row);
        }
        
        // Cursor line endpoints
        const int line_top = kPageTabBottom;
        const int line_box_offset = kStepBoxRadius;
        
        // End cursor line
        if (step_index == sequencer_.end_cursor()) {
            const int line_x = x + kStepBoxRadius;
            const bool thick = (cursor_mode_ == CursorMode::END_OF_SEQ);
            if (is_top_row) {
                DrawCursorLine(line_x, line_top, y + line_box_offset, thick);
            } else {
                DrawCursorLine(line_x, y - line_box_offset, kDisplayHeight - 1, thick);
            }
        }
        
        // Start cursor line
        if (step_index == sequencer_.start_cursor()) {
            const int line_x = x - kStepBoxRadius - 1;
            const bool thick = (cursor_mode_ == CursorMode::START_OF_SEQ);
            if (is_top_row) {
                DrawCursorLine(line_x, line_top, y + line_box_offset, thick);
            } else {
                DrawCursorLine(line_x, y - line_box_offset, kDisplayHeight - 1, thick);
            }
        }
    }
    
    void DrawSteps() {
        const uint8_t page_start = sequencer_.page_cursor() * TrigSeq64::PAGE_LENGTH;
        
        for (int step = 0; step < static_cast<int>(TrigSeq64::PAGE_LENGTH); ++step) {
            const int step_index = page_start + step;
            const int col = step % kStepsPerRow;
            const bool is_top_row = (step < kStepsPerRow);
            
            const int x = kStepGridStartX + (col * kStepSpacing);
            const int y = is_top_row ? kStepGridTopY : kStepGridBottomY;
            
            DrawStep(x, y, step_index, is_top_row);
        }
    }
};

TrigSeq64App TrigSeq64_instance;

// App stubs
void TrigSeq64_init() {
  TrigSeq64_instance.BaseStart();
}

// Storage: save sequencer state
static constexpr size_t TrigSeq64_storageSize() {
    return sizeof(uint64_t) + 4 * sizeof(uint8_t) + TrigSeq64::MAX_STEPS * sizeof(float);
}

size_t TrigSeq64_save(void *storage) {
    return TrigSeq64_instance.SaveToStorage(storage);
}

size_t TrigSeq64_restore(const void *storage) {
    return TrigSeq64_instance.RestoreFromStorage(storage);
}

void TrigSeq64_isr() {
  TrigSeq64_instance.BaseController();
}

void TrigSeq64_handleAppEvent(OC::AppEvent event) {
  if (event ==  OC::APP_EVENT_RESUME) {
    TrigSeq64_instance.Resume();
  }
  if (event == OC::APP_EVENT_SUSPEND) {
    TrigSeq64_instance.OnSendSysEx();
  }
}

void TrigSeq64_loop() {} // Deprecated

void TrigSeq64_menu() {
  TrigSeq64_instance.BaseView();
}

void TrigSeq64_screensaver() {
  TrigSeq64_instance.BaseScreensaver(true);
}

void TrigSeq64_handleButtonEvent(const UI::Event &event) {
    // For left encoder, handle press and long press
    // For right encoder, only handle press (long press is reserved)
    // For up button, handle only press (long press is reserved)
    // For down button, handle press and long press
    switch (event.type) {
    case UI::EVENT_BUTTON_DOWN: // when button is first pressed
        //TrigSeq64_instance.OnButtonDown(event);
        break;

    case UI::EVENT_BUTTON_PRESS: { // when button is released
        switch (event.control) {
        case OC::CONTROL_BUTTON_L:
          TrigSeq64_instance.OnLeftButtonRelease();
          TrigSeq64_instance.OnLeftButtonPress();
          break;
        case OC::CONTROL_BUTTON_R:
          TrigSeq64_instance.OnRightButtonPress();
          break;
        case OC::CONTROL_BUTTON_DOWN:
          TrigSeq64_instance.OnDownButtonPress();
          break;
        case OC::CONTROL_BUTTON_UP:
          TrigSeq64_instance.OnUpButtonPress();
          break;
        default: break;
        }
    } break;

    case UI::EVENT_BUTTON_LONG_PRESS:
        if (event.control == OC::CONTROL_BUTTON_L) {
            TrigSeq64_instance.OnLeftButtonLongPress();
        }
        if (event.control == OC::CONTROL_BUTTON_UP) {
            TrigSeq64_instance.OnUpButtonLongPress();
        }
        if (event.control == OC::CONTROL_BUTTON_DOWN) {
            TrigSeq64_instance.OnDownButtonLongPress();
        }
        break;

    default: break;
    }
}

void TrigSeq64_handleEncoderEvent(const UI::Event &event) {
    // Left encoder turned
    if (event.control == OC::CONTROL_ENCODER_L) TrigSeq64_instance.OnLeftEncoderMove(event.value);

    // Right encoder turned
    if (event.control == OC::CONTROL_ENCODER_R) TrigSeq64_instance.OnRightEncoderMove(event.value);
}

// TODO: instructions in a semantic way