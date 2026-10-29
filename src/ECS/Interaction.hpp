#include "Inventory/Item.hpp"
#include "World/Blocks/BlockRegistry.hpp"
#include "World/Area.hpp"
#include "World/Blocks/Block.hpp"
#include "Util/Types.hpp"

/*
Facade for other classes when breaking blocks ect ->
    - sends break block request to break block method
    - sends whatever is in the players use slot if applicable

    - on receive gets the block or thing trying to be broken
    - checks that against correct tool, tier of tool, ect
    - if that succeeds break block

    -> Future
    - Have blocks turn into an entity when a destroy attempt is made
    - Have block break durability
    - Because it would be an entity have that breaking not reset immediately
*/

class Interaction {
  public:
    static void Interact(lve::Item item, ivec3 breakablePosition, lve::Area &area) {
        BlockID targetBlockID = area.getBlockID(breakablePosition);
        auto block = BlockRegistry::Get().GetBlockByID(targetBlockID);
        if (block) {
            // block.
        }
    }

    static void UseItem(lve::Item item) {}
};