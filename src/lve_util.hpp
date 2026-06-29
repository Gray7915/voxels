#pragma once

#include <functional>

namespace lve
{

  template <typename T, typename... Rest>
  void hashCombine(std::size_t &seed, const T &v, const Rest &...rest)
  {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
  };

  struct Vertex
  {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal{};
    glm::vec2 uv{};

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

    bool operator==(const Vertex &other) const
    {
      return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
    }
  };

  struct RayHit
  {
    glm::ivec3 hitPosition;
    glm::ivec3 hitDirection;
  };
}
