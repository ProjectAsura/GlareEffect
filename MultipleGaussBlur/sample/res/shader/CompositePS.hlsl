//-------------------------------------------------------------------------------------------------
// File : CopyPS.hlsl
// Desc : Single Texture Fetch.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

//-------------------------------------------------------------------------------------------------
// Textures and Samplers.
//-------------------------------------------------------------------------------------------------
Texture2D       ColorBuffer0  : register(t0);
Texture2D       ColorBuffer1  : register(t1);
Texture2D       ColorBuffer2  : register(t2);
Texture2D       ColorBuffer3  : register(t3);
Texture2D       ColorBuffer4  : register(t4);
Texture2D       ColorBuffer5  : register(t5);
Texture2D       ColorBuffer6  : register(t6);
SamplerState    ColorSampler : register(s0);

float4 FetchColor(Texture2D map, float2 uv)
{
    float4 output = 0;
    output += map.SampleLevel(ColorSampler, uv, 0);
    return output;
}


//-------------------------------------------------------------------------------------------------
//      メインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float4 result = 0;
    result += FetchColor(ColorBuffer0, input.TexCoord);
    result += FetchColor(ColorBuffer1, input.TexCoord);
    result += FetchColor(ColorBuffer2, input.TexCoord);
    result += FetchColor(ColorBuffer3, input.TexCoord);
    result += FetchColor(ColorBuffer4, input.TexCoord);
    result += FetchColor(ColorBuffer5, input.TexCoord);
    result += FetchColor(ColorBuffer6, input.TexCoord);

    result.w = 1.0f;
    return result;
}
