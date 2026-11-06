#include "MaterialParameters.hlsli"
cbuffer MaterialBuffer : register(b4)
{
 float4 Tint = float4(1, 1, 1, 1);
 float Roughness = 0.5;
 float Metalness = 0.0;
 float EmissionStrength = 0.0;
 float Tiling = 1.0;
}
void Material_Vertex(inout MaterialVertexParameters p)
{
 p.UV0 *= Tiling;
}
void Material_Pixel(inout MaterialPixelParameters p)
{
 p.PixelColor *= Tint;
 p.SurfaceValues.g = Roughness;
 p.SurfaceValues.b = Metalness;
 p.Emission = float4(Tint.rgb * EmissionStrength, 1);
}
