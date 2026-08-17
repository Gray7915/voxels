#pragma once
#include <glm/glm.hpp>
#include <chrono>

struct InputComponent {
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool jump = false;
    bool moveUp = false;
    bool moveDown = false;

    bool releaseSpace = false;

    bool openInventory = false;

    float mouseDeltaX = 0.f;
    float mouseDeltaY = 0.f;
    float lastX = 0.0;
    float lastY = 0.0;

    bool firstMouse = true;

    std::chrono::steady_clock::time_point currentTime{};
    bool firstJump = false;

    bool escape = false;

    bool leftMouseClicked = false;
    bool middleMouseClicked = false;
    bool rightMouseClicked = false;

    bool primaryAction = false;   // left mouse break block thingie
    bool secondaryAction = false; // open chest, interact with block, ect

    std::chrono::steady_clock::time_point leftMouseWasClicked;
    std::chrono::steady_clock::time_point middleMouseWasClicked;
    std::chrono::steady_clock::time_point rightMouseWasClicked;

    int hotbarSlot = 0;
};
