#include "Name.h"

#include <NameRegistry.h>

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

    bool Name::isValid() const
    {
        return mId != NameRegistry::INVALID_ID;
    }

    const String& Name::string() const
    {
        return NameRegistry::Instance().GetName(mId);
    }

    const char* Name::c_str() const
    {
        return string().c_str();
    }
}