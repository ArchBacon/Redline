#pragma once
#include <glm/gtc/constants.hpp>

#include "../Curve.h"

struct Engine
{
    
    uint16_t RPM {1000};
    
    // ── Specs (set once) ───────────────────
    Curve torqueCurve {"vehicles/buick_grand_national_87/Car_Buick_GrandNational_1987_TorqueData.csv"};
    float bmep = 16176000.0f;           // Pa ─ Brake Mean Effective Pressure
    float displacement = 3.8f;          // L — engine size (3.8L turbo V6)
    uint8_t cylinders = 6;          
    float pumpingLossFraction = 0.15f;  // 0.05 light  0.15 street  0.30 sporty  0.50 race
    
    float displacementPerCyl = 0.0f;    // m^3
    float engineBrakingTorque = 0.0f;   // N
    
    // ── Runtime state (updated every frame) ──────────────────
    float currentRPM = 800.0f;
    float driveTorque = 0.0f;   // Nm
    
    void Init()
    {
        // Liters → m^3 per cylinder: 1L = 0.001 m^3, divided by cylinder count
        displacementPerCyl = (displacement * 0.001f) / static_cast<float>(cylinders);
        // 4.0f relates to 4-stroke
        engineBrakingTorque = (bmep * displacementPerCyl) * static_cast<float>(cylinders) / (4.0f * glm::pi<float>()) * pumpingLossFraction;
        currentRPM = torqueCurve.GetMinT(); // Idle RPM
    }
};
