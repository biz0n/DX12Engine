#pragma once

#include <Types.h>
#include <d3d12.h>

#include <vector>

namespace Engine::Memory
{
    class UploadBuffer
    {
    public:
        struct Allocation
        {
            Byte *CPU;
            D3D12_GPU_VIRTUAL_ADDRESS GPU;
            Size bufferSize;
            Size offset;

            template <typename T>
            void CopyTo(T *data)
            {
                memcpy(CPU, data, sizeof(T));
            }

            template <typename T>
            void CopyTo(const std::vector<T> &v)
            {
                memcpy(CPU, v.data(), sizeof(T) * v.size());
            }
        };

    public:
        UploadBuffer(ID3D12Device *device, Size size);
        ~UploadBuffer();

        Allocation Allocate(Size sizeInBytes, Size alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

        void Reset()
        {
            mOffset = 0;
        }

        ID3D12Resource *GetD3D12Resource() const { return mBuffer.Get(); }

    private:
        ComPtr<ID3D12Resource> mBuffer;
        Byte *mMappedData;
        D3D12_GPU_VIRTUAL_ADDRESS mGpuAddress;
        Size mSize;
        Size mOffset;
    };

} // namespace Engine::Memory