#include "Engine.hpp"

namespace MapleLeaf {
Engine* Engine::Instance = nullptr;

Engine::Engine(std::string argv0)
    : argv0(std::move(argv0))
{
    Instance = this;
}

Engine::~Engine() {}
}   // namespace MapleLeaf