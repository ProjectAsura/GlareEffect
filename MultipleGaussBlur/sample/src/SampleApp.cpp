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
#include "../res/shader/Compiled/GaussBlurPS.inc"
#include "../res/shader/Compiled/CopyPS.inc"
#include "../res/shader/Compiled/CompositePS.inc"

//-------------------------------------------------------------------------------------------------
//      ガウスの重みを計算します.
//-------------------------------------------------------------------------------------------------
inline float GaussianDistribution( const asdx::Vector2 pos, float rho )
{
    return exp( -( pos.x * pos.x + pos.y * pos.y ) / (2.0f * rho * rho ));
}

//-------------------------------------------------------------------------------------------------
//      ブラーパラメータを計算します.
//-------------------------------------------------------------------------------------------------
inline GaussBlurParam CalcBlurParam( int width, int height, asdx::Vector2 dir, float deviation, float multiply)
{
    GaussBlurParam result;
    result.SampleCount = 15;
    auto tu = 1.0f / float(width);
    auto tv = 1.0f / float(height);

    result.Offset[0].z = GaussianDistribution(asdx::Vector2(0.0f, 0.0f), deviation) * multiply;
    auto total_weight = result.Offset[0].z;

    result.Offset[0].x = 0.0f;
    result.Offset[0].y = 0.0f;

    for(auto i=1; i<8; ++i)
    {
        result.Offset[i].x = dir.x * i * tu;
        result.Offset[i].y = dir.y * i * tv;
        result.Offset[i].z = GaussianDistribution( dir * float(i), deviation ) * multiply;
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
: Application   ( "Multiple Gauss Blur", 1280, 720 )
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
        hr = m_pDevice->CreatePixelShader(CompositePS, sizeof(CompositePS), nullptr, &m_pCopyPS);

        if ( FAILED( hr ) )
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
        if (!m_InputTexture.CreateFromFile( m_pDevice, "../res/texture/input_image.map"))
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
        desc.MaxLOD             = 3.402823466e+38F; // FLT_MAX

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
        desc.MaxLOD             = 3.402823466e+38F; // FLT_MAX

        hr = m_pDevice->CreateSamplerState( &desc, &m_pLinearSampler );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreateSamplerState() Failed." );
            return false;
        }
    }

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

   {
      if ( !m_BlurBuffer.Create(m_pDevice, sizeof(GaussBlurParam)))
      { return false; }
   }

   {
       asdx::RenderTarget2D::Description desc;
       desc.Width               = m_Width  / 4;
       desc.Height              = m_Height / 4;
       desc.MipLevels           = 1;
       desc.ArraySize           = 1;
       desc.Format              = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
       desc.SampleDesc.Count    = 1;
       desc.SampleDesc.Quality  = 0;

       for(auto i=0; i<12; i+=2)
       {
           if (!m_PingPong[i + 0].Create(m_pDevice, desc))
           { return false; }

           if (!m_PingPong[i + 1].Create(m_pDevice, desc))
           { return false; }

           desc.Width  >>= 1;
           desc.Height >>= 1;
       }
   }

    return true;
}

void SampleApplication::OnTerm()
{
    for(auto i=0; i<12; i++)
    { m_PingPong[i].Release(); }
    m_Font.Term();
    m_InputTexture.Release();
    m_BlurBuffer.Release();
    m_Quad.Term();
    ASDX_RELEASE( m_pOpequeBS );
    ASDX_RELEASE( m_pPointSampler );
    ASDX_RELEASE( m_pLinearSampler );
    ASDX_RELEASE( m_pGaussBlurPS );
    ASDX_RELEASE( m_pCopyPS );
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
    auto pSrcSRV = m_InputTexture.GetSRV();
    auto pDstRTV = m_PingPong[0].GetRTV();

    auto w = m_Width  / 4;
    auto h = m_Height / 4;
    auto deviation = 2.5f;

    // 最初のパス.
    {
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

        // シェーダの設定.
        m_pDeviceContext->VSSetShader( m_pFullScreenVS, nullptr, 0 );
        m_pDeviceContext->GSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->HSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->DSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->PSSetShader( m_pGaussBlurPS, nullptr, 0 );

        // シェーダリソースビューを設定.
        auto pSRV = m_InputTexture.GetSRV();
        m_pDeviceContext->PSSetShaderResources( 0, 1, &pSrcSRV );
        m_pDeviceContext->PSSetSamplers( 0, 1, &m_pPointSampler );

        D3D11_VIEWPORT viewport;
        viewport.TopLeftX   = 0;
        viewport.TopLeftY   = 0;
        viewport.Width      = float(w);
        viewport.Height     = float(h);
        viewport.MinDepth   = 0.0f;
        viewport.MaxDepth   = 1.0f;

        m_pDeviceContext->RSSetViewports( 1, &viewport );

        auto pCB = m_BlurBuffer.GetBuffer();
        GaussBlurParam src = CalcBlurParam(w, h, asdx::Vector2(1.0f, 0.0f), deviation, 1.0f);
        m_pDeviceContext->UpdateSubresource( pCB, 0, nullptr, &src, 0, 0 );
        m_pDeviceContext->PSSetConstantBuffers( 0, 1, &pCB );

        // 描画.
        m_Quad.Draw(m_pDeviceContext);

 
        pDstRTV = m_PingPong[1].GetRTV();

        // クリア処理.
        m_pDeviceContext->ClearRenderTargetView( pDstRTV, m_ClearColor );

        // 出力マネージャに設定.
        m_pDeviceContext->OMSetRenderTargets( 1, &pDstRTV, nullptr );

        // シェーダリソースビューを設定.
        pSRV = m_PingPong[0].GetSRV();
        m_pDeviceContext->PSSetShaderResources( 0, 1, &pSRV );
        m_pDeviceContext->PSSetSamplers( 0, 1, &m_pPointSampler );

        src = CalcBlurParam(w, h, asdx::Vector2(0.0f, 1.0f), deviation, 1.0f);

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

        w >>= 1;
        h >>= 1;

        pDstRTV = m_PingPong[2].GetRTV();
        pSrcSRV = m_PingPong[1].GetSRV();
    }

    float m = 2.0f;
    for(auto i=1; i<5; ++i)
    {
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

        // シェーダの設定.
        m_pDeviceContext->VSSetShader( m_pFullScreenVS, nullptr, 0 );
        m_pDeviceContext->GSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->HSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->DSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->PSSetShader( m_pGaussBlurPS, nullptr, 0 );

        // シェーダリソースビューを設定.
        auto pSRV = m_InputTexture.GetSRV();
        m_pDeviceContext->PSSetShaderResources( 0, 1, &pSrcSRV );
        m_pDeviceContext->PSSetSamplers( 0, 1, &m_pPointSampler );

        D3D11_VIEWPORT viewport;
        viewport.TopLeftX   = 0;
        viewport.TopLeftY   = 0;
        viewport.Width      = float(w);
        viewport.Height     = float(h);
        viewport.MinDepth   = 0.0f;
        viewport.MaxDepth   = 1.0f;

        m_pDeviceContext->RSSetViewports( 1, &viewport );

        auto pCB = m_BlurBuffer.GetBuffer();
        GaussBlurParam src = CalcBlurParam(w, h, asdx::Vector2(1.0f, 0.0f), deviation, m);
        m_pDeviceContext->UpdateSubresource( pCB, 0, nullptr, &src, 0, 0 );
        m_pDeviceContext->PSSetConstantBuffers( 0, 1, &pCB );

        // 描画.
        m_Quad.Draw(m_pDeviceContext);

 
        pDstRTV = m_PingPong[i * 2 + 1].GetRTV();

       // クリア処理.
        m_pDeviceContext->ClearRenderTargetView( pDstRTV, m_ClearColor );

        // 出力マネージャに設定.
        m_pDeviceContext->OMSetRenderTargets( 1, &pDstRTV, nullptr );

        // シェーダリソースビューを設定.
        pSRV = m_PingPong[i * 2 + 0].GetSRV();
        m_pDeviceContext->PSSetShaderResources( 0, 1, &pSRV );
        m_pDeviceContext->PSSetSamplers( 0, 1, &m_pPointSampler );

        src = CalcBlurParam(w, h, asdx::Vector2(0.0f, 1.0f), deviation, m);

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

        w >>= 1;
        h >>= 1;
        m *= 2.0f;

        pDstRTV = m_PingPong[i * 2 + 2].GetRTV();
        pSrcSRV = m_PingPong[i * 2 + 1].GetSRV();
    }

    {
        // レンダーターゲットビュー・深度ステンシルビューを取得.
        pDstRTV = m_RenderTarget2D.GetRTV();
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

        // シェーダの設定.
        m_pDeviceContext->VSSetShader( m_pFullScreenVS, nullptr, 0 );
        m_pDeviceContext->GSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->HSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->DSSetShader( nullptr, nullptr, 0 );
        m_pDeviceContext->PSSetShader( m_pCopyPS, nullptr, 0 );

        // シェーダリソースビューを設定.
        ID3D11ShaderResourceView* pSRV[] = {
            m_InputTexture.GetSRV(),
            m_PingPong[1].GetSRV(),
            m_PingPong[3].GetSRV(),
            m_PingPong[5].GetSRV(),
            m_PingPong[7].GetSRV(),
            m_PingPong[9].GetSRV(),
            m_PingPong[11].GetSRV()
        };
        m_pDeviceContext->PSSetShaderResources( 0, 6, pSRV );
        m_pDeviceContext->PSSetSamplers( 0, 1, &m_pLinearSampler );

        // 描画.
        m_Quad.Draw(m_pDeviceContext);

        // シェーダリソースをクリア.
        ID3D11ShaderResourceView* nullTarget[] = { nullptr, nullptr, nullptr, nullptr, nullptr };
        m_pDeviceContext->PSSetShaderResources( 0, 5, nullTarget );

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
