#pragma once

#include <array>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
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
    float drag {0.38f};
    float mass {1530.f};
    float differentialRatio {3.42f};
    int gears {4};
    std::vector<float> gearRatios {2.74f, 1.57f, 1.00f, 0.67f};
    float reverseGearRatio {4.5f}; // Higher ratio = lower reverse top speed (~34 km/h at redline)
    float wheelRadius {0.33f}; // Meters
    float tyreFrictionCoefficient {1.0f};
    float wheelBase {2.746f}; // Meters
    float height {1.387f}; // Meters
    float tractionConstant {15000.0f}; // C_t: slip ratio → traction force slope (N per unit SR)
    float bmep {1617600000.f};              // Pa  — Brake Mean Effective Pressure (161,760 kPa, tuned for feel)
    float engineDisplacement {0.0038f};// m³  — 3.8 L turbocharged V6
    float engineBrakingSlip {0.05f};        // Fraction of free-roll ω removed when off-throttle (0=no braking, 1=locked)
    std::string torqueCurvePath {"vehicles/buick_grand_national_87/Car_Buick_GrandNational_1987_TorqueData.csv"}; 
    std::string horsepowerCurvePath {"vehicles/buick_grand_national_87/Car_Buick_GrandNational_1987_PowerData.csv"};
};

/** Constants
 *  EngineForce = 3000
 *  Cdrag = 0.44
 *  v = {0, 0}  // velocity
 *  u = {0, 0}  // direction
 *  Crr = Cdrag * 30
 *  M = 1530    // Mass in Kg
 *  BMEP = 1617.6 kPa (Brake Mean Effective Pressure)
 */

/** Maths
 * Ftraction = u * EngineForce
 * speed = sqrt(v.x * v.x + v.y * v.y)
 * Fdrag = -Cdrag * v * speed (est.)
 * Frr = -Crr * v
 * Flong = Ftraction + Fdrag + Frr
 * a = F / M // F is net force on car (Flong + Flat)
 * v = v + (a * dt)
 * p = p + (v * dt)
 * Fbraking = -u * Cbraking
 */
struct Vehicle
{
    float engineForce {};
    float drag {};
    float engineBrakingTorque {}; // Nm at engine shaft, applied through drivetrain when off-throttle
    float engineBrakingSlip {};   // Fraction of free-roll ω removed when off-throttle
    float rr {}; // rolling resistance
    float M {}; // mass in KG (curb)
    glm::vec3 v {}; // velocity (m/s)
    glm::vec3 u {}; // direction
    float differentialRatio {};
    int gears {};
    float reverseGearRatio {};
    std::vector<float> gearRatios {};       // forward gears only: [0]=1st, [1]=2nd, etc.
    float wheelAngularVelocity {0.0f};      // ω, rad/s for drive (rear) axle
    float tractionConstant {};              // C_t: slip ratio → traction force
    float wheelInertia {};                  // kg·m², rear axle (both wheels)
    float transmissionEfficiency {0.85f};
    float wheelRadius {};
    float weight {}; // Newtons
    float ForceMax {};
    float wheelBase {};
    float carHeight {};
    float b {}; // Distance from CG (Center of Gravity) to Front Axle
    float c {}; // Distance from CG to Rear Axle
    float h {};
    float tyreFrictionCoefficient {};
    float g {9.8f}; // Gravity
    std::unique_ptr<Curve> torqueCurve {nullptr};
    std::unique_ptr<Curve> horsepowerCurve {nullptr};
    int activeGear {0};
    
    Vehicle(const VehicleData& data)
    {
        engineForce = data.engineForce;
        drag = data.drag;
        rr = data.drag * 30;
        M = data.mass;
        differentialRatio = data.differentialRatio;
        gears = data.gears;
        reverseGearRatio = data.reverseGearRatio;
        gearRatios = data.gearRatios; // forward gears: [0]=1st, [1]=2nd, etc.
        wheelRadius = data.wheelRadius;
        weight = data.mass * g;
        tyreFrictionCoefficient = data.tyreFrictionCoefficient;
        ForceMax = data.tyreFrictionCoefficient * weight;
        wheelBase = data.wheelBase;
        // Weight distribution: 43% front / 57% rear (typical rear-biased RWD layout)
        b = wheelBase * 0.57f; 
        c = wheelBase * 0.43f;
        h = data.height;
        torqueCurve = std::make_unique<Curve>(data.torqueCurvePath);
        horsepowerCurve = std::make_unique<Curve>(data.horsepowerCurvePath);
        // Engine braking torque from BMEP: T = BMEP × V_d / 4π  (4-stroke engine)
        // This gives peak engine torque (~489 Nm). Use ~15% of that as off-throttle resistance
        // (pumping losses + friction when the throttle plate is closed).
        engineBrakingTorque = (data.bmep * data.engineDisplacement / (4.0f * glm::pi<float>())) * 0.15f;

        // Wheel inertia: solid cylinder (I = ½·m·r²) for both rear wheels (~20 kg each)
        // plus drivetrain/axle contribution. Low inertia causes instant wheelspin.
        const float wheelMass = 20.0f;
        const float pureWheelInertia = 2.0f * (0.5f * wheelMass * wheelRadius * wheelRadius);
        const float drivetrainInertia = 4.0f; // kg·m² — driveshaft, diff, half-shafts
        wheelInertia = pureWheelInertia + drivetrainInertia;
        tractionConstant = data.tractionConstant;
        engineBrakingSlip = data.engineBrakingSlip;
    }

    /// Returns the gear ratio for a given gear: -1=reverse (negative), 0=neutral, 1..N=forward.
    [[nodiscard]] float GetGearRatio(int gear) const
    {
        if (gear == 0) return 0.0f;
        if (gear < 0)  return -reverseGearRatio;
        return gearRatios[static_cast<size_t>(gear) - 1];
    }

    /// SR = (ω·R − v_long) / max(|v_long|, |ω·R|, ε).
    /// Zero for free-rolling, −1 for locked-wheel braking, positive during acceleration.
    /// Using the larger of car speed and wheel surface speed as the reference avoids the
    /// constant-floor problem that weakens braking force at low speeds.
    [[nodiscard]] float SlipRatio() const
    {
        const float vLong      = glm::dot(v, u);
        const float wheelSpeed = wheelAngularVelocity * wheelRadius;
        const float refSpeed   = glm::max(glm::abs(vLong), glm::abs(wheelSpeed));
        const float denom      = glm::max(refSpeed, 0.001f); // epsilon prevents ÷0 at rest
        return (wheelSpeed - vLong) / denom;
    }

    /// F_traction = C_t * SR, clamped to ±MaxTraction (rear wheel weight only — drive wheels).
    [[nodiscard]] float TractionForceScalar() const
    {
        return glm::clamp(tractionConstant * SlipRatio(), -MaxTraction(), MaxTraction());
    }

    [[nodiscard]] glm::vec3 Drag() const { return -drag * v * glm::length(v); }
    [[nodiscard]] glm::vec3 RollingResistance() const { return -(drag * 30) * v; }
    [[nodiscard]] glm::vec3 Velocity() const { return v; }
    void SetVelocity(const glm::vec3 inV) { v = inV; }
    [[nodiscard]] float Speed() const { return glm::length(v); }
    [[nodiscard]] glm::vec3 Direction() const { return u; }
    [[nodiscard]] float Torque(const float rpm) const { return torqueCurve->GetValueAt(rpm); }
    [[nodiscard]] float Horsepower(const float rpm) const { return horsepowerCurve->GetValueAt(rpm); }
    [[nodiscard]] float DriveTorque(const float rpm, const int gear) const { return Torque(rpm) * GetGearRatio(gear) * differentialRatio * transmissionEfficiency; }
    [[nodiscard]] glm::vec3 DriveForce(const float rpm, const int gear) const { return u * DriveTorque(rpm, gear) / wheelRadius; }
    [[nodiscard]] float MaxTraction() const { return tyreFrictionCoefficient * (b / wheelBase) * M * g; }
    [[nodiscard]] bool HasWheelspin(const float rpm, const int gear) const { return glm::length(DriveForce(rpm, gear)) > MaxTraction(); }
    [[nodiscard]] float WheelTorque(const float rpm, const int gear) const { return Torque(rpm) * GetGearRatio(gear) * differentialRatio * transmissionEfficiency; }
    [[nodiscard]] float WheelRPM(const float rpm, const int gear) const
    {
        const float gr = GetGearRatio(gear);
        return (glm::abs(gr) > 0.001f) ? rpm / (gr * differentialRatio) : 0.0f;
    }

    /// Engine RPM derived from wheel angular velocity through the gear train.
    [[nodiscard]] float RPM() const
    {
        const float gRatio = glm::abs(GetGearRatio(activeGear));
        if (gRatio < 0.001f) return torqueCurve->GetMinT(); // neutral → idle
        const float rpm = glm::abs(wheelAngularVelocity) * gRatio * differentialRatio * 60.0f / glm::two_pi<float>();
        return glm::clamp(rpm, torqueCurve->GetMinT(), torqueCurve->GetMaxT());
    }

    // Solve drag·v² + rr·v - engineForce = 0  →  quadratic formula
    [[nodiscard]] float TopSpeed() const
    {
        return (-rr + glm::sqrt(rr * rr + 4.0f * drag * engineForce)) / (2.0f * drag);
    }
};
