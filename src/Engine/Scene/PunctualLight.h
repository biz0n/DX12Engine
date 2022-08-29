#pragma once

#include <Render/ShaderTypes.h>

#include <DirectXMath.h>

namespace Engine::Scene
{
    enum LightType
    {
        DirectionalLight = DIRECTIONAL_LIGHT,
        PointLight = POINT_LIGHT,
        SpotLight = SPOT_LIGHT
    };

    class PunctualLight
    {
    public:
        PunctualLight()
         : mLightType(LightType::DirectionalLight)
         , mEnabled(false)
         , mColor({0.0f, 0.0f, 0.0f})
         , mIntensity(1)
         , mConstantAttenuation(0)
         , mLinearAttenuation(0)
         , mQuadraticAttenuation(0)
         , mInnerConeAngle(0)
         , mOuterConeAngle(0)
        {
        }
        void SetLightType(LightType lightType) { mLightType = lightType; }
        LightType GetLightType() const { return mLightType; }

        void SetEnabled(bool enabled) { mEnabled = enabled; }
        bool IsEnabled() const { return mEnabled; }

        void SetColor(const DirectX::XMFLOAT3& color) { mColor = color; }
        const DirectX::XMFLOAT3& GetColor() const { return mColor; }

        void SetIntensity(float intensity) { mIntensity = intensity; }
        float GetIntensity() const { return mIntensity; }

        void SetConstantAttenuation(float attenuation) { mConstantAttenuation = attenuation; }
        float GetConstantAttenuation() const { return mConstantAttenuation; }

        void SetLinearAttenuation(float attenuation) { mLinearAttenuation = attenuation; }
        float GetLinearAttenuation() const { return mLinearAttenuation; }

        void SetQuadraticAttenuation(float attenuation) { mQuadraticAttenuation = attenuation; }
        float GetQuadraticAttenuation() const { return mQuadraticAttenuation; }

        void SetInnerConeAngle(float coneAngle) { mInnerConeAngle = coneAngle; }
        float GetInnerConeAngle() const { return mInnerConeAngle; }

        void SetOuterConeAngle(float coneAngle) { mOuterConeAngle = coneAngle; }
        float GetOuterConeAngle() const { return mOuterConeAngle; }

    private:
        LightType mLightType;
        bool mEnabled;
        DirectX::XMFLOAT3 mColor;
        float mIntensity;
        float mConstantAttenuation;
        float mLinearAttenuation;
        float mQuadraticAttenuation;
        float mInnerConeAngle;
        float mOuterConeAngle;
    };
}