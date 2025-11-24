#pragma once
#include "HSApplication.h"
#include "TrigSeq64.h"

class TrigSeq64App : public HSApplication, public SystemExclusiveHandler {
public:
    void Start() {
      sequencer = TrigSeq64();
    }

    void Resume() {
    }

    void Suspend() {
      // maybe save some state here
    }

    void Controller() {
      // Clock input on trigger 1: check step and advance
      if (Clock(0)) {
        if (sequencer.is_playhead_step_on()) {
          ClockOut(0);  // Send trigger on output A if step is on
        }
        sequencer.advance_playhead();
      }

      // Reset sequencer on trigger input 2
      if (Clock(1)) {
        sequencer.reset_playhead();
      }
    }

    void View() {
      gfxHeader("TriggerSeq4");
      DrawPages();
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
      sequencer.clear_page();
    }

    void OnRightButtonPress() {
      sequencer.set_end_cursor_to_page_end();
    }

    void OnUpButtonPress() {
      sequencer.advance_page();
    }

    void OnDownButtonPress() {
      sequencer.retreat_page();
    }

    void OnDownButtonLongPress() {
      sequencer.fill_page();
    }

    void OnLeftEncoderMove(int direction) {
      if (direction > 0) {
        sequencer.advance_step_cursor();
      } else {
        sequencer.retreat_step_cursor();
      }
    }

    void OnRightEncoderMove(int direction) {
      if (direction > 0) {
        sequencer.advance_end_cursor();
      } else {
        sequencer.retreat_end_cursor();
      }
    }

private:
    TrigSeq64 sequencer;

    void DrawPlayheadIndicator(int x, int page_y, int page_width) {
        // Right-pointing triangle
        int tri_x = x + page_width - 14;
        int tri_y = page_y + 8;
        gfxLine(tri_x, tri_y - 4, tri_x, tri_y + 4);
        gfxLine(tri_x, tri_y - 4, tri_x + 4, tri_y);
        gfxLine(tri_x, tri_y + 4, tri_x + 4, tri_y);
    }
    
    void DrawRitornello(int x, int page_y, int page_width) {
        // Ritornello: two dots and double bar
        int rit_x = x + page_width - 8;
        int rit_y = page_y + 8;
        gfxPixel(rit_x, rit_y - 3);
        gfxPixel(rit_x, rit_y + 3);
        gfxLine(rit_x + 2, rit_y - 5, rit_x + 2, rit_y + 5);
        gfxLine(rit_x + 4, rit_y - 5, rit_x + 4, rit_y + 5);
    }

    void DrawPages() {
        const int page_width = menu::kDisplayWidth / TrigSeq64::PAGE_COUNT;
        const int page_y = 14;
        const int page_height = 16;
        
        for (int page = 0; page < TrigSeq64::PAGE_COUNT; page++) {
            int x = page * page_width;
            
            // Draw page box
            gfxFrame(x, page_y, page_width, page_height);
            
            
            // Draw page label
            gfxPrint(x + 2, page_y + 4, "P");
            gfxPrint(page + 1);
            
            // Draw playhead indicator if playhead is on this page
            if (page == sequencer.get_playhead_page()) {
                DrawPlayheadIndicator(x, page_y, page_width);
            }
            
            // Draw ritornello if end cursor is on this page
            uint8_t end_page = sequencer.end_cursor() / TrigSeq64::PAGE_LENGTH;
            if (page == end_page) {
                DrawRitornello(x, page_y, page_width);
            }

            // Invert if this is the selected page (page_cursor)
            if (page == sequencer.page_cursor()) {
                gfxInvert(x + 1, page_y + 1, page_width - 2, page_height - 2);
            }
        }
    }
};

TrigSeq64App TrigSeq64_instance;

// App stubs
void TrigSeq64_init() {
  TrigSeq64_instance.BaseStart();
}

// Not using O_C Storage
size_t TrigSeq64_storageSize() {return 0;}
size_t TrigSeq64_save(void *storage) {return 0;}
size_t TrigSeq64_restore(const void *storage) {return 0;}

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
