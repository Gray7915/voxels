#include <memory>
#include "../Rendering/Core/lve_model.hpp"

namespace lve
{
    class Block
    {
    public:
        Block(uint16_t id, std::string name) : id{id}, name{name} {}
        Block(uint16_t id, std::string name);

        uint16_t id;

        std::string name;

        // AABB collision boxes for each rotation

        // Render options struct? contains things like is transparent, ect

        // mesh / model -> optional only for non blocks (things like chest count as non-block)

        int hardness;
    };
}
