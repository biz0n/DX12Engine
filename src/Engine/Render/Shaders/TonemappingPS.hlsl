struct TonemapingPassCB
{
    uint InputTexIndex;
    uint OutputTexIndex;

    float2 Padding;
};

#define PassDataType TonemapingPassCB

#include "BaseLayout.hlsl"

[numthreads(16, 16, 1)]
void mainCS(int3 dispatchThreadId : SV_DispatchThreadId)
{
    Texture2D input = ResourceDescriptorHeap[PassCB.InputTexIndex];
    RWTexture2D<float4> output = ResourceDescriptorHeap[PassCB.OutputTexIndex];

    float4 color = input[dispatchThreadId.xy];
    float3 mapped = float3(1.0, 1.0, 1.0) - exp(-color.xyz * 5);

    color = pow(float4(mapped, 1.0), 1.0/2.2);

    output[dispatchThreadId.xy] = color;
}