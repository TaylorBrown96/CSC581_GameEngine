#include "utils/AssetManager.hpp"
#include <iostream>

namespace Engine {

bool AssetManager::loadBMP(Texture& out, SDL_Renderer* ren, const std::string& path) {
    if (!out.fromFile(ren, path.c_str())) {
        std::cerr << "Failed to load " << path << "\\n";
        return false;
    }
    return true;
}

} // namespace Engine
