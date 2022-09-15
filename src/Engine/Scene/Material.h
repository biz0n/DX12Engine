#pragma once

#include <Types.h>
#include <Bin3D/Material.h>
#include <Scene/SceneForwards.h>
#include <Memory/Texture.h>
#include <DirectXMath.h>

namespace Engine::Scene
{
    struct Material
    {
    public:
        void SetProperties(const Bin3D::MaterialProperties &properties) { mMaterialProperties = properties; }
        const Bin3D::MaterialProperties &GetProperties() const { return mMaterialProperties; }

        void SetBaseColorTexture(const SharedPtr<Memory::Texture> &texture) { mBaseColorTexture = texture; }
        void SetNormalTexture(const SharedPtr<Memory::Texture> &texture) { mNormalTexture = texture; }
        void SetMetallicRoughnessTexture(const SharedPtr<Memory::Texture> &texture) { mMetallicRoughnessTexture = texture; }
        void SetAmbientOcclusionTexture(const SharedPtr<Memory::Texture> &texture) { mAmbientOcclusionTexture = texture; }
        void SetEmissiveTexture(const SharedPtr<Memory::Texture> &texture) { mEmissiveTexture = texture; }

        SharedPtr<Memory::Texture> GetBaseColorTexture() const { return mBaseColorTexture; }
        SharedPtr<Memory::Texture> GetNormalTexture() const { return mNormalTexture; }
        SharedPtr<Memory::Texture> GetMetallicRoughnessTexture() const { return mMetallicRoughnessTexture; }
        SharedPtr<Memory::Texture> GetAmbientOcclusionTexture() const { return mAmbientOcclusionTexture; }
        SharedPtr<Memory::Texture> GetEmissiveTexture() const { return mEmissiveTexture; }

        bool HasBaseColorTexture() const { return mBaseColorTexture != nullptr; }
        bool HasNormalTexture() const { return mNormalTexture != nullptr; }
        bool HasMetallicRoughnessTexture() const { return mMetallicRoughnessTexture != nullptr; }
        bool HasAmbientOcclusionTexture() const { return mAmbientOcclusionTexture != nullptr; }
        bool HasEmissiveTexture() const { return mEmissiveTexture != nullptr; }

    private:
        Bin3D::MaterialProperties mMaterialProperties;
        SharedPtr<Memory::Texture> mBaseColorTexture;
        SharedPtr<Memory::Texture> mNormalTexture;
        SharedPtr<Memory::Texture> mMetallicRoughnessTexture;
        SharedPtr<Memory::Texture> mAmbientOcclusionTexture;
        SharedPtr<Memory::Texture> mEmissiveTexture;
    };

} // namespace Engine::Scene