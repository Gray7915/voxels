#pragma once

#include "lve_device.hpp"

// std
#include <string>

namespace lve
{
    class LveTexture
    {
    public:
        VkImage image;
        VkDeviceMemory imageMemory;
        VkImageView imageView;
        VkSampler sampler;
        VkDeviceSize imageSize;

        int texWidth;
        int texHeight;
        int texChannels;
        uint32_t mipLevels;
        VkFormat format;

        std::string filepath;

        // load from disk
        LveTexture(LveDevice &device, const std::string &filepath);

        // load from memory
        //LveTexture(LveDevice &device, const unsigned char *pixels, uint32_t width, uint32_t height);
        ~LveTexture();

        LveTexture(const LveTexture &) = delete;
        LveTexture &operator=(const LveTexture &) = delete;

        VkImageView getImageView() const;

        VkSampler getSampler() const;

        VkImage getImage() const;

        uint32_t getWidth() const;

        uint32_t getHeight() const;

    private:
        void loadTextureData();
        void createTextureImage();
        void createTextureImageView();
        void createTextureSampler();
        void generateMipmaps();
        void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer);
        LveDevice &lveDevice;
    };
} // namespace lve
