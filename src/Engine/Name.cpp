#include "Name.h"

namespace Engine
{
    Name::Name() : mId{ NameRegistry::INVALID_ID }
    {
    }

    Name::Name(const String& name)
    {
        mId = NameRegistry::Instance().GetId(name);
    }

    Name::Name(const char* name)
    {
        mId = NameRegistry::Instance().GetId(name);
    }

    Name::Name(const Name& other) : mId(other.mId)
    {

    }

    Name::Name(Name&& other) : mId(other.mId)
    {
        
    }

    Name& Name::operator=(const Name& other)
    {
        mId = other.mId;
        return *this;
    }

    Name& Name::operator=(Name&& other)
    {
        mId = other.mId;
        return *this;
    }

    const String& Name::string() const
    {
        return NameRegistry::Instance().GetName(mId);
    }

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
}