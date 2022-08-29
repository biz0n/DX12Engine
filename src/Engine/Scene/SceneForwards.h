#pragma once

namespace Engine::Scene::Loader
{
    class SceneLoader;
    class SceneDto;
}

namespace Engine::Scene
{
    class Camera;
    class CubeMap;
    class SceneObject;
    class Material;
    class Mesh;
    class PunctualLight;
    class Texture;
    class Image;
    class SceneStorage;
    class SceneToGPULoader;

    struct SceneLoadingInfo;
    struct Vertex;
    struct TextureInfo;
    struct NormalTextureInfo;
    struct EmmissiveTextureInfo;
    struct BaseColor;
    struct MetallicRaughness;
    struct Emissive;
    struct MaterialProperties;
}