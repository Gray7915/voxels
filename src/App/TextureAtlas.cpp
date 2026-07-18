#include <iostream>
#include <filesystem>
#include <fstream>
#include "App/TextureAtlas.hpp"

namespace lve
{
    TextureAtlas::~TextureAtlas()
    {
        // cleanup here
    }
    void TextureAtlas::createAtlas()
    {
        int currentX = 0;
        int currentY = 0;
        int rowHeight = 0;

        for (const auto &entry : std::filesystem::directory_iterator("../Textures"))
        {
            if (entry.path().extension() == ".png")
            {
                LoadedTexture texture{};

                texture.pixels = stbi_load(entry.path().string().c_str(), &texture.width, &texture.height, &texture.channels, STBI_rgb_alpha);

                if (currentX + texture.width > atlasWidth)
                {
                    currentX = 0;
                    currentY += rowHeight;
                    rowHeight = 0;
                }
                texture.name = entry.path().stem().string();

                texture.atlasX = currentX;
                texture.atlasY = currentY;

                currentX += texture.width;
                rowHeight = std::max(rowHeight, texture.height);

                textures.push_back(std::move(texture));
            }
        }

        atlasHeight = currentY + rowHeight;

        std::vector<uint8_t> atlasPixels(atlasWidth * atlasHeight * 4);
        for (auto &texture : textures)
        {
            for (int y = 0; y < texture.height; y++)
            {
                for (int x = 0; x < texture.width; x++)
                {
                    int src = (y * texture.width + x) * 4;

                    int dst = ((texture.atlasY + y) * atlasWidth + (texture.atlasX + x)) * 4;

                    atlasPixels[dst + 0] = texture.pixels[src + 0];
                    atlasPixels[dst + 1] = texture.pixels[src + 1];
                    atlasPixels[dst + 2] = texture.pixels[src + 2];
                    atlasPixels[dst + 3] = texture.pixels[src + 3];
                }
            }
            atlasRegions.emplace(texture.name, AtlasRegion{texture.name, texture.atlasX, texture.atlasY, texture.width, texture.height});
        }

        for (auto &texture : textures)
        {
            stbi_image_free(texture.pixels);
            texture.pixels = nullptr;
        }

        std::ofstream out("../Textures/atlas.ppm", std::ios::binary);

        out << "P6\n";
        out << atlasWidth << " " << atlasHeight << "\n255\n";

        for (int i = 0; i < atlasWidth * atlasHeight; ++i)
        {
            out.put(atlasPixels[i * 4 + 0]);
            out.put(atlasPixels[i * 4 + 1]);
            out.put(atlasPixels[i * 4 + 2]);
        }
    }
}