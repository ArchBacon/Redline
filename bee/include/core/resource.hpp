#pragma once

#include <string>
#include "core/engine.hpp"
#include "core/fileio.hpp"

namespace bee
{

enum class ResourceType
{
    Image,
    Shader,
    Model,
    Mesh,
    Vibration,
    Font
};

/// <summary>
/// Base class to all resources, gives book-keeping option to the Resources class
/// </summary>
class Resource
{
    friend class Resources;

public:
    /// <summary>
    /// Get the id for times where is better to use it instead of a
    /// pointer to the resource.
    /// </summary>
    size_t ResourceID() const { return m_ID; }

    /// <summary>
    /// Retrieve the type of this resource, as set by constructor.
    /// @returns the resource type.
    /// </summary>
    ResourceType GetType() const { return m_type; }

    /// <summary>
    /// Returns the path to this resource.
    /// @returns the resource path.
    /// </summary>
    const std::string& GetPath() const { return m_path; }

    /// <summary>
    /// Returns true if the resource has been generated, rather than loaded
    /// </summary>
    bool Generated() const { return m_generated; }

    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    Resource(Resource&&) = delete;
    Resource& operator=(Resource&&) = delete;

protected:
    /// <summary>
    /// Protected constructor: creation/deletion of resources is handled by the (befriended) Resources class.
    /// @param type The resource type of this resource.
    /// </summary>
    Resource(ResourceType type) : m_ID(0), m_type(type) {}

    /// <summary>
    /// Protected destructor: creation/deletion of resources is handled by the (befriended) Resources class.
    /// Non-virtual because of CppCoreGuidelines C.35
    /// https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-dtor-virtual.
    /// With raw pointers, this would require 'virtual' to ensure the chain of destructors: ~Derived > ~Base.
    /// Fortunately, Resources uses 'shared_ptr' takes care of
    /// calling the full chain even when the destructor is non-virtual.
    /// See the example: https://en.cppreference.com/w/cpp/memory/shared_ptr.html#Example
    /// </summary>
    ~Resource() = default;

    /// <summary>
    /// Returns the path for this (type of) resource. This is used by the
    /// resource  to find the resources of different types. If the
    /// type needs a different path, override this function.
    /// @returns The resource path.
    /// </summary>
    static std::string GetPath(FileIO::Directory directory, const std::string& path)
    {
        return Engine.FileIO().GetPath(directory, path);
    }

    /// <summary>
    /// Reload, not mandatory to handle this by all resources at current. Used
    /// for runtime reloading of assets.
    /// </summary>
    virtual void Reload() {}

    /// <summary>
    ///  Used when generating resources, to keep track of how many resources
    ///  have been generated and give them a unique ID.
    /// </summary>
    static size_t GetNextGeneratedID() { return m_nextGeneratedID++; }

    /// This is how the  tracks the resources
    size_t m_ID;

    /// Saves the path for runtime reloading of assets
    std::string m_path;

    /// The type of this resource, as set by the constructor.
    ResourceType m_type;

    /// The directory this resource was loaded from
    FileIO::Directory m_directory = FileIO::Directory::None;

    /// True if the resource has been generated, rather than loaded
    bool m_generated = false;

private:
    /// Count how many resources have been generated
    /// Initialized to 0 in the Resources cpp file
    static size_t m_nextGeneratedID;
};

}  // namespace bee