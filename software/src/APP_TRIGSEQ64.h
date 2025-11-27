#pragma once
#include "HSApplication.h"
#include "TrigSeq64.h"

class TrigSeq64App : public HSApplication, public SystemExclusiveHandler {
    friend size_t TrigSeq64_save(void *storage);
    friend size_t TrigSeq64_restore(const void *storage);
    
public:
    void Start() {
      sequencer = TrigSeq64();
    }

    void Resume() {
      // State is restored automatically via TrigSeq64_restore()
    }

    void Suspend() {
      // maybe save some state here
    }

    void Controller() {
      // Clock input on trigger 1: check step and advance
      if (Clock(0)) {
        // Only blink if we're at the end and will wrap to step 0
        if (sequencer.playhead_cursor() >= sequencer.end_cursor()) {
          playhead_blink_counter_ = 6;  // Trigger blink effect on clock (longer duration)
        }
        
        // Check if step is on and apply probability
        if (sequencer.is_playhead_step_on()) {
          float prob = sequencer.get_step_probability(sequencer.playhead_cursor());
          int prob_percent = static_cast<int>(prob * 100.0f);
          // Trigger if random roll is within probability threshold
          if (random(1, 101) <= prob_percent) {
            ClockOut(0);  // Send trigger on output A if probability passes
          }
        }
        
        sequencer.advance_playhead();
      }

      // Reset sequencer on trigger input 2
      if (Clock(1)) {
        sequencer.reset_playhead();
        // Don't trigger blink on reset
      }
    }

    void View() {
      // Decrement blink counter
      if (playhead_blink_counter_ > 0) {
        playhead_blink_counter_--;
      }
      
      // Debug header with cursor positions
      char header[32];
      snprintf(header, sizeof(header), "PH:%d ST:%d END:%d", 
               sequencer.playhead_cursor(), 
               sequencer.step_cursor(), 
               sequencer.end_cursor());
      // gfxHeader(header);
      
      DrawPages();
      DrawSteps();
      
      // Draw border around page tab container when NOT in probability mode
      if (cursor_mode_ != CursorMode::PROBABILITY) {
        gfxFrame(0, 0, 128, 18);  // Only around the page tabs area
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
      sequencer.toggle_step_cursor();
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
          sequencer.set_end_cursor_to_page_end();
          break;
        case CursorMode::START_OF_SEQ:
          // Start cursor mode: reset start cursor to page start
          sequencer.reset_start_cursor_to_page_start();
          break;
        case CursorMode::PROBABILITY:
          // Probability mode: reset current step to 100%
          sequencer.reset_step_cursor_probability();
          break;
      }
    }

    void OnUpButtonPress() {
      sequencer.retreat_page();
    }

    void OnUpButtonLongPress() {
      if (cursor_mode_ == CursorMode::PROBABILITY) {
        // Probability mode: set current page probabilities to 100%
        sequencer.set_page_probabilities_max();
      } else {
        // End/Start cursor modes: clear page
        sequencer.clear_page();
      }
    }

    void OnDownButtonPress() {
      sequencer.advance_page();
    }

    void OnDownButtonLongPress() {
      if (cursor_mode_ == CursorMode::PROBABILITY) {
        // Probability mode: set current page probabilities to 10%
        sequencer.set_page_probabilities_min();
      } else {
        // End/Start cursor modes: fill page
        sequencer.fill_page();
      }
    }

    void OnLeftEncoderMove(int direction) {
      // Left encoder always moves step cursor (in both normal and probability mode)
      if (direction > 0) {
        sequencer.advance_step_cursor();
      } else {
        sequencer.retreat_step_cursor();
      }
    }

    void OnRightEncoderMove(int direction) {
      switch (cursor_mode_) {
        case CursorMode::END_OF_SEQ:
          // End cursor mode: move end cursor
          if (direction > 0) {
            sequencer.advance_end_cursor();
          } else {
            sequencer.retreat_end_cursor();
          }
          break;
        case CursorMode::START_OF_SEQ:
          // Start cursor mode: move start cursor
          if (direction > 0) {
            sequencer.advance_start_cursor();
          } else {
            sequencer.retreat_start_cursor();
          }
          break;
        case CursorMode::PROBABILITY:
          // Probability mode: adjust probability with right encoder
          if (direction > 0) {
            sequencer.increase_step_cursor_probability();
          } else {
            sequencer.decrease_step_cursor_probability();
          }
          break;
      }
    }

private:
    TrigSeq64 sequencer;
    
    enum class CursorMode {
        END_OF_SEQ,
        START_OF_SEQ,
        PROBABILITY
    };
    CursorMode cursor_mode_ = CursorMode::END_OF_SEQ;
    
    uint8_t playhead_blink_counter_ = 0;  // Counter for playhead blink effect

    static constexpr uint8_t STORAGE_VERSION = 0xA2;  // Magic + version to detect old format
    
    size_t SaveToStorage(void *storage) {
        uint8_t *data = static_cast<uint8_t*>(storage);
        size_t offset = 0;
        
        data[offset++] = STORAGE_VERSION;
        
        uint64_t steps_value = sequencer.steps().to_ullong();
        memcpy(data + offset, &steps_value, sizeof(steps_value));
        offset += sizeof(steps_value);
        
        data[offset++] = sequencer.page_cursor();
        data[offset++] = sequencer.step_cursor();
        data[offset++] = sequencer.playhead_cursor();
        data[offset++] = sequencer.end_cursor();
        data[offset++] = sequencer.start_cursor();
        
        // Save probabilities array
        for (size_t i = 0; i < TrigSeq64::MAX_STEPS; ++i) {
            float prob = sequencer.get_step_probability(i);
            memcpy(data + offset, &prob, sizeof(float));
            offset += sizeof(float);
        }
        
        return offset;
    }
    
    size_t RestoreFromStorage(const void *storage) {
        const uint8_t *data = static_cast<const uint8_t*>(storage);
        size_t offset = 0;
        
        uint8_t version = data[offset++];
        if (version != STORAGE_VERSION) {
            // Old or incompatible format, use defaults
            sequencer = TrigSeq64();
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
        
        sequencer = TrigSeq64(steps, playhead_cursor, step_cursor, start_cursor, end_cursor, page_cursor, probabilities);
        
        return offset;
    }

    void DrawPlayheadIndicator(int x, int page_y, int page_width) {
        // Right-pointing triangle ► positioned between number and ritornello
        int tri_x = x + page_width - 14;  // Consistent offset from right edge
        int tri_y = page_y + 8;
        gfxLine(tri_x, tri_y - 4, tri_x, tri_y + 4);
        gfxLine(tri_x, tri_y - 4, tri_x + 4, tri_y);
        gfxLine(tri_x, tri_y + 4, tri_x + 4, tri_y);
    }
    
    void DrawRitornello(int x, int page_y, int page_width) {
        // Vertical rectangle end marker - symmetric with start indicator
        int rit_x = x + page_width - 6;  // 2px from right edge (mirroring start at x+2)
        int rit_y = page_y + 8;
        
        // Draw outline (4px wide, 13px tall - same as start indicator)
        gfxLine(rit_x, rit_y - 7, rit_x, rit_y + 5);      // Left vertical
        gfxLine(rit_x + 3, rit_y - 7, rit_x + 3, rit_y + 5);  // Right vertical
        gfxLine(rit_x, rit_y - 7, rit_x + 3, rit_y - 7);  // Top horizontal
        gfxLine(rit_x, rit_y + 5, rit_x + 3, rit_y + 5);  // Bottom horizontal
        
        // Fill interior when in END_OF_SEQ mode (same size as outline)
        if (cursor_mode_ == CursorMode::END_OF_SEQ) {
            for (int fy = rit_y - 6; fy <= rit_y + 4; fy++) {
                gfxLine(rit_x + 1, fy, rit_x + 2, fy);
            }
        }
    }

    void DrawPages() {
        const int page_width = menu::kDisplayWidth / TrigSeq64::PAGE_COUNT;
        const int page_y = 1;
        const int page_height = 16;
        
        for (size_t page = 0; page < TrigSeq64::PAGE_COUNT; page++) {
            int x = static_cast<int>(page) * page_width;
            
            // Draw page box without top border (header provides top line)
            // Draw bottom border for all pages
            gfxLine(x, page_y + page_height - 1, x + page_width - 1, page_y + page_height - 1);  // Bottom
            // Draw vertical divider only between pages (not at edges)
            if (page > 0) {
                gfxLine(x, page_y, x, page_y + page_height - 1);  // Divider
            }
            
            // Draw start cursor indicator on the page where start cursor is
            uint8_t start_page = sequencer.start_cursor() / TrigSeq64::PAGE_LENGTH;
            if (page == start_page) {
                int start_x = x + 2;
                int start_y = page_y + 8;
                // Draw vertical rectangle on the left
                gfxLine(start_x, start_y - 7, start_x, start_y + 5);      // Left vertical
                gfxLine(start_x + 3, start_y - 7, start_x + 3, start_y + 5);  // Right vertical
                gfxLine(start_x, start_y - 7, start_x + 3, start_y - 7);  // Top horizontal
                gfxLine(start_x, start_y + 5, start_x + 3, start_y + 5);  // Bottom horizontal
                
                // Fill when in START_OF_SEQ mode
                if (cursor_mode_ == CursorMode::START_OF_SEQ) {
                    for (int fy = start_y - 6; fy <= start_y + 4; fy++) {
                        gfxLine(start_x + 1, fy, start_x + 2, fy);
                    }
                }
            }
            
            // Draw page label (just the number, centered in tab)
            int num_x = x + (page_width / 2) - 7;  // Center the single digit
            gfxPrint(num_x, page_y + 5, page + 1);
            
            // Draw playhead indicator if playhead is on this page
            if (page == sequencer.get_playhead_page()) {
                DrawPlayheadIndicator(x, page_y, page_width);
            }
            
            // Draw ritornello if end cursor is on this page
            uint8_t end_page = sequencer.end_cursor() / TrigSeq64::PAGE_LENGTH;
            if (page == end_page) {
                DrawRitornello(x, page_y, page_width);
            }

            // Draw selection indicator for selected page (page_cursor)
            if (page == sequencer.page_cursor()) {
                // Draw a thin white rectangle centered at the top of the selected tab
                int rect_width = 12;  // Narrow centered rectangle
                int rect_height = 2;  // Thinner
                int rect_x = x + (page_width - rect_width) / 2;  // Center it
                int rect_y = page_y;  // At the top
                
                // Fill the rectangle
                for (int i = 0; i < rect_height; i++) {
                    gfxLine(rect_x, rect_y + i, rect_x + rect_width - 1, rect_y + i);
                }
            }
        }
    }
    
    void DrawStep(int x, int y, int step_index, bool is_top_row) {
        const int box_radius = 6;
        
        // Always show probability numbers
        float prob = sequencer.get_step_probability(step_index);
        
        // Draw probability numbers first
        if (prob >= 0.999f) {
            // 100%: just show "1"
            gfxPrint(x - 2, y - 3, "1");
        } else {
            // <100%: show .1, .2, .3, etc
            int tenths = static_cast<int>(prob * 10.0f + 0.5f);
            gfxPrint(x - 5, y - 3, ".");
            gfxPrint(x - 2, y - 3, tenths);
        }
        
        // Invert background for steps that are ON
        if (sequencer.steps()[step_index]) {
            gfxInvert(x - box_radius + 1, y - box_radius + 1, 
                     (box_radius - 1) * 2, (box_radius - 1) * 2);
        }
        
        // Draw step cursor indicator (square around probability number)
        if (step_index == sequencer.step_cursor()) {
            gfxFrame(x - box_radius - 1, y - box_radius - 1, 
                    (box_radius + 1) * 2, (box_radius + 1) * 2);
        }
        
        // Always draw playhead and end cursor indicators (even in probability mode)
        
        // Draw playhead indicator (solid triangle) - hide when blinking
        uint8_t playhead = sequencer.playhead_cursor();
        if (step_index == playhead && playhead_blink_counter_ == 0) {
            if (is_top_row) {
                // Solid triangle touching page bottom for top row (pointing down)
                int tri_y = 18;  // Page bottom
                for (int dy = 0; dy <= 3; dy++) {
                    int width = 3 - dy;
                    gfxLine(x - width, tri_y + dy, x + width, tri_y + dy);
                }
            } else {
                // Solid triangle at bottom of box for bottom row (pointing up)
                int tri_y = y + box_radius + 6;
                for (int dy = 0; dy <= 3; dy++) {
                    int width = 3 - dy;
                    gfxLine(x - width, tri_y - dy, x + width, tri_y - dy);
                }
            }
        }
        
        // Draw end cursor indicator (vertical line, thick in END_OF_SEQ mode)
        if (step_index == sequencer.end_cursor()) {
            int line_x = x + box_radius;
            
            if (is_top_row) {
                // Line extending UP from top of step box to bottom of page boxes
                if (cursor_mode_ == CursorMode::END_OF_SEQ) {
                    // End cursor mode: 2px wide
                    gfxLine(line_x - 1, 18, line_x - 1, y + 6);
                    gfxLine(line_x, 18, line_x, y + 6);
                } else {
                    // Other modes: 1px wide
                    gfxLine(line_x, 18, line_x, y + 6);
                }
            } else {
                // Line extending DOWN from bottom of step box to screen bottom
                if (cursor_mode_ == CursorMode::END_OF_SEQ) {
                    // End cursor mode: 2px wide
                    gfxLine(line_x - 1, y - 6, line_x - 1, 63);
                    gfxLine(line_x, y - 6, line_x, 63);
                } else {
                    // Other modes: 1px wide
                    gfxLine(line_x, y - 6, line_x, 63);
                }
            }
        }
        
        // Draw start cursor indicator (vertical line on the LEFT of the step, thick in START_OF_SEQ mode)
        if (step_index == sequencer.start_cursor()) {
            int line_x = x - box_radius - 1;
            
            if (is_top_row) {
                // Line extending UP from top of step box to bottom of page boxes
                if (cursor_mode_ == CursorMode::START_OF_SEQ) {
                    // Start cursor mode: 2px wide (expand left like end cursor)
                    gfxLine(line_x - 1, 18, line_x - 1, y + 6);
                    gfxLine(line_x, 18, line_x, y + 6);
                } else {
                    // Other modes: 1px wide
                    gfxLine(line_x, 18, line_x, y + 6);
                }
            } else {
                // Line extending DOWN from bottom of step box to screen bottom
                if (cursor_mode_ == CursorMode::START_OF_SEQ) {
                    // Start cursor mode: 2px wide (expand left like end cursor)
                    gfxLine(line_x - 1, y - 6, line_x - 1, 63);
                    gfxLine(line_x, y - 6, line_x, 63);
                } else {
                    // Other modes: 1px wide
                    gfxLine(line_x, y - 6, line_x, 63);
                }
            }
        }
    }
    
    void DrawSteps() {
        const int steps_per_row = 8;
        const int step_spacing = 16;
        const int start_x = 8;
        const int row_y_top = 32;
        const int row_y_bottom = 51;
        
        // Get current page's step range
        uint8_t page_start = sequencer.page_cursor() * TrigSeq64::PAGE_LENGTH;
        
        // Steps 0-7 draw at top (y=37), steps 8-15 at bottom (y=54)
        for (int step = 0; step < 16; step++) {
            int step_index = page_start + step;
            int col = step % steps_per_row;
            // For visual positioning: steps 0-7 are at top, steps 8-15 at bottom
            bool is_visual_top = (step < steps_per_row);
            
            int x = start_x + (col * step_spacing);
            int y = is_visual_top ? row_y_top : row_y_bottom;
            
            DrawStep(x, y, step_index, is_visual_top);
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