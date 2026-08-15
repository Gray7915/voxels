#pragma once

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Types.h>

#include "Util/ButtonListener.hpp"
#include "Util/ItemListener.hpp"
namespace UI
{
    class InventoryUI {
      public:
        /// Constructs a new inventory and opens its window.
        /// @param[in] title The title of the new inventory.
        /// @param[in] position The position of the inventory window.
        /// @param[in] context The context to open the inventory window in.
        InventoryUI(const Rml::String &title, const Rml::Vector2f &position, Rml::Context *context);
        /// Destroys the inventory and closes its window.
        ~InventoryUI();

        /// Adds a brand-new item into this inventory.
        /// @param[in] name The name of this item.
        void AddItem(const Rml::String &name);
        void SetSelected(const Rml::String &name);

        void openCreativeInventory();
        std::function<void(const Rml::String &itemId)> onItemClicked;

      private:
        Rml::ElementDocument *document = nullptr;
        Rml::ElementDocument *inventoryWindow = nullptr;

        bool invWindowOpen = false;

        Rml::DataModelHandle itemsModel;
        std::unique_ptr<ItemListener> itemClickListener;
        std::vector<std::unique_ptr<ItemListener>> itemListeners;
        void setupItemsModel(Rml::Context *context);
        void attachClickListeners();

        struct DisplayItem {
            Rml::String id;
            Rml::String name;
        };
        std::vector<DisplayItem> displayItems;
    };
} // namespace UI