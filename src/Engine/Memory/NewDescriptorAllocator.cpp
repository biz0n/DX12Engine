#include "NewDescriptorAllocator.h"

#include <Exceptions.h>

namespace Engine::Memory
{
    NewDescriptorAllocator::NewDescriptorAllocator(ID3D12Device* device, DescriptorAllocatorConfig heapSizes) :
        mDevice { device },
        mRTHeap 
        {
            device, 
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 
            {
                heapSizes.RenderTargetHeapSize
            }
        },
        mDSHeap 
        {
            device, 
            D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 
            {
                heapSizes.DepthStencilHeapSize
            }
        },
        mSrvCbvUavHeap 
        {
            device, 
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 
            {
                heapSizes.ConstantBufferViewRange,
                heapSizes.ShaderResourceViewRange,
                heapSizes.UnorderedAccessViewRange
            }
        },
        mSamplerHeap 
        {
            device, 
            D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 
            {
                heapSizes.SamplerHeapSize
            }
        }
    {

    }
    
    NewDescriptorAllocator::~NewDescriptorAllocator() = default;

    Descriptor NewDescriptorAllocator::AllocateRTDescriptor(ID3D12Resource* resource, Index heapIndex, uint32 mipSlice)
    {
        auto resDesc = resource->GetDesc();
        D3D12_RENDER_TARGET_VIEW_DESC desc = {};
        
        switch (resDesc.Dimension)
        {
            case D3D12_RESOURCE_DIMENSION_BUFFER:
            {
                desc.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
                desc.Buffer.FirstElement = 0;
                desc.Buffer.NumElements = (uint32)resDesc.Width;
            }
            break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
            {
                if (resDesc.DepthOrArraySize > 1)
                {
                    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                    desc.Texture1DArray.FirstArraySlice = 0;
                    desc.Texture1DArray.ArraySize = resDesc.DepthOrArraySize;
                    desc.Texture1DArray.MipSlice = mipSlice;
                }
                else
                {
                    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                    desc.Texture1D.MipSlice = mipSlice;
                }
            }
            break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
            {
                if (resDesc.DepthOrArraySize > 1)
                {
                    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    desc.Texture2DArray.FirstArraySlice = 0;
                    desc.Texture2DArray.PlaneSlice = 0;
                    desc.Texture2DArray.ArraySize = resDesc.DepthOrArraySize;
                    desc.Texture2DArray.MipSlice = mipSlice;
                }
                else
                {
                    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                    desc.Texture2D.MipSlice = mipSlice;
                    desc.Texture2D.PlaneSlice = 0;
                }
            }
            break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
            {
                desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                desc.Texture3D.MipSlice = mipSlice;
                desc.Texture3D.FirstWSlice = 0;
                desc.Texture3D.WSize = -1;
            }
            break;
        }

        desc.Format = resDesc.Format;

        auto destCpu = mRTHeap.GetCpuAddress(heapIndex, 0);
        mDevice->CreateRenderTargetView(resource, &desc, destCpu);

        return Descriptor { destCpu, 0, heapIndex };
    }

    Descriptor NewDescriptorAllocator::AllocateDSDescriptor(ID3D12Resource* resource, Index heapIndex, uint32 mipSlice)
    {
        auto resDesc = resource->GetDesc();
        D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};

        switch (resDesc.Dimension)
        {
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
            {
                if (resDesc.DepthOrArraySize > 1)
                {
                    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                    desc.Texture1DArray.FirstArraySlice = 0;
                    desc.Texture1DArray.ArraySize = resDesc.DepthOrArraySize;
                    desc.Texture1DArray.MipSlice = mipSlice;
                }
                else
                {
                    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                    desc.Texture1D.MipSlice = mipSlice;
                }
            }
            break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
            {
                if (resDesc.DepthOrArraySize > 1)
                {
                    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    desc.Texture2DArray.FirstArraySlice = 0;
                    desc.Texture2DArray.ArraySize = resDesc.DepthOrArraySize;
                    desc.Texture2DArray.MipSlice = mipSlice;
                }
                else
                {
                    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                    desc.Texture2D.MipSlice = mipSlice;
                }
            }
            break;
        }

        desc.Format = resDesc.Format;

        auto destCpu = mDSHeap.GetCpuAddress(heapIndex, 0);
        mDevice->CreateDepthStencilView(resource, &desc, destCpu);

        return Descriptor { destCpu, 0, heapIndex };
    }

    Descriptor NewDescriptorAllocator::AllocateSRDescriptor(ID3D12Resource* resource, Index heapIndex, uint32 strideSize)
    {
        auto resDesc = resource->GetDesc();
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        
        switch (resDesc.Dimension)
        {
            case D3D12_RESOURCE_DIMENSION_BUFFER:
            {
                desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                desc.Buffer.FirstElement = 0;
                desc.Buffer.NumElements = (uint32)(resDesc.Width / strideSize);
                desc.Buffer.StructureByteStride = strideSize;
            }
            break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
            {
                if (resDesc.DepthOrArraySize > 1)
                {
                    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                    desc.Texture1DArray.FirstArraySlice = 0;
                    desc.Texture1DArray.ArraySize = resDesc.DepthOrArraySize;
                    desc.Texture1DArray.MostDetailedMip = 0;
                    desc.Texture1DArray.ResourceMinLODClamp = 0.0;
                    desc.Texture1DArray.MipLevels = resDesc.MipLevels;
                }
                else
                {
                    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                    desc.Texture1D.MipLevels = resDesc.MipLevels;
                    desc.Texture1D.MostDetailedMip = 0;
                    desc.Texture1D.ResourceMinLODClamp = 0.0;
                }
            }
            break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
            {
                if (resDesc.DepthOrArraySize > 1)
                {
                    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    desc.Texture2DArray.FirstArraySlice = 0;
                    desc.Texture2DArray.PlaneSlice = 0;
                    desc.Texture2DArray.ArraySize = resDesc.DepthOrArraySize;
                    desc.Texture2DArray.MipLevels = resDesc.MipLevels;
                    desc.Texture2DArray.MostDetailedMip = 0;
                    desc.Texture2DArray.ResourceMinLODClamp = 0.0;
                }
                else
                {
                    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                    desc.Texture2D.MipLevels = resDesc.MipLevels;
                    desc.Texture2D.PlaneSlice = 0;
                    desc.Texture2D.ResourceMinLODClamp = 0.0;
                    desc.Texture2D.MostDetailedMip = 0;
                }
            }
            break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
            {
                desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                desc.Texture3D.MipLevels = resDesc.MipLevels;
                desc.Texture3D.MostDetailedMip = 0;
                desc.Texture3D.ResourceMinLODClamp = 0.0;
            }
            break;
        }

        desc.Format = resDesc.Format;

        switch (desc.Format)
        {
            case DXGI_FORMAT_D16_UNORM:
                desc.Format = DXGI_FORMAT_R16_FLOAT;
            break;
            case DXGI_FORMAT_D24_UNORM_S8_UINT:
                desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            break;
            case DXGI_FORMAT_D32_FLOAT:
                desc.Format = DXGI_FORMAT_R32_FLOAT;
            break;
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
                desc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
            break;
        }

        auto destCpu = mSrvCbvUavHeap.GetCpuAddress(heapIndex, DescriptorAllocatorConfig::SRVRange);
        auto destGpu = mSrvCbvUavHeap.GetGpuAddress(heapIndex, DescriptorAllocatorConfig::SRVRange);
        mDevice->CreateShaderResourceView(resource, &desc, destCpu);

        return Descriptor { destCpu, destGpu, heapIndex };
    }

    Descriptor NewDescriptorAllocator::AllocateSRDescriptor(Index heapIndex, uint32 strideSize)
    {
        auto destCpu = mSrvCbvUavHeap.GetCpuAddress(heapIndex, DescriptorAllocatorConfig::SRVRange);
        auto destGpu = mSrvCbvUavHeap.GetGpuAddress(heapIndex, DescriptorAllocatorConfig::SRVRange);

        return Descriptor { destCpu, destGpu, heapIndex };
    }

    Descriptor NewDescriptorAllocator::AllocateSRCubeDescriptor(ID3D12Resource* resource, Index heapIndex)
    {
        auto resDesc = resource->GetDesc();
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        
        switch (resDesc.Dimension)
        {
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
            {
                if (resDesc.DepthOrArraySize > 6)
                {
                    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                    desc.TextureCubeArray.First2DArrayFace = 0;
                    desc.TextureCubeArray.MipLevels = resDesc.MipLevels;
                    desc.TextureCubeArray.MostDetailedMip = 0;
                    desc.TextureCubeArray.ResourceMinLODClamp = 0.0;
                    desc.TextureCubeArray.NumCubes = resDesc.DepthOrArraySize / 6;
                }
                else
                {
                    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                    desc.TextureCube.MipLevels = resDesc.MipLevels;
                    desc.TextureCube.ResourceMinLODClamp = 0.0;
                    desc.TextureCube.MostDetailedMip = 0;
                }
            }
            break;
        }

        desc.Format = resDesc.Format;
        
        auto destCpu = mSrvCbvUavHeap.GetCpuAddress(heapIndex, DescriptorAllocatorConfig::SRVRange);
        auto destGpu = mSrvCbvUavHeap.GetGpuAddress(heapIndex, DescriptorAllocatorConfig::SRVRange);
        mDevice->CreateShaderResourceView(resource, &desc, destCpu);

        return Descriptor { destCpu, destGpu, heapIndex };
    }

    Descriptor NewDescriptorAllocator::AllocateSRRaytracingASDescriptor(ID3D12Resource* resource, Index heapIndex)
    {
        auto resDesc = resource->GetDesc();

        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Format = resDesc.Format;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;

        desc.RaytracingAccelerationStructure.Location = resource->GetGPUVirtualAddress();

        auto destCpu = mSrvCbvUavHeap.GetCpuAddress(heapIndex, DescriptorAllocatorConfig::SRVRange);
        auto destGpu = mSrvCbvUavHeap.GetGpuAddress(heapIndex, DescriptorAllocatorConfig::SRVRange);
        mDevice->CreateShaderResourceView(nullptr, &desc, destCpu);

        return Descriptor { destCpu, destGpu, heapIndex };
    }

    Descriptor NewDescriptorAllocator::AllocateCBDescriptor(ID3D12Resource* resource, Index heapIndex, uint32 strideSize)
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
        desc.BufferLocation = resource->GetGPUVirtualAddress();
        desc.SizeInBytes = strideSize;

        auto destCpu = mSrvCbvUavHeap.GetCpuAddress(heapIndex, DescriptorAllocatorConfig::CBVRange);
        auto destGpu = mSrvCbvUavHeap.GetGpuAddress(heapIndex, DescriptorAllocatorConfig::CBVRange);
        mDevice->CreateConstantBufferView(&desc, destCpu);

        return Descriptor { destCpu, destGpu, heapIndex };
    }

    Descriptor NewDescriptorAllocator::AllocateUADescriptor(ID3D12Resource* resource, Index heapIndex, uint32 strideSize, uint32 mipSlice)
    {
        auto resDesc = resource->GetDesc();
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};

        switch (resDesc.Dimension)
        {
            case D3D12_RESOURCE_DIMENSION_BUFFER:
            {
                desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                desc.Buffer.FirstElement = 0;
                desc.Buffer.NumElements = (uint32)(resDesc.Width / strideSize);
                desc.Buffer.StructureByteStride = strideSize;
                desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
                desc.Buffer.CounterOffsetInBytes = 0;
            }
            break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
            {
                if (resDesc.DepthOrArraySize > 1)
                {
                    desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                    desc.Texture1DArray.FirstArraySlice = 0;
                    desc.Texture1DArray.ArraySize = resDesc.DepthOrArraySize;
                    desc.Texture1DArray.MipSlice = mipSlice;
                }
                else
                {
                    desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
                    desc.Texture1D.MipSlice = mipSlice;
                }
            }
            break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
            {
                if (resDesc.DepthOrArraySize > 1)
                {
                    desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                    desc.Texture2DArray.FirstArraySlice = 0;
                    desc.Texture2DArray.PlaneSlice = 0;
                    desc.Texture2DArray.ArraySize = resDesc.DepthOrArraySize;
                    desc.Texture2DArray.MipSlice = mipSlice;
                }
                else
                {
                    desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                    desc.Texture2D.MipSlice = mipSlice;
                    desc.Texture2D.PlaneSlice = 0;
                }
            }
            break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
            {
                desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                desc.Texture3D.MipSlice = mipSlice;
                desc.Texture3D.FirstWSlice = 0;
                desc.Texture3D.WSize = -1;
            }
            break;
        }

        desc.Format = resDesc.Format;

        auto destCpu = mSrvCbvUavHeap.GetCpuAddress(heapIndex, DescriptorAllocatorConfig::UAVRange);
        auto destGpu = mSrvCbvUavHeap.GetGpuAddress(heapIndex, DescriptorAllocatorConfig::UAVRange);
        mDevice->CreateUnorderedAccessView(resource, nullptr, &desc, destCpu);

        return Descriptor { destCpu, destGpu, heapIndex };
    }

    Descriptor NewDescriptorAllocator::AllocateSamplerDescriptor(const D3D12_SAMPLER_DESC& samplerDesc, Index heapIndex)
    {
        auto destCpu = mSamplerHeap.GetCpuAddress(heapIndex, 0);
        auto destGpu = mSamplerHeap.GetGpuAddress(heapIndex, 0);

        mDevice->CreateSampler(&samplerDesc, destCpu);

        return Descriptor { destCpu, destGpu, heapIndex };
    }
}