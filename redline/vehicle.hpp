#pragma once

#include <array>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <imgui/IconsFontAwesome.h>

#include "core/ecs.hpp"
#include "tools/inspectable.hpp"

class BuickGrandNational87
{
    bee::Entity ID;

public:
    BuickGrandNational87();

    bee::Entity GetEntity() const { return ID; }
    
    bee::Entity CreateCarBody();
    void CreateCarWheel(bee::Entity parent, const std::string& affix, const glm::vec3& position, bool mirror);
};

/**
 *   - Cd (drag coefficient): ~0.41 (GM tested, commonly cited as 0.40–0.45)                                                                                                                                                          
  - Frontal area: ~2.1 m² (estimated — the car is about 1.80m wide × 1.37m tall, and frontal area is typically ~85% of width × height for boxy 80s cars)                                                                           
  - Curb weight: ~1,720 kg (3,795 lbs)                                                                                                                                                                                             
                                                                                                                                                                                                                                   
  For the aerodynamic drag formula from that tutorial:                                                                                                                                                                             
                                                                                                                                                                                                                                   
  Fdrag = 0.5 * Cd * A * rho * v²                                                                                                                                                                                                  
                                                                                                                                                                                                                                   
  Where rho (air density) = 1.225 kg/m³, so your combined drag constant would be:                                                                                                                                                  

  Cdrag = 0.5 * 0.41 * 2.1 * 1.225 ≈ 0.528

  Sources:
  - https://turbobuick.com/threads/cd-and-frontal-area.92993/
  - https://gbodyforum.com/threads/aerodynamics.26760/
  - https://www.automobile-catalog.com/car/1987/317510/buick_regal_grand_national_coupe.html

  The 1987 Buick Grand National had a 3.8L turbocharged V6:                                                                                                                                                                      
                                                                                                                                                                                                                                   
  - 245 hp @ 4,400 RPM (advertised, real dyno numbers closer to ~295 hp)                                                                                                                                                           
  - 355 lb-ft (481 Nm) torque @ 2,800 RPM                                                                                                                                                                                          
  - Very flat torque curve — turbo torque comes on early and stays                                                                                                                                                                 
                                                                                                                                                                                                                                   
  For your engine force calculation, the traction force at the wheels is:                                                                                                                                                          
                                                                                                                                                                                                                                   
  Ftraction = (torque * gearRatio * finalDrive * drivetrainEfficiency) / tireRadius                                                                                                                                                
                                                                                                                                                                                                                                   
  With the stock drivetrain:                                                                                                                                                                                                       
  - Final drive: 3.42:1
  - Tire radius: ~0.33m (245/50R16)
  - Drivetrain efficiency: ~0.85
  - Gear ratios (200-4R): 1st: 2.74, 2nd: 1.57, 3rd: 1.00, 4th: 0.67

  So in 1st gear at peak torque:
  Ftraction = (481 * 2.74 * 3.42 * 0.85) / 0.33 ≈ 11,600 N

  For a simple constant engine force (ignoring gears), a reasonable average would be around 3,000–4,000 N.

  Sources:
  - https://www.turbobuicks.com/threads/true-stock-specs-for-87-gn.2556/
  - https://www.automobile-catalog.com/car/1987/317510/buick_regal_grand_national_coupe.html
  - https://www.outmotorsports.com/2019/08/1987-buick-grand-national-review-blast-from-the-past/
*/
class VehicleSystem : public bee::System, public bee::IPanel
{
public:
    VehicleSystem();
    ~VehicleSystem() override = default;
    void Update(float dt) override;
    
    void OnPanel() override;
    [[nodiscard]] std::string GetName() const override { return "Vehicle Info"; }
    [[nodiscard]] std::string GetIcon() const override { return ICON_FA_AREA_CHART; }
};

struct VehicleData
{
    float engineForce {3000.f};
    float drag {0.44f};
    float mass {1530.f};
    float brakeForce {8000.f};
    float differentialRatio {3.42f};
    uint8_t gears {4};
    std::vector<float> gearRatios {2.74f, 1.57f, 1.00f, 0.67f};
    float reverseGearRatio {2.0f};
    float wheelRadius {0.33f}; // Meters
    float tyreFrictionCoefficient {1.0f};
    float wheelBase {2.746f}; // Meters
    float height {1.387f}; // Meters
};

struct Vehicle
{
    float engineForce {};
    float drag {};
    float brakeForce {};
    float rr {}; // rolling resistance
    float M {}; // mass in KG (curb)
    glm::vec3 v {}; // velocity (m/s)
    glm::vec3 u {}; // direction
    float differentialRatio {};
    uint8_t gears {};
    std::vector<float> gearRatios {};
    float transmissionEfficiency {0.85f};
    float wheelRadius {};
    float reverseGearRatio {};
    float weight {}; // Newtons
    float ForceMax {};
    float wheelBase {};
    float weightFront {};
    float weightRear {};
    float carHeight {};
    float b {}; // Distance from CG (Center of Gravity) to Front Axle
    float c {}; // Distance from CG to Rear Axle
    float h {};
    
    Vehicle(const VehicleData& data)
    {
        engineForce = data.engineForce;
        drag = data.drag;
        rr = data.drag * 30;
        M = data.mass;
        brakeForce = data.brakeForce;
        differentialRatio = data.differentialRatio;
        gears = data.gears;
        gearRatios = data.gearRatios;
        wheelRadius = data.wheelRadius;
        reverseGearRatio = data.reverseGearRatio;
        weight = data.mass * 9.8f; // 9.8m/s for gravity force
        ForceMax = data.tyreFrictionCoefficient * weight;
        wheelBase = data.wheelBase;
        // Weight distribution is a general estimation of 57%/43% Front/Rear
        b = wheelBase * 0.57f; 
        c = wheelBase * 0.43f;
        h = data.height;
    }

    [[nodiscard]] float WeightFront(const float alpha) const { return (c / wheelBase) * weight - (h / wheelBase) * M * Acceleration(alpha).y; }
    [[nodiscard]] float WeightRear(const float alpha) const { return (b / wheelBase) * weight + (h / wheelBase) * M * Acceleration(alpha).y; }
    [[nodiscard]] glm::vec3 Traction(const float alpha) const { return u * (engineForce * alpha); }
    [[nodiscard]] glm::vec3 Drag() const { return -drag * v * glm::length(v); }
    [[nodiscard]] glm::vec3 RollingResistance() const { return -(drag * 30) * v; }
    // [[nodiscard]] glm::vec3 LongitudinalForce() const { return Traction() + Drag() + RollingResistance(); }
    [[nodiscard]] glm::vec3 LongitudinalForce(const float alpha, const bool braking) const
    {
        if (braking)
        {
            return BreakForce(alpha) + Drag() + RollingResistance();
        }
        
        return Traction(alpha) + Drag() + RollingResistance();
    }
    // [[nodiscard]] glm::vec3 Acceleration() const { return LongitudinalForce() / M; } // m/s^2
    [[nodiscard]] glm::vec3 Acceleration(const float alpha) const { return LongitudinalForce(alpha, false) / M; } // m/s^2
    [[nodiscard]] glm::vec3 Braking(const float alpha) const { return LongitudinalForce(alpha, true) / M; } // m/s^2
    [[nodiscard]] glm::vec3 Velocity() const { return v; }
    void SetVelocity(const glm::vec3 inV) { v = inV; }
    [[nodiscard]] float Speed() const { return glm::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
    [[nodiscard]] float SpeedXY() const { return glm::sqrt(v.x * v.x + v.y * v.y); }
    [[nodiscard]] glm::vec3 Direction() const { return u; }
    [[nodiscard]] glm::vec3 BreakForce(const float alpha) const { return -u * (brakeForce * alpha); }
    // Buick GN 3.8L Turbo V6 — actual lb-ft values, peak 355 lb-ft @ 2800 RPM
    [[nodiscard]] float Torque(const float rpm) const
    {
        // TMP, untill RPM and proper Torque calculations are in
        static constexpr float rpms[]    = {  800,   1500,  2000,  2800,  3500,  4400,  5000,  5500,  6000,  6500 };
        static constexpr float torques[] = { 124.f, 206.f, 274.f, 355.f, 344.f, 291.f, 220.f, 160.f, 100.f,  50.f };
        static constexpr int n = 10;
        if (rpm <= rpms[0])   return torques[0];
        if (rpm >= rpms[n-1]) return torques[n-1];
        for (int i = 1; i < n; ++i)
        {
            if (rpm <= rpms[i])
            {
                const float t = (rpm - rpms[i-1]) / (rpms[i] - rpms[i-1]);
                return torques[i-1] + t * (torques[i] - torques[i-1]);
            }
        }
        return 0.0f;
    }
    [[nodiscard]] float Horsepower(const float rpm) const { return Torque(rpm) * rpm / 3000.f; }
    [[nodiscard]] glm::vec3 DriveForce(const float rpm, const int gear) const { return u * Torque(rpm) * gearRatios[gear - 1] * differentialRatio * transmissionEfficiency / wheelRadius; }

    // Solve drag·v² + rr·v - engineForce = 0  →  quadratic formula
    [[nodiscard]] float TopSpeed() const
    {
        return (-rr + glm::sqrt(rr * rr + 4.0f * drag * engineForce)) / (2.0f * drag);
    }
};
