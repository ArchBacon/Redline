#pragma once
#include <string>

#include "core/resource.hpp"

namespace bee
{

class Model;

class Image : public Resource
{
    friend class Resources;

public:
    enum class Format
    {
        kSrgb,
        kUnorm
    };

    static const char* FormatToString(Format format)
    {
        switch (format)
        {
            case Format::kSrgb:
                return "SRGB";
            case Format::kUnorm:
                return "UNORM";
            default:
                return "UNDEFINED";
        }
    }

    Image(const Model& model, int index, Format format);
    Image(FileIO::Directory directory, const std::string& path, Format format);
    virtual ~Image();
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Image(Image&&) = delete;
    Image& operator=(Image&&) = delete;

    /// Creates a texture from RGBA provided data
    void CreateGLTextureWithData(unsigned char* data, int width, int height, int channels, bool genMipMaps);
    unsigned int GetTextureId() const { return m_texture; }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    static std::string GetPath(const Model& model, int index, Format format);
    static std::string GetPath(FileIO::Directory directory, const std::string& path, Format format)
    {
        return Resource::GetPath(directory, path) + "_" + FormatToString(format);
    }

    unsigned int m_texture = 0;
    int m_width = -1;
    int m_height = -1;
    int m_channels = -1;
    Format m_format = Format::kUnorm;
};

}  // namespace bee
