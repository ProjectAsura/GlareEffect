//---------------------------------------------------------------------------------------
// File : SampleApp.cpp
// Desc : Sample Application
// Copyright(c) Project Asura. All right reserved.
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------
#include "SampleApp.h"
#include <asdxLog.h>
#include <asdxShader.h>
#include <asdxTimer.h>


namespace {

#include "../res/shader/Compiled/FullScreenVS.inc"
#include "../res/shader/Compiled/CopyPS.inc"
#include "../res/shader/Compiled/CompositePS.inc"
#include "../res/shader/Compiled/LensGhostPS.inc"
#include "../res/shader/Compiled/GaussBlurPS.inc"


//////////////////////////////////////////////////////////////////////////////////////////
// LensGhostParam structure
//////////////////////////////////////////////////////////////////////////////////////////
ASDX_ALIGN(16)
struct LensGhostParam
{
    asdx::Vector4   MultiplyColor;   //!< 乗算カラーです.
};

//////////////////////////////////////////////////////////////////////////////////////////
// GaussBlurParam structure
//////////////////////////////////////////////////////////////////////////////////////////
ASDX_ALIGN(16)
struct GaussBlurParam
{
    asdx::Vector4   Offset[15];    //!< オフセットです.
};

//-------------------------------------------------------------------------------------------------
//      ガウスの重みを計算します.
//-------------------------------------------------------------------------------------------------
inline float GaussianDistribution( const asdx::Vector2& pos, float rho )
{
    return exp( -( pos.x * pos.x + pos.y * pos.y ) / (2.0f * rho * rho ));
}

//-------------------------------------------------------------------------------------------------
//      ブラーパラメータを計算します.
//-------------------------------------------------------------------------------------------------
inline GaussBlurParam CalcBlurParam( int width, int height, asdx::Vector2 dir, float deviation )
{
    GaussBlurParam result;
    auto tu = 1.0f / float(width);
    auto tv = 1.0f / float(height);

    result.Offset[0].z = GaussianDistribution(asdx::Vector2(0.0f, 0.0f), deviation);
    auto total_weight = result.Offset[0].z;

    result.Offset[0].x = 0.0f;
    result.Offset[0].y = 0.0f;

    for(auto i=1; i<8; ++i)
    {
        result.Offset[i].x = dir.x * i * tu;
        result.Offset[i].y = dir.y * i * tv;
        result.Offset[i].z = GaussianDistribution( dir * float(i), deviation );
        result.Offset[i].w = 0.0f;
        total_weight += result.Offset[i].z * 2.0f;
    }

    for(auto i=0; i<8; ++i)
    {
        result.Offset[i].z /= total_weight;
    }
    for(auto i=8; i<15; ++i)
    {
        result.Offset[i].x = -result.Offset[i - 7].x;
        result.Offset[i].y = -result.Offset[i - 7].y;
        result.Offset[i].z =  result.Offset[i - 7].z;
        result.Offset[i].w = 0.0f;
    }

    return result;
}

} // namespace 


/////////////////////////////////////////////////////////////////////////////////////////
// SampleApplication class
/////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
//      コンストラクタです.
//---------------------------------------------------------------------------------------
SampleApplication::SampleApplication()
: Application   ( "Lens Ghost", 960, 540 )
, m_Font        ()
{
}

//---------------------------------------------------------------------------------------
//      デストラクタです.
//---------------------------------------------------------------------------------------
SampleApplication::~SampleApplication()
{ /* DO_NOTHING */ }

//---------------------------------------------------------------------------------------
//      初期化時の処理です.
//---------------------------------------------------------------------------------------
bool SampleApplication::OnInit()
{
    if ( !m_Font.Init( m_pDevice, "../res/font/SetoMini-P.fnt", (float)m_Width, (float)m_Height ) )
    {
        ELOG( "Error : asdx::Font::Init() Failed." );
        return false;
    }

    HRESULT hr = S_OK;
    {
        // 頂点シェーダを生成.
        hr = m_pDevice->CreateVertexShader(FullScreenVS, sizeof(FullScreenVS), nullptr, &m_pFullScreenVS);
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreateVertexShader() Failed." );
            return false;
        }

        if (!m_Quad.Init(m_pDevice, FullScreenVS, sizeof(FullScreenVS)))
        { return false; }
    }

    {
        // ピクセルシェーダを生成.
        hr = m_pDevice->CreatePixelShader(CompositePS, sizeof(CompositePS), nullptr, &m_pCompositePS);
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11CreatePixelShader() Failed." );
            return false;
        }
    }

    {
        hr = m_pDevice->CreatePixelShader(CopyPS, sizeof(CopyPS), nullptr, &m_pCopyPS);
        if ( FAILED(hr) )
        {
            ELOG( "Error : ID3D11CreatePixelShader() Failed." );
            return false;
        }
    }

    {
        hr = m_pDevice->CreatePixelShader(LensGhostPS, sizeof(LensGhostPS), nullptr, &m_pLensGhostPS);
        if ( FAILED(hr) )
        {
            ELOG( "Error : ID3D11CreatePixelShader() Failed." );
            return false;
        }
    }


    {
        hr = m_pDevice->CreatePixelShader(GaussBlurPS, sizeof(GaussBlurPS), nullptr, &m_pGaussBlurPS);
        if ( FAILED(hr) )
        {
            ELOG( "Error : ID3D11CreatePixelShader() Failed." );
            return false;
        }
    }

    {
        if (!m_InputTexture.CreateFromFile( m_pDevice, "../res/texture/input.map"))
        {
            ELOG( "Error : Texture::CreateFromFile() Failed. ");
            return false;
        }
    }

    {
        if (!m_MaskTexture.CreateFromFile( m_pDevice, "../res/texture/mask.map" ))
        {
            ELOG( "Error : Texture::CreateFromFile() Failed." );
            return false;
        }
    
    }

   {
        D3D11_SAMPLER_DESC desc;
        ZeroMemory( &desc, sizeof(desc));
        desc.Filter             = D3D11_FILTER_MIN_MAG_MIP_POINT;
        desc.AddressU           = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV           = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW           = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.MipLODBias         = 0;
        desc.MaxAnisotropy      = 1;
        desc.ComparisonFunc     = D3D11_COMPARISON_NEVER;
        desc.BorderColor[ 0 ]   = 0.0f;
        desc.BorderColor[ 1 ]   = 0.0f;
        desc.BorderColor[ 2 ]   = 0.0f;
        desc.BorderColor[ 3 ]   = 0.0f;
        desc.MinLOD             = -3.402823466e+38F; // -FLT_MAX
        desc.MaxLOD             =  3.402823466e+38F; // FLT_MAX

        hr = m_pDevice->CreateSamplerState( &desc, &m_pPointSampler );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreateSamplerState() Failed." );
            return false;
        }
    }

   {
        D3D11_SAMPLER_DESC desc;
        ZeroMemory( &desc, sizeof(desc));
        desc.Filter             = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU           = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV           = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW           = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.MipLODBias         = 0;
        desc.MaxAnisotropy      = 1;
        desc.ComparisonFunc     = D3D11_COMPARISON_NEVER;
        desc.BorderColor[ 0 ]   = 0.0f;
        desc.BorderColor[ 1 ]   = 0.0f;
        desc.BorderColor[ 2 ]   = 0.0f;
        desc.BorderColor[ 3 ]   = 0.0f;
        desc.MinLOD             = -3.402823466e+38F; // -FLT_MAX
        desc.MaxLOD             =  3.402823466e+38F; // FLT_MAX

        hr = m_pDevice->CreateSamplerState( &desc, &m_pLinearClamp );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreateSamplerState() Failed." );
            return false;
        }
    }

   {
        D3D11_SAMPLER_DESC desc;
        ZeroMemory( &desc, sizeof(desc));
        desc.Filter             = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU           = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV           = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW           = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.MipLODBias         = 0;
        desc.MaxAnisotropy      = 1;
        desc.ComparisonFunc     = D3D11_COMPARISON_NEVER;
        desc.BorderColor[ 0 ]   = 0.0f;
        desc.BorderColor[ 1 ]   = 0.0f;
        desc.BorderColor[ 2 ]   = 0.0f;
        desc.BorderColor[ 3 ]   = 0.0f;
        desc.MinLOD             = -3.402823466e+38F; // -FLT_MAX
        desc.MaxLOD             =  3.402823466e+38F; // FLT_MAX

        hr = m_pDevice->CreateSamplerState( &desc, &m_pLinearWrap );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreateSamplerState() Failed." );
            return false;
        }
    }

   // 不透明.
   {
       D3D11_BLEND_DESC desc = {};
       desc.AlphaToCoverageEnable       = FALSE;
       desc.IndependentBlendEnable      = FALSE;
       desc.RenderTarget[0].BlendEnable = FALSE;
       desc.RenderTarget[0].BlendOp     = desc.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
       desc.RenderTarget[0].SrcBlend    = desc.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_ONE;
       desc.RenderTarget[0].DestBlend   = desc.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_ZERO;
       desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
       hr = m_pDevice->CreateBlendState(&desc, &m_pOpequeBS);
       if (FAILED(hr))
       {
           ELOG( "Error : ID3D11Device::CreateBlendState() Failed.");
           return false;
       }
   }

   // 加算.
   {
       D3D11_BLEND_DESC desc = {};
       desc.AlphaToCoverageEnable       = FALSE;
       desc.IndependentBlendEnable      = FALSE;
       desc.RenderTarget[0].BlendEnable = TRUE;
       desc.RenderTarget[0].BlendOp     = desc.RenderTarget[0].BlendOpAlpha     = D3D11_BLEND_OP_ADD;
       desc.RenderTarget[0].SrcBlend    = desc.RenderTarget[0].SrcBlendAlpha    = D3D11_BLEND_SRC_ALPHA;
       desc.RenderTarget[0].DestBlend   = desc.RenderTarget[0].DestBlendAlpha   = D3D11_BLEND_ONE;
       desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
       hr = m_pDevice->CreateBlendState(&desc, &m_pAdditiveBS);
       if (FAILED(hr))
       {
           ELOG( "Error : ID3D11Device::CreateBlendState() Failed.");
           return false;
       }
   }

   {
      if ( !m_LensGhostBuffer.Create(m_pDevice, sizeof(LensGhostParam)))
      { return false; }
   }

   {
       if ( !m_GaussBlurBuffer.Create(m_pDevice, sizeof(GaussBlurParam)) )
       { return false; }
   }

   {
       asdx::RenderTarget2D::Description desc;
       desc.Width               = m_Width;
       desc.Height              = m_Height;
       desc.MipLevels           = 1;
       desc.ArraySize           = 1;
       desc.Format              = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
       desc.SampleDesc.Count    = 1;
       desc.SampleDesc.Quality  = 0;

       for(auto i=0; i<2; i++)
       {
           if (!m_WorkBuffer[i].Create(m_pDevice, desc))
           { return false; }
       }

       desc.Width  = m_Width  / 4;
       desc.Height = m_Height / 4;
       for(auto i=2; i<4; i++)
       {
           if (!m_WorkBuffer[i].Create(m_pDevice, desc))
           { return false; }
       }
   }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了時の処理です.
//-------------------------------------------------------------------------------------------------
void SampleApplication::OnTerm()
{
    for(auto i=0; i<2; i++)
    { m_WorkBuffer[i].Release(); }
    m_Font.Term();
    m_InputTexture.Release();
    m_MaskTexture.Release();
    m_LensGhostBuffer.Release();
    m_GaussBlurBuffer.Release();
    m_Quad.Term();
    ASDX_RELEASE( m_pOpequeBS );
    ASDX_RELEASE( m_pAdditiveBS );
    ASDX_RELEASE( m_pPointSampler );
    ASDX_RELEASE( m_pLinearClamp );
    ASDX_RELEASE( m_pLinearWrap );
    ASDX_RELEASE( m_pLensGhostPS );
    ASDX_RELEASE( m_pCopyPS );
    ASDX_RELEASE( m_pCompositePS );
    ASDX_RELEASE( m_pFullScreenVS );
    ASDX_RELEASE( m_pGaussBlurPS );
}

//---------------------------------------------------------------------------------------
//      フレーム遷移時の処理です.
//---------------------------------------------------------------------------------------
void SampleApplication::OnFrameMove( double time, double elapsedTime )
{

}

//---------------------------------------------------------------------------------------
//      テキスト描画時の処理です.
//---------------------------------------------------------------------------------------
void SampleApplication::OnDrawText()
{
    m_Font.Begin( m_pDeviceContext );
    {
        m_Font.DrawStringArg( 10, 10, "FPS : %.2f", GetFPS() );
    }
    m_Font.End( m_pDeviceContext );
}

//---------------------------------------------------------------------------------------
//      描画時の処理です.
//---------------------------------------------------------------------------------------
void SampleApplication::OnFrameRender( double time, double elapsedTime )
{
    auto w = m_Width;
    auto h = m_Height;

    ID3D11ShaderResourceView*   pSrc = nullptr;
    ID3D11RenderTargetView*     pDst = nullptr;

    auto pMask = m_MaskTexture.GetSRV();

    ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };

    UINT  sampleMask     = D3D11_DEFAULT_SAMPLE_MASK;
    float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float clearColor[4]  = { 0.0f, 0.0f, 0.0f, 1.0f };
    auto deviation = 5.0f;

    // 入力画像にガウスブラーを掛ける,
    {
        pDst = m_WorkBuffer[2].GetRTV();
        pSrc = m_InputTexture.GetSRV();

        // クリア処理.
        m_pDeviceContext->ClearRenderTargetView( pDst, m_ClearColor );

        // 出力マネージャに設定.
        m_pDeviceContext->OMSetRenderTargets( 1, &pDst, nullptr );

        // ステートを設定.
        m_pDeviceContext->RSSetState( m_pRasterizerState );
        m_pDeviceContext->OMSetBlendState( m_pOpequeBS, blendFactor, sampleMask );
        m_pDeviceContext->OMSetDepthStencilState( m_pDepthStencilState, m_StencilRef );

        // シェーダの設定.
        m_pDeviceContext->VSSetShader( m_pFullScreenVS, nullptr, 0 );
        m_pDeviceContext->GSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->HSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->DSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->PSSetShader( m_pGaussBlurPS, nullptr, 0 );

        // シェーダリソースビューを設定.
        m_pDeviceContext->PSSetShaderResources( 0, 1, &pSrc );
        m_pDeviceContext->PSSetSamplers( 0, 1, &m_pLinearClamp );


        D3D11_VIEWPORT viewport;
        viewport.TopLeftX   = 0;
        viewport.TopLeftY   = 0;
        viewport.Width      = float(w) / 4.0f;
        viewport.Height     = float(h) / 4.0f;
        viewport.MinDepth   = 0.0f;
        viewport.MaxDepth   = 1.0f;

        m_pDeviceContext->RSSetViewports( 1, &viewport );

        auto pCB = m_GaussBlurBuffer.GetBuffer();
        GaussBlurParam src = CalcBlurParam(w, h, asdx::Vector2(1.0f, 0.0f), deviation);
        m_pDeviceContext->UpdateSubresource( pCB, 0, nullptr, &src, 0, 0 );
        m_pDeviceContext->PSSetConstantBuffers( 0, 1, &pCB );

        // 描画.
        m_Quad.Draw(m_pDeviceContext);
 
        pSrc = m_WorkBuffer[2].GetSRV();
        pDst = m_WorkBuffer[3].GetRTV();

       // クリア処理.
        m_pDeviceContext->ClearRenderTargetView( pDst, m_ClearColor );

        // 出力マネージャに設定.
        m_pDeviceContext->OMSetRenderTargets( 1, &pDst, nullptr );

        // シェーダリソースビューを設定.
        m_pDeviceContext->PSSetShaderResources( 0, 1, &pSrc );
        m_pDeviceContext->PSSetSamplers( 0, 1, &m_pLinearClamp );

        src = CalcBlurParam(w, h, asdx::Vector2(0.0f, 1.0f), deviation);
        m_pDeviceContext->UpdateSubresource( pCB, 0, nullptr, &src, 0, 0 );
        m_pDeviceContext->PSSetConstantBuffers( 0, 1, &pCB );

        // ステートを設定.
        m_pDeviceContext->RSSetState( m_pRasterizerState );
        m_pDeviceContext->OMSetBlendState( m_pOpequeBS, blendFactor, sampleMask );
        m_pDeviceContext->OMSetDepthStencilState( m_pDepthStencilState, m_StencilRef );

        // 描画.
        m_Quad.Draw(m_pDeviceContext);

        // シェーダリソースをクリア.
        ID3D11ShaderResourceView* nullTarget[1] = { nullptr };
        m_pDeviceContext->PSSetShaderResources( 0, 1, nullTarget );
    }

    // ゴーストを生成
    {
        // 乗算カラー / テクスチャスケール.
        asdx::Vector4 colors[] = {
            asdx::Vector4(1.0f,  0.5f,  0.6f, -1.2f),
            asdx::Vector4(0.3f,  1.0f,  0.6f, -1.2f),
            asdx::Vector4(0.6f,  0.35f, 1.0f, -1.5f),
            asdx::Vector4(1.0f,  0.6f,  0.3f, -1.75f),
            asdx::Vector4(0.25f, 1.0f,  0.7f, 1.2f),
            asdx::Vector4(0.5f,  0.9f,  1.0f, 1.35f),
            asdx::Vector4(0.3f,  1.0f,  0.5f, 1.5f),
            asdx::Vector4(0.7f,  0.5f,  1.0f, 2.0f),
        };

        pSrc = m_WorkBuffer[3].GetSRV();
        pDst = m_WorkBuffer[0].GetRTV();

        auto pCB = m_LensGhostBuffer.GetBuffer();

        // レンダーターゲット設定.
        m_pDeviceContext->ClearRenderTargetView( pDst, clearColor );
        m_pDeviceContext->OMSetRenderTargets( 1, &pDst, nullptr );

        // ブレンドステート設定.
        m_pDeviceContext->OMSetBlendState( m_pAdditiveBS, blendFactor, sampleMask );

        D3D11_VIEWPORT viewport;
        viewport.TopLeftX   = 0;
        viewport.TopLeftY   = 0;
        viewport.Width      = float(w);
        viewport.Height     = float(h);
        viewport.MinDepth   = 0.0f;
        viewport.MaxDepth   = 1.0f;

        m_pDeviceContext->RSSetViewports( 1, &viewport );

        // シェーダの設定.
        m_pDeviceContext->VSSetShader( m_pFullScreenVS, nullptr, 0 );
        m_pDeviceContext->GSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->HSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->DSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->PSSetShader( m_pLensGhostPS, nullptr, 0 );
 
        m_pDeviceContext->PSSetShaderResources( 0, 1, &pSrc );
        m_pDeviceContext->PSSetShaderResources( 1, 1, &pMask );
        m_pDeviceContext->PSSetSamplers( 0, 1, &m_pLinearClamp );

        // ゴーストを描画.
        for(auto i=0; i<8;++i)
        {
            LensGhostParam param;
            param.MultiplyColor = colors[i];

            // 定数バッファ更新.
            m_pDeviceContext->UpdateSubresource( pCB, 0, nullptr, &param, 0, 0 );

            // 定数バッファ設定.
            m_pDeviceContext->PSSetConstantBuffers( 0, 1, &pCB );

            // 矩形描画.
            m_Quad.Draw(m_pDeviceContext);
        }

        // リソースを解除.
        m_pDeviceContext->PSSetShaderResources( 0, 2, nullSRVs );
    }

    // WorkBuffer[0]を元にゴーストを生成.
    {
        // 乗算カラー / テクスチャスケール.
        asdx::Vector4 colors[] = {
            asdx::Vector4(0.1f, 0.7f, 1.0f, 1.0f),
            asdx::Vector4(1.0f, 0.3f, 0.6f, 2.5f),
            asdx::Vector4(0.3f, 0.8f, 0.8f, 2.3f),
            asdx::Vector4(0.8f, 0.2f, 0.7f, 3.95f),
            asdx::Vector4(0.2f, 0.1f, 0.9f, -1.5f),
            asdx::Vector4(0.9f, 0.1f, 0.2f, -1.7f),
            asdx::Vector4(0.1f, 0.8f, 0.3f, -2.25f),
            asdx::Vector4(0.9f, 0.2f, 0.1f, -3.35f),
        };


        pSrc = m_WorkBuffer[0].GetSRV();
        pDst = m_WorkBuffer[1].GetRTV();

        auto pCB = m_LensGhostBuffer.GetBuffer();
 
        // レンダーターゲット生成.
        m_pDeviceContext->ClearRenderTargetView( pDst, clearColor );
        m_pDeviceContext->OMSetRenderTargets( 1, &pDst, nullptr );

        // ブレンドステート設定.
        m_pDeviceContext->OMSetBlendState( m_pAdditiveBS, blendFactor, sampleMask );

        // シェーダの設定.
        m_pDeviceContext->VSSetShader( m_pFullScreenVS, nullptr, 0 );
        m_pDeviceContext->GSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->HSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->DSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->PSSetShader( m_pLensGhostPS, nullptr, 0 );

        m_pDeviceContext->PSSetShaderResources( 0, 1, &pSrc );
        m_pDeviceContext->PSSetShaderResources( 1, 1, &pMask );
        m_pDeviceContext->PSSetSamplers( 0, 1, &m_pLinearClamp );

        // 8個描画.
        for(auto i=0; i<8; ++i)
        {
            LensGhostParam param;
            param.MultiplyColor = colors[i];

            // 定数バッファ更新.
            m_pDeviceContext->UpdateSubresource( pCB, 0, nullptr, &param, 0, 0 );

            // 定数バッファ設定.
            m_pDeviceContext->PSGetConstantBuffers( 0, 1, &pCB );

           // 矩形描画.
            m_Quad.Draw(m_pDeviceContext);
        }

        // リソースを解除.
        m_pDeviceContext->PSSetShaderResources( 0, 2, nullSRVs );
    }

    // コンポジット.
    {
        // レンダーターゲットビュー・深度ステンシルビューを取得.
        auto pDstRTV = m_RenderTarget2D.GetRTV();
        ID3D11DepthStencilView* pDSV = m_DepthStencilTarget.GetDSV();

        m_pDeviceContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

        // 出力マネージャに設定.
        m_pDeviceContext->OMSetRenderTargets( 1, &pDstRTV, pDSV );

        D3D11_VIEWPORT viewport;
        viewport.TopLeftX   = 0;
        viewport.TopLeftY   = 0;
        viewport.Width      = float(m_Width);
        viewport.Height     = float(m_Height);
        viewport.MinDepth   = 0.0f;
        viewport.MaxDepth   = 1.0f;

        m_pDeviceContext->RSSetViewports(1, &viewport);

        // ステートを設定.
        m_pDeviceContext->RSSetState( m_pRasterizerState );
        m_pDeviceContext->OMSetBlendState( m_pOpequeBS, blendFactor, sampleMask );
        m_pDeviceContext->OMSetDepthStencilState( m_pDepthStencilState, m_StencilRef );

        // シェーダの設定.
        m_pDeviceContext->VSSetShader( m_pFullScreenVS, nullptr, 0 );
        m_pDeviceContext->GSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->HSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->DSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->PSSetShader( m_pCompositePS, nullptr, 0 );

        // シェーダリソースビューを設定.
        ID3D11ShaderResourceView* pSRV[] = {
            m_InputTexture.GetSRV(),
            m_WorkBuffer[1].GetSRV(),
        };
        m_pDeviceContext->PSSetShaderResources( 0, 2, pSRV );
        m_pDeviceContext->PSSetSamplers( 0, 1, &m_pLinearClamp );

        // 描画.
        m_Quad.Draw(m_pDeviceContext);

        // シェーダリソースをクリア.
        ID3D11ShaderResourceView* nullTarget[] = { nullptr, nullptr, nullptr, nullptr, nullptr };
        m_pDeviceContext->PSSetShaderResources( 0, 2, nullTarget );

        // テキストを描画.
        OnDrawText();
    }

    // コマンドを実行して，画面に表示.
    m_pSwapChain->Present( 0, 0 );
}

//---------------------------------------------------------------------------------------
//      リサイズイベントの処理です.
//---------------------------------------------------------------------------------------
void SampleApplication::OnResize( const asdx::ResizeEventParam& param )
{
}

//---------------------------------------------------------------------------------------
//      キーイベントの処理です.
//---------------------------------------------------------------------------------------
void SampleApplication::OnKey( const asdx::KeyEventParam& param )
{
}

//---------------------------------------------------------------------------------------
//      マウスインベント時の処理です.
//---------------------------------------------------------------------------------------
void SampleApplication::OnMouse( const asdx::MouseEventParam& param )
{
}
