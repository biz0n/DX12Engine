#include "Resource.h"
#include <StringUtils.h>

namespace Engine::Memory
{
    Resource::Resource() : mResource(nullptr)
    {
    }

    Resource::~Resource()
    {
    }


    void Resource::SetName(const std::string &name)
    {
        if (mResource)
        {
            mResource->SetName(StringToWString(name).c_str());
        }
        mResourceName = name;
    }
} // namespace Engine::Memory