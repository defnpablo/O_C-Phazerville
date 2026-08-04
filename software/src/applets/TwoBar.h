#pragma once

#include "../HemisphereApplet.h"
#include "clips.h"

class TwoBar : public HemisphereApplet {
public:
  const char* applet_name() override {
    return "2Bar";
  }

  void Start() override {
  }

  void Reset() override {
  }

  void Controller() override {
  }

  void View() override {
  }

  uint64_t OnDataRequest() override {
    return 0;
  }

  void OnDataReceive(uint64_t data) override {
    (void)data;
  }

  void OnButtonPress() override {
    HemisphereApplet::OnButtonPress();
  }

  void OnEncoderMove(int direction) override {
    (void)direction;
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
};
