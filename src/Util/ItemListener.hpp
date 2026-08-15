#pragma once
#include <RmlUi/Core.h>
#include <functional>
#include <iostream>
class ItemListener : public Rml::EventListener {
  public:
    ItemListener(Rml::String id, std::function<void(const Rml::String &)> cb) : itemId(std::move(id)), callback(std::move(cb)) {}

    void ProcessEvent(Rml::Event &) override {
        if (callback) {
            callback(itemId);
            std::cout << "item id from listen " << itemId << '\n';
        } else {
            std::cout << "no id? " << '\n';
        }
    }

  private:
    Rml::String itemId;
    std::function<void(const Rml::String &)> callback;
};