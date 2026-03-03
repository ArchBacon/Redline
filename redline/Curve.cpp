#include "Curve.h"
#include "csv.hpp"
#include "core/engine.hpp"
#include "core/fileio.hpp"
#include "tools/log.hpp"

Curve::Curve(const std::string& path)
{
    bee::Log::Info("creating curve...");
    
    LoadCSV(path);
}

bool Curve::LoadCSV(const std::string& path)
{
    const std::string fullPathName = bee::Engine.FileIO().GetPath(bee::FileIO::Directory::Assets, path);
    if (!bee::Engine.FileIO().Exists(bee::FileIO::Directory::Assets, path))
    {
        bee::Log::Error("Curve file \"{}\" does not exist.", path.c_str());
        return false;
    }
    
    // @see https://github.com/vincentlaucsb/csv-parser
    csv::CSVFormat format {};
    format.trim({' '});
    csv::CSVReader reader(fullPathName, format);
    for (const auto& row : reader)
    {
        const auto x = row["x"].get<float>();
        const auto y = row["y"].get<float>();
        
        
        data[x] = y;
    }
    
    return true;
}

float Curve::GetValueAt(const float t) const
{
    if (data.empty()) return 0.0f;
    
    // Fetch lowest and highest values to clamp to when T goes out of bounds
    const float lowest = data.begin()->first;
    if (t <= lowest) return data.begin()->second;
    
    const float highest = std::prev(data.end())->first;
    if (t >= highest) return std::prev(data.end())->second;

    // Interpolate the closest values to T to give the estimated closest value
    const auto next = data.upper_bound(t); // Find the first key that's larger then T
    const auto prev = std::prev(next); // Find the first key that's smaller then T
    
    const float x0 = prev->first;
    const float x1 = next->first;
    const float alpha = (t - x0) / (x1 - x0); // (v - min) / (max - min)
    
    const float y0 = prev->second;
    const float y1 = next->second;
    return y0 + alpha * (y1 - y0); // Lerp
}

float Curve::GetMinT() const
{
    return data.begin()->first;
}

float Curve::GetMaxT() const
{
    return std::prev(data.end())->first;
}
