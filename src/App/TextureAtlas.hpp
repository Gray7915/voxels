#pragma once
#include <string>
#include <stb_image.h>
#include <vector>
#include <unordered_map>

namespace lve
{
    struct LoadedTexture
    {
        std::string name;

        int width;
        int height;
        int channels;

        int atlasX;
        int atlasY;

        stbi_uc *pixels;
    };

    struct AtlasRegion
    {
        std::string name;
        int x;
        int y;
        int width;
        int height;
    };

    class TextureAtlas
    {
    public:
        static TextureAtlas &Get()
        {
            static TextureAtlas instance;
            return instance;
        }
        ~TextureAtlas();

        void createAtlas();

        std::vector<LoadedTexture> textures;

        std::unordered_map<std::string, AtlasRegion> atlasRegions;

        const int atlasWidth = 512;
        int atlasHeight = 0;
        stbi_uc *atlasPixels = nullptr;
    };
}
