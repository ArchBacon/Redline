#pragma once

#include <array>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <imgui/IconsFontAwesome.h>

#include "Curve.h"
#include "core/ecs.hpp"
#include "tools/inspectable.hpp"

class Curve;

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
    uint32_t gears {4};
    std::vector<float> gearRatios {2.74f, 1.57f, 1.00f, 0.67f};
    float reverseGearRatio {2.0f};
    float wheelRadius {0.33f}; // Meters
    float tyreFrictionCoefficient {1.0f};
    float wheelBase {2.746f}; // Meters
    float height {1.387f}; // Meters
    std::string torqueCurvePath {"vehicles/buick_grand_national_87/Car_Buick_GrandNational_1987_TorqueData.csv"}; 
    std::string horsepowerCurvePath {"vehicles/buick_grand_national_87/Car_Buick_GrandNational_1987_PowerData.csv"};
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
    float carHeight {};
    float b {}; // Distance from CG (Center of Gravity) to Front Axle
    float c {}; // Distance from CG to Rear Axle
    float h {};
    float g {9.8f}; // Gravity
    std::unique_ptr<Curve> torqueCurve {nullptr};
    std::unique_ptr<Curve> horsepowerCurve {nullptr};
    
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
        weight = data.mass * g;
        ForceMax = data.tyreFrictionCoefficient * weight;
        wheelBase = data.wheelBase;
        // Weight distribution is a general estimation of 57%/43% Front/Rear
        b = wheelBase * 0.57f; 
        c = wheelBase * 0.43f;
        h = data.height;
        torqueCurve = std::make_unique<Curve>(data.torqueCurvePath);
        horsepowerCurve = std::make_unique<Curve>(data.horsepowerCurvePath);
    }

    [[nodiscard]] float WeightFront(const float alpha) const { return MaxTraction() - ((c / wheelBase) * weight - (h / wheelBase) * M * Acceleration(alpha).y); }
    [[nodiscard]] float WeightRear(const float alpha) const { return MaxTraction() + ((b / wheelBase) * weight + (h / wheelBase) * M * Acceleration(alpha).y); }
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
    [[nodiscard]] glm::vec3 Acceleration(const float alpha) const { return LongitudinalForce(alpha, false) / M; } // m/s^2
    [[nodiscard]] glm::vec3 Braking(const float alpha) const { return LongitudinalForce(alpha, true) / M; } // m/s^2
    [[nodiscard]] glm::vec3 Velocity() const { return v; }
    void SetVelocity(const glm::vec3 inV) { v = inV; }
    [[nodiscard]] float Speed() const { return glm::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
    [[nodiscard]] float SpeedXY() const { return glm::sqrt(v.x * v.x + v.y * v.y); }
    [[nodiscard]] glm::vec3 Direction() const { return u; }
    [[nodiscard]] glm::vec3 BreakForce(const float alpha) const { return -u * (brakeForce * alpha); }
    [[nodiscard]] float Torque(const float rpm) const { return torqueCurve->GetValueAt(rpm); }
    [[nodiscard]] float Horsepower(const float rpm) const { return horsepowerCurve->GetValueAt(rpm); }
    [[nodiscard]] glm::vec3 DriveForce(const float rpm, const int gear) const { return u * Torque(rpm) * gearRatios[gear - 1] * differentialRatio * transmissionEfficiency / wheelRadius; }
    [[nodiscard]] float MaxTraction() const { return (M * 0.57f) * g; }


    // Solve drag·v² + rr·v - engineForce = 0  →  quadratic formula
    [[nodiscard]] float TopSpeed() const
    {
        return (-rr + glm::sqrt(rr * rr + 4.0f * drag * engineForce)) / (2.0f * drag);
    }
};
