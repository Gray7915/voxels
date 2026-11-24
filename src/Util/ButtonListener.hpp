#pragma once
#include <RmlUi/Core.h>

class ButtonListener : public Rml::EventListener {
  public:
    std::function<void()> callback;

    ButtonListener(std::function<void()> callback) : callback(std::move(callback)) {}

    void ProcessEvent(Rml::Event &event) override { callback(); }
};