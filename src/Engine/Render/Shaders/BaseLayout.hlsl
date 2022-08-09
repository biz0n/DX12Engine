#ifndef BASELAYOUT_HLSL
#define BASELAYOUT_HLSL

#include "ShaderTypes.h"

struct Dummy
{
    int Padding;
};

#ifndef PassDataType
#define PassDataType Dummy
#endif

ConstantBuffer<PassDataType>   PassCB                   : register(b0, space10);
ConstantBuffer<FrameDataType>  FrameCB_                 : register(b1, space10);


Texture2D                Textures2D[]                   : register(t0, space10);
Texture2D<uint4>         Textures2D_UInt4               : register(t0, space11);
Texture3D                Textures3D[]                   : register(t0, space12);
Texture3D<uint4>         Textures3D_UIn4[]              : register(t0, space13);
Texture2DArray           Texture2DArrays[]              : register(t0, space14);
TextureCube              TextureCubes[]                 : register(t0, space15);
TextureCubeArray         TextureCubeArrays[]            : register(t0, space16);
ByteAddressBuffer        BindlessBuffers[]              : register(t0, space17);



RWTexture2D<float4>      RWTextures2D_Float4[]          : register(u0, space10);
RWTexture2D<uint4>       RWTextures2D_UInt4[]           : register(u0, space11);
RWTexture2D<uint>        RWTextures2D_UInt[]            : register(u0, space12);
RWTexture3D<float4>      RWTextures3D_Float4[]          : register(u0, space13);
RWTexture3D<uint4>       RWTextures3D_UInt4[]           : register(u0, space14);
RWTexture2DArray<float4> RWTexture2DArrays_Float4[]     : register(u0, space15);

SamplerState Samplers[]                                 : register(s0, space10);
SamplerComparisonState ComparsionSamplers[]             : register(s0, space11);

SamplerState gsamPointWrap : register(s0);
SamplerComparisonState gsamShadow : register(s1);

#endif