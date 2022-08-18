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

SamplerState Samplers[]                                 : register(s0, space10);
SamplerComparisonState ComparsionSamplers[]             : register(s0, space11);

SamplerState gsamPointWrap : register(s0);
SamplerComparisonState gsamShadow : register(s1);

#endif