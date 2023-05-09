#pragma once
#include <string>

namespace MapleLeaf {
class Version
{
public:
    uint8_t major, minor, patch;

    Version(uint8_t major, uint8_t minor, uint8_t patch)
        : major(major)
        , minor(minor)
        , patch(patch)
    {}
};

class App
{
    friend class Engine;

public:
    explicit App(std::string name, const Version& version = {1, 0, 0})
        : name(std::move(name))
        , version(version)
    {}

    virtual ~App() = default;

    virtual void Start() = 0;

    virtual void Update() = 0;

    const std::string& GetName() const { return name; }

    void SetName(const std::string& name) { this->name = name; }

    const Version& GetVersion() const { return version; }

    void SetVersion(const Version& version) { this->version = version; }

private:
    bool        started = false;
    std::string name;
    Version     version;
};
}   // namespace MapleLeaf