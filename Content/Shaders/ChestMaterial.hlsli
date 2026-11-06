#include "MaterialParameters.hlsli"

cbuffer MaterialBuffer : register(b4)
{
    float4 MB_Tint = float4(1, 1, 1, 1);
    float MB_EmissionStrength = 8.0;
    float3 __EmissionPadding;
}

void Material_Vertex(inout MaterialVertexParameters aParameters)
{
}

void Material_Pixel(inout MaterialPixelParameters aParameters)
{
    aParameters.PixelColor *= MB_Tint;

    // The supplied chest has cyan inlays painted into albedo, with no separate mask.
    // Select those cool-colored texels while keeping wood and metal non-emissive.
    const float3 sourceColor = aParameters.Emission.rgb;
    const float cyanContrast = min(sourceColor.g, sourceColor.b) - sourceColor.r;
    const float inlayMask = smoothstep(0.04, 0.16, cyanContrast);
    aParameters.Emission = float4(sourceColor * inlayMask * MB_EmissionStrength * MB_Tint.rgb, 0);
}
