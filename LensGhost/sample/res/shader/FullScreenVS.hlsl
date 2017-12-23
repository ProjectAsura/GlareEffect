//-------------------------------------------------------------------------------------------------
// File : FullScreenVS.hlsl
// Desc : Full Screen Triangle.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

struct VSInput
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

//-------------------------------------------------------------------------------------------------
//      メインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
VSOutput main(const VSInput input)
{
    VSOutput output;
    output.Position = float4(input.Position, 1.0f);
    output.TexCoord = input.TexCoord;

    return output;
}