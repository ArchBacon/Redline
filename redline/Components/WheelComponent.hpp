#pragma once

struct Wheel
{
    // ── Specs (set once) ───────────────────
    float radius = 0.33f;           // m — wheel + tire radius
    float mass = 20.0f;             // kg per wheel (used to calculate inertia)
    float mu = 1.0f;                // tire friction coefficient
    float C_traction = 15000.0f;    // slope of slip ratio to traction force ratio
    bool isDriven = false;          // true for rear wheels on RWD
    bool isSteering = false;        // true for front wheels
    float inertia = 0.0f;           // kg*m^2 — solid cylinder (0.5*m*r^2) * 2 wheels + drivetrain
    
    // ── Runtime state (updated every frame) ──────────────────
    float angularVelocity = 0.0f;   // w rad/s
    float slipRatio = 0.0f;
    float tractionForce = 0.0f;     // N
    float axleLoad = 0.0f;          // N
    
    void Init()
    {
        const float pureWheelInertia = 2.0f * (0.5f * mass * radius * radius);
        const float drivetrainInertia = 4.0f; // kg*m^2 — driveshaft, diff, half-shafts
        inertia = pureWheelInertia + drivetrainInertia;
    }
};
