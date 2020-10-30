#pragma once

#include <Scene/Node.h>
#include <ShaderTypes.h>
#include <Scene/PunctualLight.h>

#include <DirectXMath.h>

namespace Engine::Scene
{
	class LightNode : public Node
	{
	public:
		LightNode()
		 : Node()
		{
		}
		void SetPunctualLight(const PunctualLight& light) { mLight = light; }
		inline const PunctualLight& GetPunctualLight() const { return mLight; }

	private:
		PunctualLight mLight;
	};
}