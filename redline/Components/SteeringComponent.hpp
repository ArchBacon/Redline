#pragma once
#include <glm/trigonometric.hpp>

struct Steering
{
    // ── Specs (set once) ───────────────────
    float maxAngleRad = glm::radians(14.0f); // half lock-to-lock (28 degrees / 2 for GNX)
    float inertiaYaw = 2000.f;                      // kg*m^2 — resistance to rotation; lower = snappier turns, higher = more tank-like
    
    // ── Runtime state (updated every frame) ──────────────────
    float currentInput = 0.0f; // -1..1
    float currentAngle = 0.0f; // rad
    float yawRate = 0.0f;      // rad/s
};
