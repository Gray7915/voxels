#pragma once

struct MovementStats {
    float moveSpeed = 6.5f;
    float fallMoveSpeed = 2.5f; //
    float lookSpeed = 1.5f;
    float mouseSensitivity = 0.012f;
    float drag = 0.8;
    float decelleration = 0.5;

    float groundFriction = 0.6f;
    float airFriction = 0.95f;
    float groundAcceleration = 170.0f;
    float airAcceleration = 4.0f;
    float jumpForce = 6.1f;

    bool flying = false;
};
