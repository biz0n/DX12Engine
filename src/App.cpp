#include "App.h"
#include "Utils.h"
#include <iostream>

struct VertexPosColor
{
    XMFLOAT3 Vertex;
    XMFLOAT3 Color;
};

static VertexPosColor g_Vertices[8] = {
    {XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f)}, // 0
    {XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)},  // 1
    {XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f)},   // 2
    {XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)},  // 3
    {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f)},  // 4
    {XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f)},   // 5
    {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f)},    // 6
    {XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f)}    // 7
};

static uint16 g_Indicies[36] =
    {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7};

App::App()
{
    window = MakeUnique<Window>(800, 600, TEXT("d3d12"));
    graphics = MakeUnique<Graphics>();

    window->OnResize.Bind(&App::OnResize, this);
    window->OnActiveChanged.Bind(&App::OnActiveChanged, this);
}

App::~App()
{
    window->OnResize.Unbind(&App::OnResize, this);
    window->OnActiveChanged.Unbind(&App::OnActiveChanged, this);
}

int App::Run()
{
    Init();

    timer.Reset();

    while (true)
    {
        if (auto const returnCode = window->ProcessMessages())
        {
            Flush(mDirectCommandQueue, mFence, mFenceEvent, mFenceValue);
            ::CloseHandle(mFenceEvent);

            return *returnCode;
        }

        timer.Tick();

        if (timer.IsPaused())
        {
            Sleep(16);
        }
        else
        {
            Update(timer);
            Draw(timer);
        }
    }
}

void App::Init()
{
    mDirectCommandQueue = CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

    mSwapChain = CreateSwapChain();

    mCurrentBackBufferIndex = 0;

    mRTVDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SwapChainBufferCount);
    mRTVDescriptorSize = graphics->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    //mSRVDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
    mDSVDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

    D3D12_DESCRIPTOR_HEAP_DESC hd;
    hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    hd.NumDescriptors = 1;
    hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    hd.NodeMask = 0;

    ThrowIfFailed(graphics->GetDevice()->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&mSRVDescriptorHeap)));

    for (auto i = 0; i < SwapChainBufferCount; ++i)
    {
        mCommandAllocators[i] = CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
    }
    mCommandList = CreateCommandList(mCommandAllocators[mCurrentBackBufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT);

    mFence = CreateFence();
    mFenceEvent = CreateEventHandle();

    UpdateBackBuffers(mSwapChain, mRTVDescriptorHeap);

    mCommandList->Reset(mCommandAllocators[mCurrentBackBufferIndex].Get(), nullptr);

    ComPtr<ID3D12Resource> intermediateVertexResource;
    UpdateBufferResoure(
        mCommandList,
        &mVertexBuffer,
        &intermediateVertexResource,
        _countof(g_Vertices),
        sizeof(VertexPosColor),
        g_Vertices,
        D3D12_RESOURCE_FLAG_NONE);
    mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
    mVertexBufferView.SizeInBytes = sizeof(g_Vertices);
    mVertexBufferView.StrideInBytes = sizeof(VertexPosColor);

    ComPtr<ID3D12Resource> intermediateIndexResource;
    UpdateBufferResoure(
        mCommandList,
        &mIndexBuffer,
        &intermediateIndexResource,
        _countof(g_Indicies),
        sizeof(uint16),
        g_Indicies,
        D3D12_RESOURCE_FLAG_NONE);
    mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
    mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    mIndexBufferView.SizeInBytes = sizeof(g_Indicies);

    ComPtr<ID3DBlob> pixelShaderBlob;
    pixelShaderBlob = Utils::CompileShader(L"src\\Resources\\Shaders\\PixelShader.hlsl", nullptr, "main", "ps_5_1");

    ComPtr<ID3DBlob> vertexShaderBlob = Utils::CompileShader(L"src\\Resources\\Shaders\\VertexShader.hlsl", nullptr, "main", "vs_5_1");

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    CD3DX12_ROOT_PARAMETER rootParameters[1];

    rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, rootParameters, 0, nullptr,
                                            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                             serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char *)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(graphics->GetDevice()->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&mRootSignature)));

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    pipelineStateStream.pRootSignature = mRootSignature.Get();
    pipelineStateStream.InputLayout = {inputLayout, _countof(inputLayout)};
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream};
    ThrowIfFailed(graphics->GetDevice()->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&mPipelineState)));

    ThrowIfFailed(mCommandList->Close());

    ID3D12CommandList *const commandLists[] = {mCommandList.Get()};
    mDirectCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
    mFenceValues[mCurrentBackBufferIndex] = Signal(mDirectCommandQueue, mFence, mFenceValue);

    mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

    WaitForFenceValue(mFence, mFenceEvent, mFenceValues[mCurrentBackBufferIndex]);

    mScreenViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()));

    mScissorRect = CD3DX12_RECT(0, 0, window->GetWidth(), window->GetHeight());

    ResizeDepthBuffer(window->GetWidth(), window->GetHeight());
}

void App::Update(const Timer &time)
{
    float angle = static_cast<float>(time.TotalTime() * 90.0);
    const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
    mModelMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));

    const XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
    const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
    const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
    mViewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

    float m_FoV = 45;
    float aspectRatio = window->GetWidth() / static_cast<float>(window->GetHeight());
    mProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FoV), aspectRatio, 0.1f, 100.0f);
}

void App::Draw(const Timer &time)
{
    auto commandAllocator = mCommandAllocators[mCurrentBackBufferIndex];
    auto backBuffer = mBackBuffers[mCurrentBackBufferIndex];

    commandAllocator->Reset();
    mCommandList->Reset(commandAllocator.Get(), nullptr);

    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        mCommandList->ResourceBarrier(1, &barrier);
    }

    FLOAT clearColor[] = {0.4f, 0.6f, 0.9f, 1.0f};
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                                      mCurrentBackBufferIndex, mRTVDescriptorSize);

    mCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    mCommandList->ClearDepthStencilView(mDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    mCommandList->SetPipelineState(mPipelineState.Get());
    mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
    mCommandList->IASetIndexBuffer(&mIndexBufferView);

    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    XMMATRIX mvpMatrix = XMMatrixMultiply(mModelMatrix, mViewMatrix);
    mvpMatrix = XMMatrixMultiply(mvpMatrix, mProjectionMatrix);
    mCommandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

mCommandList->OMSetRenderTargets(1, &rtv, false, &mDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    mCommandList->DrawIndexedInstanced(_countof(g_Indicies), 1, 0, 0, 0);

    

    mCommandList->SetDescriptorHeaps(1, mSRVDescriptorHeap.GetAddressOf());

    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT);
        mCommandList->ResourceBarrier(1, &barrier);
    }

    ThrowIfFailed(mCommandList->Close());

    ID3D12CommandList *const commandLists[] = {mCommandList.Get()};
    mDirectCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    bool vSync = false;
    int32 syncInterval = vSync ? 1 : 0;
    int32 presentFlags = graphics->IsTearingSupported() && !vSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    ThrowIfFailed(mSwapChain->Present(syncInterval, presentFlags));

    mFenceValues[mCurrentBackBufferIndex] = Signal(mDirectCommandQueue, mFence, mFenceValue);

    mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

    WaitForFenceValue(mFence, mFenceEvent, mFenceValues[mCurrentBackBufferIndex]);
}

void App::OnResize(int32 width, int32 height)
{
    std::cout << "Width: " << width << " Height: " << height << std::endl;

    Flush(mDirectCommandQueue, mFence, mFenceEvent, mFenceValue);

    for (int i = 0; i < SwapChainBufferCount; ++i)
    {
        mBackBuffers[i].Reset();
        mFenceValues[i] = mFenceValues[mCurrentBackBufferIndex];
    }

    DXGI_SWAP_CHAIN_DESC desc = {};
    ThrowIfFailed(mSwapChain->GetDesc(&desc));
    ThrowIfFailed(mSwapChain->ResizeBuffers(SwapChainBufferCount, width, height, desc.BufferDesc.Format, desc.Flags));

    mCurrentBackBufferIndex = 0;

    App::UpdateBackBuffers(mSwapChain, mRTVDescriptorHeap);

    mScreenViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

    mScissorRect = CD3DX12_RECT(0, 0, width, height);

    ResizeDepthBuffer(width, height);
}

void App::OnActiveChanged(bool isActive)
{
    if (isActive)
    {
        timer.Start();
    }
    else
    {
        timer.Stop();
    }
}

ComPtr<ID3D12CommandQueue> App::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandQueue> commandQueue;

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(graphics->GetDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue)));

    return commandQueue;
}

ComPtr<IDXGISwapChain4> App::CreateSwapChain()
{
    ComPtr<IDXGISwapChain> swapChain;

    bool isTearingSupported = graphics->IsTearingSupported();
    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = window->GetWidth();
    sd.BufferDesc.Height = window->GetHeight();

    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    sd.SampleDesc = {1, 0};

    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = SwapChainBufferCount;
    sd.Windowed = true;

    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.OutputWindow = window->GetHWnd();
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | (isTearingSupported ? DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);

    ThrowIfFailed(graphics->GetGIFactory()->CreateSwapChain(mDirectCommandQueue.Get(), &sd, swapChain.GetAddressOf()));

    ThrowIfFailed(graphics->GetGIFactory()->MakeWindowAssociation(window->GetHWnd(), DXGI_MWA_NO_ALT_ENTER));

    ComPtr<IDXGISwapChain4> swapChain4;

    ThrowIfFailed(swapChain.As(&swapChain4));

    return swapChain4;
}

ComPtr<ID3D12DescriptorHeap> App::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptors)
{
    ComPtr<ID3D12DescriptorHeap> descriptorHeap;

    D3D12_DESCRIPTOR_HEAP_DESC hd;
    hd.Type = type;
    hd.NumDescriptors = numDescriptors;
    hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hd.NodeMask = 0;

    ThrowIfFailed(graphics->GetDevice()->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&descriptorHeap)));
    return descriptorHeap;
}

void App::UpdateBackBuffers(ComPtr<IDXGISwapChain> swapChain, ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap)
{
    auto rtvDescriptorSize = graphics->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    for (uint32 i = 0; i < SwapChainBufferCount; ++i)
    {
        ComPtr<ID3D12Resource> backBuffer;
        ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
        graphics->GetDevice()->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

        mBackBuffers[i] = backBuffer;
        rtvHandle.Offset(rtvDescriptorSize);
    }
}

ComPtr<ID3D12CommandAllocator> App::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandAllocator> allocator;

    ThrowIfFailed(graphics->GetDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&allocator)));

    return allocator;
}

ComPtr<ID3D12GraphicsCommandList> App::CreateCommandList(ComPtr<ID3D12CommandAllocator> allocator, D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12GraphicsCommandList> commandList;

    ThrowIfFailed(graphics->GetDevice()->CreateCommandList(0, type, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

    ThrowIfFailed(commandList->Close());

    return commandList;
}

ComPtr<ID3D12Fence> App::CreateFence()
{
    ComPtr<ID3D12Fence> fence;

    ThrowIfFailed(graphics->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    return fence;
}

HANDLE App::CreateEventHandle()
{
    HANDLE fenceEvent;

    fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

    return fenceEvent;
}

uint64 App::Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64 &fenceValue)
{
    auto fenceValueForSignal = ++fenceValue;

    ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValueForSignal));

    return fenceValueForSignal;
}

void App::WaitForFenceValue(ComPtr<ID3D12Fence> fence, HANDLE fenceEvent, uint64 fenceValue)
{
    if (fence->GetCompletedValue() < fenceValue)
    {
        fence->SetEventOnCompletion(fenceValue, fenceEvent);
        ::WaitForSingleObject(fenceEvent, INFINITE);
    }
}

void App::Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, HANDLE fenceEvent, uint64 &fenceValue)
{
    uint64 fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
    WaitForFenceValue(fence, fenceEvent, fenceValueForSignal);
}

void App::UpdateBufferResoure(
    ComPtr<ID3D12GraphicsCommandList> commandList,
    ID3D12Resource **destinationResource,
    ID3D12Resource **intermediateResource,
    Size numElements,
    Size elementSize,
    const void *data,
    D3D12_RESOURCE_FLAGS flags)
{
    Size bufferSize = numElements * elementSize;

    ThrowIfFailed(graphics->GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(destinationResource)));

    ThrowIfFailed(graphics->GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(intermediateResource)));

    D3D12_SUBRESOURCE_DATA subresource;
    subresource.pData = data;
    subresource.SlicePitch = bufferSize;
    subresource.RowPitch = bufferSize;

    UpdateSubresources(commandList.Get(), *destinationResource, *intermediateResource, 0, 0, 1, &subresource);
}

void App::ResizeDepthBuffer(int32 width, int32 height)
{
    Flush(mDirectCommandQueue, mFence, mFenceEvent, mFenceValue);

    D3D12_CLEAR_VALUE optimizedClearValue = {};
    optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    optimizedClearValue.DepthStencil = {1.0f, 0};

    ThrowIfFailed(graphics->GetDevice()->CreateCommittedResource(
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

    graphics->GetDevice()->CreateDepthStencilView(
        mDepthBuffer.Get(),
        &dsv,
        mDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}