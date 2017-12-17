//-------------------------------------------------------------------------------------------------
// File : StarPS.hlsl
// Desc : Blur Shader For Star.
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

///////////////////////////////////////////////////////////////////////////////////////////////////
// CbBlur buffer
///////////////////////////////////////////////////////////////////////////////////////////////////
cbuffer CbBlur : register(b0)
{
    float4 Offsets[16];
};

//-------------------------------------------------------------------------------------------------
// Textures and Samplers.
//-------------------------------------------------------------------------------------------------
Texture2D       ColorMap : register(t0);
SamplerState    ColorSmp : register(s0);


//-------------------------------------------------------------------------------------------------
//      ���C���G���g���[�|�C���g�ł�.
//-------------------------------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float4 result = 0;

    [unroll]
    for(int i=0; i<16; ++i)
    { result += Offsets[i].z * ColorMap.SampleLevel(ColorSmp, input.TexCoord + Offsets[i].xy, 0); }

    return result;
}