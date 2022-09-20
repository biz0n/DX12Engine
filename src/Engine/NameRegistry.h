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
        uint32 GetId(const std::string&name);
        const std::string& GetName(uint32 id) const;

    private:
        std::vector<std::string> mNames;
        std::unordered_map<std::string, uint32> mNameToId;
    };
} // namespace Engine
