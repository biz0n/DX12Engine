#pragma once

#include <Name.h>

namespace Engine
{
    class NameRegistry
    {
    public:
        static const uint32 INVALID_ID = UINT32_MAX;
        static NameRegistry &Instance();

    private:
        NameRegistry();
        ~NameRegistry();

    public:
        uint32 GetId(const String &name);
        const String &GetName(uint32 id) const;

    private:
        std::vector<String> mNames;
        std::unordered_map<String, uint32> mNameToId;
    };
} // namespace Engine
