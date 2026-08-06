#include "Ui/InventoryUI.hpp"
#include <RmlUi/Core/Factory.h>
#include <iostream>

namespace UI
{
    InventoryUI::InventoryUI(const Rml::String &title, const Rml::Vector2f &position, Rml::Context *context) {
        document = context->LoadDocument("../Content/ui/Inventory.rml");
        if (document) {
            document->Show();
        }
    }

    InventoryUI::~InventoryUI() {
        if (document) {
            document->Close();
        }
    }

    void InventoryUI::AddItem(const Rml::String &name) {
        if (!document) {
            return;
        }

        Rml::Element *content = document->GetElementById("slot_1");

        // Rml::ElementPtr icon = Rml::Factory::InstanceElement(content, "icon", "icon", Rml::XMLAttributes());
        // icon->SetClass("icon", true);
        content->SetInnerRML(name);
        ///        std::cout << "Children: " << content->GetNumChildren() << '\n';
    }

    void InventoryUI::SetSelected(const Rml::String &id) {
        if (!document)
            return;

        // Clear selected from all slots
        Rml::Element *bar = document->GetElementById("inventory_bar");
        if (!bar)
            return;

        for (int i = 0; i < bar->GetNumChildren(); ++i) {
            bar->GetChild(i)->SetClass("selected", false);
        }

        // Set selected on the target slot
        if (Rml::Element *slot = document->GetElementById("slot_1")) {
            std::cout << "slot one selected" << '\n';
            slot->SetClass("selected", true);
        }
    }

} // namespace UI
