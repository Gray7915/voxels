#pragma once
#include <glm/glm.hpp>
#include <chrono>

struct InputComponent
{
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool jump = false;
    bool moveUp = false;
    bool moveDown = false;

    float mouseDeltaX = 0.f;
    float mouseDeltaY = 0.f;
    float lastX = 0.0;
    float lastY = 0.0;

    bool firstMouse = true;

    std::chrono::steady_clock::time_point currentTime{};
    bool firstJump = false;
};
