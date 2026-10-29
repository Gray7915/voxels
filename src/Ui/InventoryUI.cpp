#include "Ui/InventoryUI.hpp"
#include <RmlUi/Core/Factory.h>
#include <iostream>
#include "Inventory/ItemRegistry.hpp"
#include "ECS/Components/Input.hpp"
#include "ECS/Components/InventoryComponent.hpp"

namespace UI
{
    InventoryUI::InventoryUI(const Rml::String &title, const Rml::Vector2f &position, Rml::Context *context) {
        setupItemsModel(context);

        document = context->LoadDocument("../Content/ui/Inventory.rml");
        inventoryWindow = context->LoadDocument("../Content/ui/inventoryWindow.rml");

        if (document) {
            Rml::ElementList imgs;
            document->GetElementsByTagName(imgs, "img");
            for (Rml::Element *img : imgs) {
                if (img->GetAttribute<Rml::String>("src", "").empty())
                    img->SetProperty("display", "none");
            }
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

    void InventoryUI::UpdateInventoryUi(lve::Coordinator &coordinator, Entity player) {
        const auto &playerInputComp = coordinator.GetComponent<InputComponent>(player);
        int hotbarSlot = playerInputComp.hotbarSlot;
        SetSelected("slot_" + std::to_string(hotbarSlot + 1));

        auto invComp = coordinator.GetComponent<InventoryComponent>(player);

        for (int i = 0; i < invComp.inventoryStacks.size(); i++) {
            auto &stack = invComp.inventoryStacks[i];

            if (!stack.has_value()) {
                continue;
            }
            int itemID = invComp.inventoryStacks.at(i).value().getItem()->itemId;
            int slot = i + 1;
            AddItemWithImage("slot_" + std::to_string(slot), itemID);
            SetSlotCount("slot_" + std::to_string(slot), invComp.inventoryStacks.at(i).value().getStackCount());
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

    void InventoryUI::AddItemWithImage(const Rml::String &slotId, BlockId blockId) {
        if (!document)
            return;

        Rml::Element *img = document->GetElementById(slotId + "_img");
        if (!img)
            return;

        img->SetProperty("display", "block");

        const std::string src = "block://" + std::to_string(blockId);
        if (img->GetAttribute<Rml::String>("src", "") == src)
            return;

        img->SetAttribute("src", src);
    }

    void InventoryUI::SetSlotCount(const Rml::String &slotId, int count) {
        if (!document)
            return;

        Rml::Element *countEl = document->GetElementById(slotId + "_count");
        if (!countEl)
            return;

        if (count <= 1) {
            if (countEl->GetInnerRML().empty())
                return;
            countEl->SetInnerRML("");
            countEl->SetProperty("display", "none");
            return;
        }

        const Rml::String text = std::to_string(count);
        if (countEl->GetInnerRML() == text)
            return;

        countEl->SetProperty("display", "block");
        countEl->SetInnerRML(text);
    }

    void InventoryUI::SetSelected(const Rml::String &slotId) {
        if (!document)
            return;
        Rml::Element *bar = document->GetElementById("inventory_bar");
        if (!bar)
            return;

        for (int i = 0; i < bar->GetNumChildren(); ++i) {
            bar->GetChild(i)->SetClass("selected", false);
        }

        if (Rml::Element *slot = document->GetElementById(slotId)) {
            slot->SetClass("selected", true);
        }
    }

    void InventoryUI::ClearSlot(const Rml::String &slotId) {
        if (!document)
            return;
        if (Rml::Element *img = document->GetElementById(slotId + "_img")) {
            img->RemoveAttribute("src");
            img->SetProperty("display", "none");
            SetSlotCount(slotId, 0);
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
