#include "NameRegistry.h"

namespace Engine
{
    NameRegistry& NameRegistry::Instance()
    {
        static NameRegistry registry;
        return registry;
    }

    NameRegistry::NameRegistry()
    {

    }

    NameRegistry::~NameRegistry()
    {

    }

    uint32 NameRegistry::GetId(const String& name)
    {
        auto iter = mNameToId.find(name);
        if (iter != mNameToId.end())
        {
            return iter->second;
        }

        auto id = static_cast<uint32>(mNames.size());
        mNames.push_back(name);
        mNameToId[name] = id;

        return id;
    }

    const String& NameRegistry::GetName(uint32 id) const
    {
        return mNames.at(id);
    }
} // namespace Engine
