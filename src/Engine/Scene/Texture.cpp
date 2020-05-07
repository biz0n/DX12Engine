#include "Texture.h"

namespace Engine::Scene
{
    Texture::Texture(const std::wstring &name)
        : Resource(name)
    {
    }

    Texture::~Texture()
    {
    }

} // namespace Engine::Scene