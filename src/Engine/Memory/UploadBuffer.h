#pragma once

#include <Types.h>
#include <d3d12.h>

#include <vector>

#include <Memory/Buffer.h>
#include <Memory/ResourceAllocator.h>
#include <Render/RenderForwards.h>

namespace Engine::Memory
{
    class UploadBuffer : public Buffer
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

            void CopyTo(const void* data, Size size)
            {
                memcpy(CPU, data, size);
            }
        };

    public:
        UploadBuffer(ID3D12Device *device,
                     ResourceAllocator *resourceFactory,
                     DescriptorAllocatorPool *descriptorAllocator,
                     Engine::Render::GlobalResourceStateTracker* stateTracker,
                     Size size);
        ~UploadBuffer() override;

        Allocation Allocate(Size sizeInBytes, Size alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

        void Reset()
        {
            mOffset = 0;
        }

    private:
        Byte *mMappedData;
        D3D12_GPU_VIRTUAL_ADDRESS mGpuAddress;
        Size mSize;
        Size mOffset;
    };

} // namespace Engine::Memory