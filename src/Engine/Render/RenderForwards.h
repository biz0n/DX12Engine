#pragma once

namespace Engine::Render
{
    class FrameResourceProvider;
    class PassRenderContext;
    class PipelineStateProvider;
    class PassCommandRecorder;
    
    class RenderContext;
    class UIRenderContext;
    class ResourcePlanner;
    class RootSignature;
    class RootSignatureBuilder;
    class RootSignatureProvider;
    class ShaderProvider;
    class FrameTransientContext;
    class RenderPassBase;
    template <class TPassData> class RenderPassBaseWithData;
    class Renderer;

    struct PipelineStateProxy;
    struct PipelineStateStream;
    struct ComputePipelineStateProxy;
    struct ComputePipelineStateStream;
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
