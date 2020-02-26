#include "App.h"
#include <iostream>

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
    //mDSVDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);


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
}

void App::Update(const Timer &time)
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

   // mCommandList->SetDescriptorHeaps(1, &mSRVDescriptorHeap);

    //mCommandList->ClearDepthStencilView(mDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);


    // Specify the buffers we are going to render to.
    mCommandList->OMSetRenderTargets(1, &rtv, false, nullptr);

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

    mCurrentBackBufferIndex = (mCurrentBackBufferIndex + 1) % SwapChainBufferCount;

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

ComPtr<IDXGISwapChain> App::CreateSwapChain()
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

    return swapChain;
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