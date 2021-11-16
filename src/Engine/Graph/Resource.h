#pragma once

#include <Name.h>

namespace Engine::Graph
{
    class Resource
    {
        public:
            Resource(Name&& name);
        
        public:
            const Name& GetName() const { return mName; }
        
        private:
            Name mName;
    };
}