#include "Ui/InventoryUI.hpp"
#include <RmlUi/Core/Factory.h>
#include <iostream>
#include "Inventory/ItemRegistry.hpp"

namespace UI
{
    InventoryUI::InventoryUI(const Rml::String &title, const Rml::Vector2f &position, Rml::Context *context) {
        setupItemsModel(context);

        document = context->LoadDocument("../Content/ui/Inventory.rml");
        inventoryWindow = context->LoadDocument("../Content/ui/inventoryWindow.rml");

        if (document) {
            document->Show();
        }

        attachClickListeners();
    }

    InventoryUI::~InventoryUI() {
        if (document) {
            document->Close();
        }
        if (inventoryWindow) {
            inventoryWindow->Close();
        }
    }

    void InventoryUI::setupItemsModel(Rml::Context *context) {
        // build display list from registry
        for (auto &[id, item] : lve::ItemRegistry::Get().GetAll()) {
            displayItems.push_back({std::to_string(id), item.itemName});
        }
        std::cout << "Items found: " << displayItems.size() << '\n';
        if (Rml::DataModelConstructor constructor = context->CreateDataModel("items")) {

            if (auto itemHandle = constructor.RegisterStruct<DisplayItem>()) {
                itemHandle.RegisterMember("id", &DisplayItem::id);
                itemHandle.RegisterMember("name", &DisplayItem::name);
            }

            constructor.RegisterArray<std::vector<DisplayItem>>();

            constructor.Bind("items", &displayItems);

            itemsModel = constructor.GetModelHandle();
        }
    }

    void InventoryUI::attachClickListeners() {
        if (!inventoryWindow)
            return;

        Rml::ElementList elements;

        inventoryWindow->GetElementsByTagName(elements, "div");

        for (Rml::Element *element : elements) {
            std::cout << "element ID " << element->GetId() << '\n';
        }

        for (auto &[id, item] : lve::ItemRegistry::Get().GetAll()) {
            auto *el = inventoryWindow->GetElementById(std::to_string(id));
            std::cout << "element Id thingie " << id << '\n';
            if (el) {
                std::cout << "attatched listener" << '\n';
                auto *listener = new ItemListener(std::to_string(item.itemId), [this](const Rml::String &id) {
                    if (onItemClicked) {
                        std::cout << "listen id " << id << '\n';
                        onItemClicked(id);
                    }
                });
                el->AddEventListener(Rml::EventId::Click, listener);
                itemListeners.push_back(std::unique_ptr<ItemListener>(listener));
            }
        }
        std::cout << "finished listener attatchment" << '\n';
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

    void InventoryUI::AddItemWithImage(const Rml::String &slotId, const VkDescriptorSet &descSet) {
        if (!document) {
            std::cout << "AddItemWithImage: document is null!\n";
            return;
        }

        std::string elemId = slotId + "_img";
        Rml::Element *img = document->GetElementById(elemId);
        std::cout << "AddItemWithImage: looking for '" << elemId << "' → " << (img ? "FOUND" : "NOT FOUND") << '\n';

        if (!img) {
            std::cout << "img not found: " << slotId << "_img\n";
            return;
        }

        std::string src = "block://" + std::to_string((uintptr_t)descSet);
        std::cout << "setting src: " << src << '\n';
        img->RemoveAttribute("src");
        img->SetAttribute("src", src);
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
        if (Rml::Element *slot = document->GetElementById("slot_2_img")) {
            // std::cout << "slot one selected" << '\n';
            slot->SetClass("selected", true);
        }
    }

    void InventoryUI::openCreativeInventory() {
        if (!invWindowOpen) {
            inventoryWindow->Show();
            invWindowOpen = true;
        } else if (inventoryWindow) {
            inventoryWindow->Hide();
            invWindowOpen = false;
        }
    }

} // namespace UI
