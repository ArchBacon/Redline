#pragma once

#include "../redline.hpp"

struct Chassis
{
    // ── Specs (set once) ───────────────────
    float mass          = 1530.0f;          // kg ─ curb weight
    float wheelbase     = 2.746f;           // m ─ front axle to rear axle
    float cgToFront     = 1.566f;           // m ─ CG to front axle (wheelbase * 0.57 for GNX)
    float cgToRear      = 1.180f;           // m ─ CG to rear axle (wheelbase * 0.43 for GNX)
    float cgHeight      = 1.387f;           // m ─ CG height above ground
    float C_drag        = 0.38f;
    
    // ── Runtime state (updated every frame) ──────────────────
    float3 velocity     = {0, 0, 0};  // m/s
    float3 direction    = {0, 0, 0};  // unit vector, Y-forward in bee
    float accelLong     = 0.0f;             // m/s^2
    float W_front       = 0.0f;             // N
    float W_rear        = 0.0f;             // N
};
