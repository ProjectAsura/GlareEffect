//-------------------------------------------------------------------------------------------------
// File : LensGhostPS.hlsl
// Desc : Lens Ghost Shader.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Constant Values.
//-------------------------------------------------------------------------------------------------
static const float2 CENTER = float2(0.5f, 0.5f);    // �e�N�X�`�����S.


///////////////////////////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// CbLensGhost constant buffer.
///////////////////////////////////////////////////////////////////////////////////////////////////
cbuffer CbLensGhost
{
    float3 MultiplyColor : packoffset(c0);      // ��Z�J���[.
    float  Scale         : packoffset(c0.w);    // �e�N�X�`���X�P�[��.
};

//-------------------------------------------------------------------------------------------------
// Textures and Samplers.
//-------------------------------------------------------------------------------------------------
Texture2D       ColorBuffer  : register(t0);    // ���͉摜.
Texture2D       MaskBuffer   : register(t1);    // �}�X�N�摜.
SamplerState    ColorSampler : register(s0);    // ���j�A�T���v���[

//-------------------------------------------------------------------------------------------------
//      ���C���G���g���[�|�C���g�ł�.
//-------------------------------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float4 result = 0;

    float2 uv    = (input.TexCoord - 0.5) * Scale + float2(0.5f, 0.5f);
    float4 color = ColorBuffer.SampleLevel(ColorSampler, uv, 0);
    float  mask  = MaskBuffer .SampleLevel(ColorSampler, uv, 0).r;
    
    result.rgb = color.rgb * MultiplyColor * mask;
    result.a   = 1.0f;

    return result;
}