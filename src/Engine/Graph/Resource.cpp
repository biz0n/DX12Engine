#include "Resource.h"

namespace Engine::Graph
{
    Resource::Resource(Name name, int32 subresource) : Id{ name }, Subresource{ subresource }
    {

    }
}