#pragma once

struct DriveInput
{
    // ── Specs (set once) ───────────────────

    
    // ── Runtime state (updated every frame) ──────────────────
    float throttle  = 0.0f;   // 0..1
    float brake     = 0.0f;   // 0..1
    float steer     = 0.0f;   // -1..1
    float handbrake = 0.0f;   // 0..1
};
