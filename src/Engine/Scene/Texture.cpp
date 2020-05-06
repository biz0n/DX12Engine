#include "Texture.h"

namespace Engine
{
    Texture::Texture(const std::wstring &name)
        : Resource(name)
    {
    }

    Texture::~Texture()
    {
    }

} // namespace Engine