#pragma once

#include <glm/vec3.hpp>
#include <imgui/IconsFontAwesome.h>

#include "core/ecs.hpp"
#include "tools/inspectable.hpp"

struct Vehicle {};

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
    float engineForce = 3000.f;
    glm::vec3 direction = {};
    float speed = {};
    float drag = 0.528f;
    float rr = 15.84f; // drag * 30
    glm::vec3 traction = {};
    glm::vec3 velocity = {};
    
public:
    VehicleSystem();
    ~VehicleSystem() override = default;
    void Update(float dt) override;
    
    void OnPanel() override;
    [[nodiscard]] std::string GetName() const override { return "Vehicle Info"; }
    [[nodiscard]] std::string GetIcon() const override { return ICON_FA_AREA_CHART; }
};