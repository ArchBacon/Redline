#pragma once
#include <vector>

struct Gearbox
{
    // ── Specs (set once) ───────────────────
    std::vector<float> gearRatios = {2.74f, 1.57f, 1.00f, 0.67f}; // forward gears
    float reverseRatio = 4.5f;  // reverse gear ratio
    float diffRatio = 3.42f;    // differential multiplier
    float efficiency = 0.85f;   // trnasmission efficiency
    
    // ── Runtime state (updated every frame) ──────────────────
    int8_t activeGear = 0;     // -1=reverse, 0=neutral, 1..N=forward
    
    [[nodiscard]] float GetRatio(const int gear) const
    {
        if (gear == 0) return 0.0f;
        if (gear < 0)  return -reverseRatio;
        return gearRatios[gear - 1];
    }
    
    [[nodiscard]] uint8_t NumForwardGears() const
    {
        return static_cast<uint8_t>(gearRatios.size());
    }
};
