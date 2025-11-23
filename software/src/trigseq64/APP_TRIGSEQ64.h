#pragma once
#include "HSApplication.h"
class TrigSeq64 : public HSApplication, public SystemExclusiveHandler {
public:
    void Start() {
      // initialize state here
    }

    void Resume() {
    }

    void Suspend() {
      // maybe save some state here
    }

    void Controller() {
      // MAIN I/O LOGIC
    }

    void View() {
      gfxHeader("TrigSeq64");
      //DrawInterface();
    }

    void OnSendSysEx() {
    }

    void OnReceiveSysEx() {
    }

    /////////////////////////////////////////////////////////////////
    // Control handlers
    /////////////////////////////////////////////////////////////////
    void OnLeftButtonPress() {
    }

    void OnLeftButtonLongPress() {
    }

    void OnRightButtonPress() {
    }

    void OnUpButtonPress() {
    }

    void OnDownButtonPress() {
    }

    void OnDownButtonLongPress() {
    }

    void OnLeftEncoderMove(int direction) {
    }

    void OnRightEncoderMove(int direction) {
    }

private:
    int8_t cursor;

    /* Example private screen-drawing method
    void DrawInterface() {
    }
    */
};

TrigSeq64 TrigSeq64_instance;

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
