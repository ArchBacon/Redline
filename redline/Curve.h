#pragma once

#include <map>
#include <string>

class Curve
{
    std::map<float, float> data {};
    
public:
    Curve() = default;
    Curve(const std::string& path);
    
    bool LoadCSV(const std::string& path);
    float GetValueAt(float t) const;
    float GetMinT() const;
    float GetMaxT() const;
};
