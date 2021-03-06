#pragma once

#include <Types.h>
#include <Scene/SceneForwards.h>
#include <DirectXMath.h>

namespace Engine::Scene
{
    enum AlphaMode
    {
        Opaque,
        Mask,
        Blend,
    };

    struct TextureInfo
    {
        Index texCoord = 0;
    };

    struct NormalTextureInfo : TextureInfo
    {
        float32 scale = 1;
    };

    struct EmmissiveTextureInfo : TextureInfo
    {
        float32 strength = 1;
    };

    struct BaseColor
    {
        TextureInfo info;
        DirectX::XMFLOAT4 baseColor = {1, 1, 1, 1};
    };

    struct MetallicRaughness
    {
        TextureInfo info;
        float32 metallicFactor = 1;
        float32 roughnessFactor = 1;
    };

    struct Emissive
    {
        EmmissiveTextureInfo info;
        DirectX::XMFLOAT3 factor = {0, 0, 0};
    };

    struct MaterialProperties
    {
        BaseColor baseColor;
        MetallicRaughness metallicRaughness;
        NormalTextureInfo normalTextureInfo;
        Emissive emissive;
        AlphaMode alphaMode = AlphaMode::Opaque;
        float32 alphaCutoff = 0.5f;
        bool doubleSided = false;
        bool unlit = false;
    };

    class Material
    {
    public:
        Material();
        ~Material();

        void SetProperties(const MaterialProperties &properties) { mMaterialProperties = properties; }
        const MaterialProperties &GetProperties() const { return mMaterialProperties; }

        void SetBaseColorTexture(const SharedPtr<Texture> &texture) { mBaseColorTexture = texture; }
        void SetNormalTexture(const SharedPtr<Texture> &texture) { mNormalTexture = texture; }
        void SetMetallicRoughnessTexture(const SharedPtr<Texture> &texture) { mMetallicRoughnessTexture = texture; }
        void SetAmbientOcclusionTexture(const SharedPtr<Texture> &texture) { mAmbientOcclusionTexture = texture; }
        void SetEmissiveTexture(const SharedPtr<Texture> &texture) { mEmissiveTexture = texture; }

        const SharedPtr<Texture> GetBaseColorTexture() const { return mBaseColorTexture; }
        const SharedPtr<Texture> GetNormalTexture() const { return mNormalTexture; }
        const SharedPtr<Texture> GetMetallicRoughnessTexture() const { return mMetallicRoughnessTexture; }
        const SharedPtr<Texture> GetAmbientOcclusionTexture() const { return mAmbientOcclusionTexture; }
        const SharedPtr<Texture> GetEmissiveTexture() const { return mEmissiveTexture; }

        bool HasBaseColorTexture() const { return mBaseColorTexture != nullptr; }
        bool HasNormalTexture() const { return mNormalTexture != nullptr; }
        bool HasMetallicRoughnessTexture() const { return mMetallicRoughnessTexture != nullptr; }
        bool HasAmbientOcclusionTexture() const { return mAmbientOcclusionTexture != nullptr; }
        bool HasEmissiveTexture() const { return mEmissiveTexture != nullptr; }

    private:
        MaterialProperties mMaterialProperties;
        SharedPtr<Texture> mBaseColorTexture;
        SharedPtr<Texture> mNormalTexture;
        SharedPtr<Texture> mMetallicRoughnessTexture;
        SharedPtr<Texture> mAmbientOcclusionTexture;
        SharedPtr<Texture> mEmissiveTexture;
    };

} // namespace Engine::Scene