#include "EventTracker.h"

#include <pix3.h>

namespace Engine
{
    void EventTracker::StartGPUEvent(const std::string& eventName, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
    {
        PIXBeginEvent(commandList.Get(), PIX_COLOR_DEFAULT, "%s", eventName.c_str());
    }

    void EventTracker::StartGPUEvent(const std::string& eventName, Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue)
    {
        PIXBeginEvent(commandQueue.Get(), PIX_COLOR_DEFAULT, "%s", eventName.c_str());
    }

    void EventTracker::SetMarker(const std::string& eventName, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
    {
        PIXSetMarker(commandList.Get(), PIX_COLOR_DEFAULT, "%s", eventName.c_str());
    }

    void EventTracker::SetMarker(const std::string& eventName, Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue)
    {
        PIXSetMarker(commandQueue.Get(), PIX_COLOR_DEFAULT, "%s", eventName.c_str());
    }

    void EventTracker::EndGPUEvent(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
    {
        PIXEndEvent(commandList.Get());
    }

    void EventTracker::EndGPUEvent(Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue)
    {
        PIXEndEvent(commandQueue.Get());
    }

        void EventTracker::StartGPUEvent(const std::string& eventName, ID3D12GraphicsCommandList* commandList)
    {
        PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, "%s", eventName.c_str());
    }

    void EventTracker::StartGPUEvent(const std::string& eventName, ID3D12CommandQueue* commandQueue)
    {
        PIXBeginEvent(commandQueue, PIX_COLOR_DEFAULT, "%s", eventName.c_str());
    }

    void EventTracker::SetMarker(const std::string& eventName, ID3D12GraphicsCommandList* commandList)
    {
        PIXSetMarker(commandList, PIX_COLOR_DEFAULT, "%s", eventName.c_str());
    }

    void EventTracker::SetMarker(const std::string& eventName, ID3D12CommandQueue* commandQueue)
    {
        PIXSetMarker(commandQueue, PIX_COLOR_DEFAULT, "%s", eventName.c_str());
    }

    void EventTracker::EndGPUEvent(ID3D12GraphicsCommandList* commandList)
    {
        PIXEndEvent(commandList);
    }

    void EventTracker::EndGPUEvent(ID3D12CommandQueue* commandQueue)
    {
        PIXEndEvent(commandQueue);
    }
}
