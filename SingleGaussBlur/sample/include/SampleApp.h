﻿//------------------------------------------------------------------------------
// File : SampleApp.h
// Desc : Sample Application.
// Copyright(c) Project Asura. All right reserved.
//------------------------------------------------------------------------------

#ifndef __SAMPLE_APP_H__
#define __SAMPLE_APP_H__

//------------------------------------------------------------------------------
// Include
//------------------------------------------------------------------------------
#include <asdxApp.h>
#include <asdxResMesh.h>
#include <asdxMesh.h>
#include <asdxCameraUpdater.h>
#include <asdxAxisRenderer.h>
#include <asdxQuadRenderer.h>
#include <asdxFont.h>
#include <asdxConstantBuffer.h>
#include <asdxTexture.h>


//////////////////////////////////////////////////////////////////////////////////////////
// GaussBlurParam structure
//////////////////////////////////////////////////////////////////////////////////////////
ASDX_ALIGN(16)
struct GaussBlurParam
{
    int             SampleCount;
    int             Dummy[3];
    asdx::Vector4   Offset[16];    //!< オフセットです.
};


////////////////////////////////////////////////////////////////////////////////////////
// SampleApplication
////////////////////////////////////////////////////////////////////////////////////////
class SampleApplication : public asdx::Application
{
    //==================================================================================
    // list of friend classes and methods.
    //==================================================================================
    /* NOTHING */

private:
    //===================================================================================
    // private variables.
    //===================================================================================
    asdx::Font                  m_Font;                         //!< フォントです.
    asdx::Texture2D             m_InputTexture;                 //!< 入力画像.
    ID3D11VertexShader*         m_pFullScreenVS = nullptr;      //!< フルスクリーン三角形用頂点シェーダ.
    ID3D11PixelShader*          m_pCopyPS       = nullptr;      //!< シングルテクスチャフェッチシェーダ.
    ID3D11PixelShader*          m_pGaussBlurPS  = nullptr;      //!< ガウスブラーシェーダ.
    ID3D11SamplerState*         m_pPointSampler = nullptr;      //!< ポイントサンプラー.
    ID3D11SamplerState*         m_pLinearSampler = nullptr;      //!< ポイントサンプラー.
    ID3D11BlendState*           m_pOpequeBS     = nullptr;
    asdx::QuadRenderer          m_Quad;
    asdx::ConstantBuffer        m_BlurBuffer;
    asdx::RenderTarget2D        m_PingPong[2];

    //==================================================================================
    // private methods.
    //==================================================================================
    void OnDrawText();

protected:
    //==================================================================================
    // protected varaibles
    //==================================================================================
    /* NOTHING */

    //==================================================================================
    // protected methods.
    //==================================================================================

    //----------------------------------------------------------------------------------
    //! @brief      初期化時の処理です.
    //!
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //----------------------------------------------------------------------------------
    bool OnInit();

    //----------------------------------------------------------------------------------
    //! @brief      終了時の処理です.
    //----------------------------------------------------------------------------------
    void OnTerm();

    //----------------------------------------------------------------------------------
    //! @brief      フレーム繊維時の処理です.
    //!
    //! @param [in]     time            アプリケーション開始してからの相対時間です.
    //! @param [in]     elapsedTime     前フレームからの経過時間です.
    //----------------------------------------------------------------------------------
    void OnFrameMove  ( double time, double elapsedTime );

    //----------------------------------------------------------------------------------
    //! @brief      フレーム描画時の処理です.
    //!
    //! @param [in]     time            アプリケーション開始してからの相対時間です.
    //! @param [in]     elapsedTime     前フレームからの経過時間です.
    //----------------------------------------------------------------------------------
    void OnFrameRender( double time, double elapsedTime );

    //----------------------------------------------------------------------------------
    //! @brief      リサイズ時の処理です.
    //!
    //! @param [in]     param           リサイズイベントパラメータです.
    //----------------------------------------------------------------------------------
    void OnResize     ( const asdx::ResizeEventParam& param );

    //----------------------------------------------------------------------------------
    //! @brief      キーイベント時の処理です.
    //!
    //! @param [in]     param           キーイベントパラメータです.
    //----------------------------------------------------------------------------------
    void OnKey        ( const asdx::KeyEventParam& param );

    //----------------------------------------------------------------------------------
    //! @brief      マウスイベント時の処理です.
    //!
    //! @param [in]     param           マウスイベントパラメータです.
    //----------------------------------------------------------------------------------
    void OnMouse      ( const asdx::MouseEventParam& param );

public:
    //==================================================================================
    // public variables.
    //==================================================================================
    /* NOTHING */

    //==================================================================================
    // public methods.
    //==================================================================================

    //----------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //----------------------------------------------------------------------------------
    SampleApplication();

    //----------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //----------------------------------------------------------------------------------
    virtual ~SampleApplication();
};



#endif//__SAMPLE_APP_H__
