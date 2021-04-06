#pragma once

#include <wrl.h>
#include <string>

#include <d3d12.h>

namespace Engine
{
    class EventTracker
    {
    public:
        void StartGPUEvent(const std::string &eventName, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
        void StartGPUEvent(const std::string &eventName, Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue);
        void SetMarker(const std::string &eventName, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
        void SetMarker(const std::string &eventName, Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue);
        void StartGPUEvent(const std::string &eventName, ID3D12GraphicsCommandList* commandList);
        void StartGPUEvent(const std::string &eventName, ID3D12CommandQueue* commandQueue);
        void SetMarker(const std::string &eventName, ID3D12GraphicsCommandList* commandList);
        void SetMarker(const std::string &eventName, ID3D12CommandQueue* commandQueue);
        void EndGPUEvent(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
        void EndGPUEvent(Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue);
        void EndGPUEvent(ID3D12GraphicsCommandList* commandList);
        void EndGPUEvent(ID3D12CommandQueue* commandQueue);
    };
} // namespace Engine
