﻿//-------------------------------------------------------------------------------------------
// File : asdxAxisRenderer.h
// Desc : Axis Renderer Module.
// Copyright(c) Project Asura. All right reserved.
//--------------------------------------------------------------------------------------------

#ifndef __ASDX_AXIS_RENDERER_H__
#define __ASDX_AXIS_RENDERER_H__

//--------------------------------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------------------------------
#include <asdxMath.h>
#include <d3d11.h>


namespace asdx {

//////////////////////////////////////////////////////////////////////////////////////////////
// AxisRenderer class
//////////////////////////////////////////////////////////////////////////////////////////////
class AxisRenderer
{
    //========================================================================================
    // list of friend classes and methods.
    //========================================================================================
    /* NOTHING */

public:
    //========================================================================================
    // public variables.
    //========================================================================================
    /* NOTHING */

    //========================================================================================
    // public methods.
    //========================================================================================

    //----------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //----------------------------------------------------------------------------------------
    AxisRenderer();

    //----------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //----------------------------------------------------------------------------------------
    virtual ~AxisRenderer();

    //----------------------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //----------------------------------------------------------------------------------------
    bool Init( ID3D11Device* pDevice );

    //----------------------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //----------------------------------------------------------------------------------------
    void Term();

    //----------------------------------------------------------------------------------------
    //! @brief      描画開始処理です.
    //----------------------------------------------------------------------------------------
    void Begin(
        ID3D11DeviceContext* pDeviceContext, 
        const asdx::Matrix& view,
        const asdx::Matrix& proj );

    //----------------------------------------------------------------------------------------
    //! @brief      軸を描画します.
    //----------------------------------------------------------------------------------------
    void Render( 
        ID3D11DeviceContext* pDeviceContext,
        const asdx::Vector3 position,
        const f32 scale = 1.0f );

    //----------------------------------------------------------------------------------------
    //! @brief      描画終了処理です.
    //----------------------------------------------------------------------------------------
    void End( ID3D11DeviceContext* pDeviceContext );

protected:
    //========================================================================================
    // protected variables.
    //========================================================================================
    ID3D11Buffer*               m_pVB;      //!< 頂点バッファです.
    ID3D11Buffer*               m_pCB0;     //!< 定数バッファ(スロット0)です.
    ID3D11Buffer*               m_pCB1;     //!< 定数バッファ(スロット1)です.
    ID3D11VertexShader*         m_pVS;      //!< 頂点シェーダです.
    ID3D11PixelShader*          m_pPS;      //!< ピクセルシェーダです.
    ID3D11InputLayout*          m_pIL;      //!< 入力レイアウトです.
    ID3D11DepthStencilState*    m_pDSS;     //!< 深度ステンシルステートです.

    //========================================================================================
    // protected methods.
    //========================================================================================
    /* NOTHING */

private:
    //========================================================================================
    // private variables.
    //========================================================================================
    /* NOTHING */

    //========================================================================================
    // private methods.
    //========================================================================================
    AxisRenderer    ( const AxisRenderer& renderer );       // アクセス禁止.
    void operator = ( const AxisRenderer& renderer );       // アクセス禁止.
};

} // namespace asdx

#endif//__ASDX_CAMERA_RENDERER_H__
