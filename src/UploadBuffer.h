#pragma once

#include "Types.h"
#include <d3d12.h>

class UploadBuffer
{
public:
    struct Allocation
    {
        BYTE* CPU;
        D3D12_GPU_VIRTUAL_ADDRESS GPU;

        template <typename T>
        void CopyTo(T* data)
        {
            memcpy(CPU, data, sizeof(T));
        }
    };
public:
    UploadBuffer(ID3D12Device* device, Size size);
    ~UploadBuffer();

    Allocation Allocate(Size sizeInBytes, Size alignment = 256);

    void Reset()
    {
        mOffset = 0;
    }

private:
    ComPtr<ID3D12Resource> mBuffer;
    BYTE* mMappedData;
    D3D12_GPU_VIRTUAL_ADDRESS mGpuAddress;
    Size mSize;
    Size mOffset;
};