#include "Game.h"
#include "Utils.h"
#include <stdlib.h>
#include "ShaderTypes.h"
#include "RenderUtils.h"

#include "RootSignature.h"

#include "CommandListContext.h"

#include <DirectXTex.h>

#include <imgui/imgui.h>

namespace Engine
{
    Game::Game(App *app)
        : IGame(app)
    {
    }

    Game::~Game()
    {
    }

    static bool isInitializing;
    bool Game::Initialize()
    {
        isInitializing = true;
        auto commandList = Graphics().GetCommandList();

        SceneLoader loader;
        //loadedScene = loader.LoadScene("Resources\\Scenes\\sponza2\\sponza.obj", 0.03f);
        loadedScene = loader.LoadScene("Resources\\Scenes\\gltf2\\sponza\\sponza.gltf", 1.0f);
        //loadedScene = loader.LoadScene("Resources\\Scenes\\gltf2\\axis.gltf", 1.0f);
        //loadedScene = loader.LoadScene("Resources\\Scenes\\glTF-Sample-Models-master\\2.0\\MetalRoughSpheres\\glTF\\MetalRoughSpheres.gltf", 1.0f);
        //loadedScene = loader.LoadScene("Resources\\Scenes\\glTF-Sample-Models-master\\2.0\\MetalRoughSpheres\\glTF-Binary\\MetalRoughSpheres.glb", 1.0f);
        //loadedScene = loader.LoadScene("Resources\\Scenes\\glTF-Sample-Models-master\\2.0\\TextureSettingsTest\\glTF\\TextureSettingsTest.gltf", 1.0f);
        //loadedScene = loader.LoadScene("Resources\\Scenes\\glTF-Sample-Models-master\\2.0\\NormalTangentMirrorTest\\glTF\\NormalTangentMirrorTest.gltf", 1.0f);
        //loadedScene = loader.LoadScene("Resources\\Scenes\\glTF-Sample-Models-master\\2.0\\FlightHelmet\\glTF\\FlightHelmet.gltf", 1.0f);
        //loadedScene = loader.LoadScene("Resources\\Scenes\\glTF-Sample-Models-master\\2.0\\DamagedHelmet\\glTF\\DamagedHelmet.gltf", 1.0f);
        //loadedScene = loader.LoadScene("Resources\\Scenes\\glTF-Sample-Models-master\\2.0\\OrientationTest\\glTF\\OrientationTest.gltf", 1.0f);

        mImGuiManager = MakeUnique<ImGuiManager>(Graphics().GetDevice(), Graphics().SwapChainBufferCount, DXGI_FORMAT_R8G8B8A8_UNORM);

        mGlobalResourceStateTracker = Graphics().mGlobalResourceStateTracker;
        for (uint32 i = 0; i < Graphics().SwapChainBufferCount; ++i)
        {
            mResourceStateTrackers[i] = MakeShared<ResourceStateTracker>(mGlobalResourceStateTracker);

            mResourceStateTrackers[i]->TrackResource(Graphics().mBackBuffers[i].Get(), D3D12_RESOURCE_STATE_COMMON);
        }

        CommandListContext commandListContext;
        for (auto &node : loadedScene->nodes)
        {
            UploadMeshes(commandList, node.get(), commandListContext);
        }

        for (uint32 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
            uint32 incrementalSize = Graphics().GetDevice()->GetDescriptorHandleIncrementSize(type);
            mDescriptorAllocators[i] = MakeUnique<DescriptorAllocator>(type, incrementalSize);
        }

        mDepthBufferDescriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        auto cbvSrvUavDescriptorSize = Graphics().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        for (uint32 i = 0; i < Graphics().SwapChainBufferCount; ++i)
        {
            mDynamicDescriptorHeaps[i] = MakeShared<DynamicDescriptHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cbvSrvUavDescriptorSize);
        }

        for (auto &node : loadedScene->nodes)
        {
            for (auto &mesh : node->mMeshes)
            {
                if (mesh->material->HasAlbedoTexture())
                {
                    auto allocation = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                    Graphics().GetDevice()->CreateShaderResourceView(mesh->material->GetAlbedoTexture()->GetD3D12Resource().Get(), nullptr, allocation.GetDescriptor());

                    mesh->material->GetAlbedoTexture()->allocaion = allocation;
                }

                if (mesh->material->HasMetallicRoughnessTexture())
                {
                    auto allocation = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                    Graphics().GetDevice()->CreateShaderResourceView(mesh->material->GetMetallicRoughnessTexture()->GetD3D12Resource().Get(), nullptr, allocation.GetDescriptor());

                    mesh->material->GetMetallicRoughnessTexture()->allocaion = allocation;
                }

                if (mesh->material->HasNormalTexture())
                {
                    auto allocation = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                    Graphics().GetDevice()->CreateShaderResourceView(mesh->material->GetNormalTexture()->GetD3D12Resource().Get(), nullptr, allocation.GetDescriptor());

                    mesh->material->GetNormalTexture()->allocaion = allocation;
                }
            }
        }

        for (uint32 frameIndex = 0; frameIndex < Graphics().SwapChainBufferCount; ++frameIndex)
        {
            mUploadBuffer[frameIndex] = MakeUnique<UploadBuffer>(Graphics().GetDevice().Get(), 2 * 1024 * 1024);
        }

        ComPtr<ID3DBlob> pixelShaderBlob = Utils::CompileShader(L"Resources\\Shaders\\Shader.hlsl", nullptr, "mainPS", "ps_5_1");

        ComPtr<ID3DBlob> vertexShaderBlob = Utils::CompileShader(L"Resources\\Shaders\\Shader.hlsl", nullptr, "mainVS", "vs_5_1");

        auto inputLayout = Vertex::GetInputLayout();

        CD3DX12_DESCRIPTOR_RANGE1 texTable1;
        texTable1.Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            1,  // number of descriptors
            0); // register t0

        CD3DX12_DESCRIPTOR_RANGE1 texTable2;
        texTable2.Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            1,  // number of descriptors
            1); // register t1

        CD3DX12_DESCRIPTOR_RANGE1 texTable3;
        texTable3.Init(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            1,  // number of descriptors
            2); // register t2

        CD3DX12_ROOT_PARAMETER1 rootParameters[7];

        rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);

        rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

        rootParameters[2].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);

        rootParameters[3].InitAsShaderResourceView(0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);

        rootParameters[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

        rootParameters[5].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);

        rootParameters[6].InitAsDescriptorTable(1, &texTable3, D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(
            0,                               // shaderRegister
            D3D12_FILTER_ANISOTROPIC,        // filter
            D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
            D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
        rootSigDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler,
                             D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        mRootSignature = MakeUnique<RootSignature>(Graphics().GetDevice(), &rootSigDesc);
        struct PipelineStateStream
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_VS VS;
            CD3DX12_PIPELINE_STATE_STREAM_PS PS;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
            CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER Rasterizer;
        } pipelineStateStream;

        D3D12_RT_FORMAT_ARRAY rtvFormats = {};
        rtvFormats.NumRenderTargets = 1;
        rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

        CD3DX12_RASTERIZER_DESC rasterizer = {};
        rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizer.CullMode = D3D12_CULL_MODE_BACK;

        pipelineStateStream.pRootSignature = mRootSignature->GetD3D12RootSignature().Get();
        pipelineStateStream.InputLayout = {inputLayout.data(), static_cast<uint32>(inputLayout.size())};
        pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
        pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
        pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        pipelineStateStream.RTVFormats = rtvFormats;
        pipelineStateStream.Rasterizer = CD3DX12_RASTERIZER_DESC(rasterizer); //CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT{});

        D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
            sizeof(PipelineStateStream), &pipelineStateStream};
        ThrowIfFailed(Graphics().GetDevice()->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&mPipelineState)));

        uint64 fenceValue = Graphics().ExecuteCommandList(commandList);

        Graphics().WaitForFenceValue(fenceValue);

        mScreenViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(Graphics().GetWidth()), static_cast<float>(Graphics().GetHeight()));

        mScissorRect = CD3DX12_RECT(0, 0, Graphics().GetWidth(), Graphics().GetHeight());

        ResizeDepthBuffer(Graphics().GetWidth(), Graphics().GetHeight());

        commandListContext.ClearResources();

        isInitializing = false;
        return true;
    }

    void Game::KeyPressed(KeyEvent event)
    {
        if (event.State == KeyEvent::KeyState::Pressed)
        {
            keyState[event.Key] = true;
        }
        else if (event.State == KeyEvent::KeyState::Released)
        {
            keyState[event.Key] = false;
        }
    }

    void Game::UploadMeshes(ComPtr<ID3D12GraphicsCommandList> commandList, Node *node, CommandListContext &commandListContext)
    {
        for (auto &mesh : node->mMeshes)
        {
            RenderUtils::UploadVertexBuffer(Graphics().GetDevice(), commandList, mesh->mVertexBuffer, commandListContext);
            RenderUtils::UploadIndexBuffer(Graphics().GetDevice(), commandList, mesh->mIndexBuffer, commandListContext);

            if (mesh->material->HasAlbedoTexture())
            {
                RenderUtils::UploadTexture(Graphics().GetDevice(), commandList, *mesh->material->GetAlbedoTexture(), commandListContext);

                mGlobalResourceStateTracker->TrackResource(mesh->material->GetAlbedoTexture()->GetD3D12Resource().Get(), D3D12_RESOURCE_STATE_COPY_DEST);
            }

            if (mesh->material->HasMetallicRoughnessTexture())
            {
                RenderUtils::UploadTexture(Graphics().GetDevice(), commandList, *mesh->material->GetMetallicRoughnessTexture(), commandListContext);

                mGlobalResourceStateTracker->TrackResource(mesh->material->GetMetallicRoughnessTexture()->GetD3D12Resource().Get(), D3D12_RESOURCE_STATE_COPY_DEST);
            }

            if (mesh->material->HasNormalTexture())
            {
                RenderUtils::UploadTexture(Graphics().GetDevice(), commandList, *mesh->material->GetNormalTexture(), commandListContext);

                mGlobalResourceStateTracker->TrackResource(mesh->material->GetNormalTexture()->GetD3D12Resource().Get(), D3D12_RESOURCE_STATE_COPY_DEST);
            }
        }
    }

    void Game::Deinitialize()
    {
    }

    void Game::Update(const Timer &time)
    {
        mViewMatrix = mCamera.GetMatrix();

        float m_FoV = 45;
        float aspectRatio = Graphics().GetWidth() / static_cast<float>(Graphics().GetHeight());
        mProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_FoV), aspectRatio, 0.1f, 100.0f);

        const float32 speed = 5 * time.DeltaTime();
        const float32 rotationSpeed = 1.0f * time.DeltaTime();
        if (keyState[KeyCode::Key::Up])
        {
            mCamera.Rotate(0.0f, -rotationSpeed);
        }
        else if (keyState[KeyCode::Key::Down])
        {
            mCamera.Rotate(0.0f, +rotationSpeed);
        }

        if (keyState[KeyCode::Key::Left])
        {
            mCamera.Rotate(-rotationSpeed, 0.0f);
        }
        else if (keyState[KeyCode::Key::Right])
        {
            mCamera.Rotate(+rotationSpeed, 0.0f);
        }

        if (keyState[KeyCode::Key::W])
        {
            mCamera.Translate({0.0f, 0.0f, +speed});
        }
        else if (keyState[KeyCode::Key::S])
        {
            mCamera.Translate({0.0f, 0.0f, -speed});
        }

        if (keyState[KeyCode::Key::D])
        {
            mCamera.Translate({+speed, 0.0f, 0.0f});
        }
        else if (keyState[KeyCode::Key::A])
        {
            mCamera.Translate({-speed, 0.0f, 0.0f});
        }
    }

    void Game::Draw(ComPtr<ID3D12GraphicsCommandList> commandList, Node *node, UploadBuffer *buffer)
    {
        if (isInitializing)
        {
            return;
        }

        DirectX::XMMATRIX tWorld = DirectX::XMMatrixTranspose(node->GetWorldTransform());
        auto d = DirectX::XMMatrixDeterminant(tWorld);
        DirectX::XMMATRIX tWorldInverseTranspose = DirectX::XMMatrixInverse(&d, tWorld);
        tWorldInverseTranspose = DirectX::XMMatrixTranspose(tWorldInverseTranspose);
        MeshUniform cb;
        DirectX::XMStoreFloat4x4(&cb.World, tWorld);
        DirectX::XMStoreFloat4x4(&cb.InverseTranspose, tWorldInverseTranspose);

        auto cbAllocation = buffer->Allocate(sizeof(MeshUniform));
        cbAllocation.CopyTo(&cb);

        commandList->SetGraphicsRootConstantBufferView(0, cbAllocation.GPU);

        auto dynamicDescriptorHeap = mDynamicDescriptorHeaps[Graphics().GetCurrentBackBufferIndex()];
        auto resourceStateTracker = mResourceStateTrackers[Graphics().GetCurrentBackBufferIndex()];

        for (auto &mesh : node->mMeshes)
        {
            MaterialProperties properties = mesh->material->GetProperties();
            MaterialUniform uniform = RenderUtils::GetMaterialUniform(mesh->material.get());

            auto matAllocation = buffer->Allocate(sizeof(MaterialUniform));
            matAllocation.CopyTo(&uniform);
            commandList->SetGraphicsRootConstantBufferView(2, matAllocation.GPU);

            if (mesh->material->HasAlbedoTexture())
            {
                CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                    mesh->material->GetAlbedoTexture()->GetD3D12Resource().Get(),
                    D3D12_RESOURCE_STATE_COMMON,
                    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                resourceStateTracker->ResourceBarrier(barrier);

                dynamicDescriptorHeap->StageDescriptor(4, 0, 1, mesh->material->GetAlbedoTexture()->allocaion.GetDescriptor());
            }

            if (mesh->material->HasMetallicRoughnessTexture())
            {
                CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                    mesh->material->GetMetallicRoughnessTexture()->GetD3D12Resource().Get(),
                    D3D12_RESOURCE_STATE_COMMON,
                    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                resourceStateTracker->ResourceBarrier(barrier);

                dynamicDescriptorHeap->StageDescriptor(5, 0, 1, mesh->material->GetMetallicRoughnessTexture()->allocaion.GetDescriptor());
            }

            if (mesh->material->HasNormalTexture())
            {
                CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                    mesh->material->GetNormalTexture()->GetD3D12Resource().Get(),
                    D3D12_RESOURCE_STATE_COMMON,
                    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                resourceStateTracker->ResourceBarrier(barrier);

                dynamicDescriptorHeap->StageDescriptor(6, 0, 1, mesh->material->GetNormalTexture()->allocaion.GetDescriptor());
            }

            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            RenderUtils::BindVertexBuffer(commandList, mesh->mVertexBuffer);
            RenderUtils::BindIndexBuffer(commandList, mesh->mIndexBuffer);

            resourceStateTracker->FlushBarriers(commandList);

            dynamicDescriptorHeap->CommitStagedDescriptors(Graphics().GetDevice(), commandList);

            commandList->DrawIndexedInstanced(static_cast<uint32>(mesh->mIndexBuffer.GetElementsCount()), 1, 0, 0, 0);
        }
    }

    void Game::Draw(const Timer &time)
    {
        auto commandList = Graphics().GetCommandList();
        auto currentBackBufferIndex = Graphics().GetCurrentBackBufferIndex();

        mUploadBuffer[currentBackBufferIndex]->Reset();
        mDynamicDescriptorHeaps[Graphics().GetCurrentBackBufferIndex()]->Reset();
        auto resourceStateTracker = mResourceStateTrackers[Graphics().GetCurrentBackBufferIndex()];

        auto backBuffer = Graphics().GetCurrentBackBuffer();

        mImGuiManager->BeginFrame();

        static bool show_demo_window = true;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        auto rtv = Graphics().GetCurrentRenderTargetView();

        {
            CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                backBuffer.Get(),
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_RENDER_TARGET);

            resourceStateTracker->ResourceBarrier(barrier);

            resourceStateTracker->FlushBarriers(commandList);
        }

        FLOAT clearColor[] = {0.4f, 0.6f, 0.9f, 1.0f};

        commandList->RSSetViewports(1, &mScreenViewport);
        commandList->RSSetScissorRects(1, &mScissorRect);

        commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
        commandList->ClearDepthStencilView(mDepthBufferDescriptor.GetDescriptor(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        commandList->OMSetRenderTargets(1, &rtv, false, &mDepthBufferDescriptor.GetDescriptor());

        commandList->SetPipelineState(mPipelineState.Get());
        commandList->SetGraphicsRootSignature(mRootSignature->GetD3D12RootSignature().Get());

        mDynamicDescriptorHeaps[Graphics().GetCurrentBackBufferIndex()]->ParseRootSignature(mRootSignature.get());

        DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixMultiply(mViewMatrix, mProjectionMatrix);
        mvpMatrix = DirectX::XMMatrixTranspose(mvpMatrix);

        FrameUniform cb = {};
        DirectX::XMStoreFloat4x4(&cb.ViewProj, mvpMatrix);

        cb.EyePos = mCamera.GetPosition();

        std::vector<LightUniform> lights;
        lights.reserve(loadedScene->lights.size());
        for (uint32 i = 0; i < loadedScene->lights.size(); ++i)
        {
            LightNode *lightNode = loadedScene->lights[i].get();
            LightUniform light = RenderUtils::GetLightUniform(lightNode);

            lights.emplace_back(light);
        }

        cb.LightsCount = static_cast<uint32>(lights.size());

        auto cbAllocation = mUploadBuffer[currentBackBufferIndex]->Allocate(sizeof(FrameUniform));
        cbAllocation.CopyTo(&cb);

        commandList->SetGraphicsRootConstantBufferView(1, cbAllocation.GPU);

        auto lightsAllocation = mUploadBuffer[currentBackBufferIndex]->Allocate(lights.size() * sizeof(LightUniform), sizeof(LightUniform));
        memcpy(lightsAllocation.CPU, lights.data(), lights.size() * sizeof(LightUniform));

        commandList->SetGraphicsRootShaderResourceView(3, lightsAllocation.GPU);

        for (auto &node : loadedScene->nodes)
        {
            Draw(commandList, node.get(), mUploadBuffer[currentBackBufferIndex].get());
        }

        mImGuiManager->Draw(commandList);

        {
            CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                backBuffer.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PRESENT);

            resourceStateTracker->ResourceBarrier(barrier);
            //commandList->ResourceBarrier(1, &barrier);
        }

        resourceStateTracker->FlushBarriers(commandList);
        mFenceValues[currentBackBufferIndex] = Graphics().ExecuteCommandList(commandList);

        currentBackBufferIndex = Graphics().Present();

        Graphics().WaitForFenceValue(mFenceValues[currentBackBufferIndex]);
    }

    void Game::Resize(int32 width, int32 height)
    {
        mScreenViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float32>(width), static_cast<float32>(height));
        mScissorRect = CD3DX12_RECT(0, 0, width, height);

        ResizeDepthBuffer(width, height);
    }

    void Game::ResizeDepthBuffer(int32 width, int32 height)
    {
        Graphics().Flush();

        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = {1.0f, 0};

        ThrowIfFailed(Graphics().GetDevice()->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &optimizedClearValue,
            IID_PPV_ARGS(&mDepthBuffer)));

        D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
        dsv.Format = DXGI_FORMAT_D32_FLOAT;
        dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsv.Texture2D.MipSlice = 0;
        dsv.Flags = D3D12_DSV_FLAG_NONE;

        Graphics().GetDevice()->CreateDepthStencilView(
            mDepthBuffer.Get(),
            &dsv,
            mDepthBufferDescriptor.GetDescriptor());
    }

} // namespace Engine