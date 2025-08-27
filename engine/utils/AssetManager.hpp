#pragma once
#include <string>
#include "render/Texture.hpp"

namespace Engine {

class AssetManager {
public:
    bool loadBMP(Texture& out, SDL_Renderer* ren, const std::string& path);
};

} // namespace Engine
