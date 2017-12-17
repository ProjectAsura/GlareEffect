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
#include "../res/shader/Compiled/StarPS.inc"


} // namespace 


/////////////////////////////////////////////////////////////////////////////////////////
// SampleApplication class
/////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
//      コンストラクタです.
//---------------------------------------------------------------------------------------
SampleApplication::SampleApplication()
: Application   ( "Light Streak", 1280, 720 )
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
        hr = m_pDevice->CreatePixelShader(StarPS, sizeof(StarPS), nullptr, &m_pStarPS);
        if ( FAILED(hr) )
        {
            ELOG( "Error : ID3D11CreatePixelShader() Failed." );
            return false;
        }
    }

    {
        if (!m_InputTexture.CreateFromFile( m_pDevice, "../res/texture/star_source.map"))
        {
            ELOG( "Error : Texture::CreateFromFile() Failed. ");
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

        hr = m_pDevice->CreateSamplerState( &desc, &m_pLinearSampler );
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
      if ( !m_BlurBuffer.Create(m_pDevice, sizeof(BlurParam)))
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

       for(auto i=0; i<4; i++)
       {
           if (!m_WorkBuffer[i].Create(m_pDevice, desc))
           { return false; }
       }
   }

    return true;
}

void SampleApplication::OnTerm()
{
    for(auto i=0; i<4; i++)
    { m_WorkBuffer[i].Release(); }
    m_Font.Term();
    m_InputTexture.Release();
    m_BlurBuffer.Release();
    m_Quad.Term();
    ASDX_RELEASE( m_pOpequeBS );
    ASDX_RELEASE( m_pAdditiveBS );
    ASDX_RELEASE( m_pPointSampler );
    ASDX_RELEASE( m_pLinearSampler );
    ASDX_RELEASE( m_pStarPS );
    ASDX_RELEASE( m_pCopyPS );
    ASDX_RELEASE( m_pCompositePS );
    ASDX_RELEASE( m_pFullScreenVS );
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

    // 減衰率.
    auto a = 0.925f; // [0.9f, 0.95f]の範囲.

    // 角度オフセット.
    auto rad_offset = asdx::F_PIDIV4;

    // レンダーターゲットサイズの逆数.
    asdx::Vector2 inv_size(1.0f / w, 1.0f / h);

    float clearColor[4] = { 0, 0, 0, 1 };
    m_pDeviceContext->ClearRenderTargetView( m_WorkBuffer[3].GetRTV(), clearColor );

    // 実験のため4方向固定.
    for(auto j=0; j<4; ++j)
    {
        // サンプリング方向.
        asdx::Vector2 dir(
            cosf(asdx::F_2PI * 0.25f * j + rad_offset),
            sinf(asdx::F_2PI * 0.25f * j + rad_offset)
        );

        auto pSRV = m_InputTexture.GetSRV();

        // 3パスでピンポンブラー.
        for(auto i=0; i<3; ++i)
        {
            auto pDstRTV = m_WorkBuffer[i].GetRTV();

            // クリア処理.
            m_pDeviceContext->ClearRenderTargetView( pDstRTV, m_ClearColor );

            // 出力マネージャに設定.
            m_pDeviceContext->OMSetRenderTargets( 1, &pDstRTV, nullptr );

            float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            UINT sampleMask = D3D11_DEFAULT_SAMPLE_MASK;
            // ステートを設定.
            m_pDeviceContext->RSSetState( m_pRasterizerState );
            m_pDeviceContext->OMSetBlendState( m_pOpequeBS, blendFactor, sampleMask );
            m_pDeviceContext->OMSetDepthStencilState( m_pDepthStencilState, m_StencilRef );

            D3D11_VIEWPORT viewport;
            viewport.TopLeftX   = 0;
            viewport.TopLeftY   = 0;
            viewport.Width      = float(w);
            viewport.Height     = float(h);
            viewport.MinDepth   = 0.0f;
            viewport.MaxDepth   = 1.0f;

            m_pDeviceContext->RSSetViewports( 1, &viewport );

            BlurParam src = {};
            for(auto s=0; s<16; ++s)
            {
                auto b = powf(4.0f, float(i));
                src.Offset[s].x = dir.x * (b * s) * inv_size.x;
                src.Offset[s].y = dir.y * (b * s) * inv_size.y;
                src.Offset[s].z = powf(a, (b * s));
            }

            auto pCB = m_BlurBuffer.GetBuffer();
            m_pDeviceContext->UpdateSubresource( pCB, 0, nullptr, &src, 0, 0 );
            m_pDeviceContext->PSSetConstantBuffers( 0, 1, &pCB );

            // シェーダリソースビューを設定.
            m_pDeviceContext->PSSetShaderResources( 0, 1, &pSRV );
            m_pDeviceContext->PSSetSamplers( 0, 1, &m_pPointSampler );

            // シェーダの設定.
            m_pDeviceContext->VSSetShader( m_pFullScreenVS, nullptr, 0 );
            m_pDeviceContext->GSSetShader( nullptr, nullptr, 0 );
            m_pDeviceContext->HSSetShader( nullptr, nullptr, 0 );
            m_pDeviceContext->DSSetShader( nullptr, nullptr, 0 );
            m_pDeviceContext->PSSetShader( m_pStarPS, nullptr, 0 );

            // 描画.
            m_Quad.Draw(m_pDeviceContext);

            // シェーダリソースをクリア.
            ID3D11ShaderResourceView* nullTarget[1] = { nullptr };
            m_pDeviceContext->PSSetShaderResources( 0, 1, nullTarget );

            // 次のパスの入力を更新.
            pSRV = m_WorkBuffer[i].GetSRV();
        }

        // WorkBuffer[3]に加算合成.
        {
            auto pDstRTV = m_WorkBuffer[3].GetRTV();

            // 出力マネージャに設定.
            m_pDeviceContext->OMSetRenderTargets( 1, &pDstRTV, nullptr );

            float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            UINT sampleMask = D3D11_DEFAULT_SAMPLE_MASK;
            m_pDeviceContext->OMSetBlendState( m_pAdditiveBS, blendFactor, sampleMask );

            // シェーダの設定.
            m_pDeviceContext->VSSetShader( m_pFullScreenVS, nullptr, 0 );
            m_pDeviceContext->GSSetShader( nullptr, nullptr, 0 );
            m_pDeviceContext->HSSetShader( nullptr, nullptr, 0 );
            m_pDeviceContext->DSSetShader( nullptr, nullptr, 0 );
            m_pDeviceContext->PSSetShader( m_pCopyPS, nullptr, 0 );

            pSRV = m_WorkBuffer[2].GetSRV();
            m_pDeviceContext->PSSetShaderResources( 0, 1, &pSRV );
            m_pDeviceContext->PSSetSamplers( 0, 1, &m_pPointSampler );

            // 描画.
            m_Quad.Draw(m_pDeviceContext);

            // シェーダリソースをクリア.
            ID3D11ShaderResourceView* nullTarget[1] = { nullptr };
            m_pDeviceContext->PSSetShaderResources( 0, 1, nullTarget );
        }
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

        float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        UINT sampleMask = D3D11_DEFAULT_SAMPLE_MASK;
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
            m_WorkBuffer[3].GetSRV(),
        };
        m_pDeviceContext->PSSetShaderResources( 0, 2, pSRV );
        m_pDeviceContext->PSSetSamplers( 0, 1, &m_pLinearSampler );

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
