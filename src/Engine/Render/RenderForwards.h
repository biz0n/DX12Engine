#pragma once

namespace Engine::Render
{
    class CommandQueue;
    class FrameResourceProvider;
    class Graphics;
    class PassContext;
    class PipelineStateProvider;
    class PassCommandRecorder;
    
    class RenderContext;
    class UIRenderContext;
    class ResourcePlanner;
    class GlobalResourceStateTracker;
    class ResourceStateTracker;
    class RootSignature;
    class RootSignatureBuilder;
    class RootSignatureProvider;
    class ShaderProvider;
    class SwapChain;
    class Texture;
    class FrameTransientContext;
    class RenderPassBase;
    template <class TPassData> class RenderPassBaseWithData;
    class Renderer;

    struct PipelineStateProxy;
    struct PipelineStateStream;
    struct ShaderCreationInfo;
    struct TextureCreationInfo;

    namespace Passes
    {
        class ForwardPass;
        class CubePass;
        class ToneMappingPass;
        class DepthPass;
    }
} // namespace Engine::Render
