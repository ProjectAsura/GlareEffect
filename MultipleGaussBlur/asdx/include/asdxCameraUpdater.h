﻿//-------------------------------------------------------------------------------------------
// File : asdxCameraUpdater.h
// Desc : Camera Updater Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------

#ifndef __ASDX_CAMERA_UPDATER_H__
#define __ASDX_CAMERA_UPDATER_H__

//-------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------
#include <asdxCamera.h>
#include <d3d11.h>


namespace asdx {

/////////////////////////////////////////////////////////////////////////////////////////////
// CameraUpdate class
/////////////////////////////////////////////////////////////////////////////////////////////
class CameraUpdater
{
    //=======================================================================================
    // list of friend classed and methods.
    //=======================================================================================
    /* NOTHING */

private:
    //=======================================================================================
    // private variables.
    //=======================================================================================
    /* NOTHING */

    //=======================================================================================
    // private methods.
    //=======================================================================================
    /* NOTHING */

protected:
    //=======================================================================================
    // protected variables
    //=======================================================================================
    struct MouseState
    {
        s32  X;             //!< 現在のカーソルのX座標です.
        s32  Y;             //!< 現在のカーソルのY座標です.
        s32  prevX;         //!< 以前のカーソルのX座標です.
        s32  prevY;         //!< 以前のカーソルのY座標です.
        bool isClick;       //!< 現在ボタンが押されたかどうか.
        bool isPrevClick;   //!< 以前ボタンが押されたかどうか

        //-----------------------------------------------------------------------------------
        //! @brief      コンストラクタです.
        //-----------------------------------------------------------------------------------
        MouseState()
        : X          ( 0 )
        , Y          ( 0 )
        , prevX      ( 0 )
        , prevY      ( 0 )
        , isClick    ( false )
        , isPrevClick( false )
        { /* DO_NOTHING */ }

        //-----------------------------------------------------------------------------------
        //! @brief      デストラクタです.
        //-----------------------------------------------------------------------------------
        virtual ~MouseState()
        { /* DO_NOTHING */ }

        //-----------------------------------------------------------------------------------
        //! @brief      パラメータをリセットします.
        //-----------------------------------------------------------------------------------
        void Reset()
        {
            X           = 0;
            Y           = 0;
            prevX       = 0;
            prevY       = 0;
            isClick     = false;
            isPrevClick = false;
        }

        //-----------------------------------------------------------------------------------
        //! @brief      パラメータを更新します.
        //!
        //! @param [in]     x       カーソルのX座標.
        //! @param [in]     y       カーソルのY座標.
        //! @param [in]     isDown  ボタンが押されていればtrue.
        //-----------------------------------------------------------------------------------
        void Update( s32 x, s32 y, bool isDown )
        {
            // 以前の状態を更新.
            prevX       = X;
            prevY       = Y;
            isPrevClick = isClick;

            // 現在の状態を更新.
            X           = x;
            Y           = y;
            isClick     = isDown;
        }
    };

    Camera      m_Camera;               //!< カメラです.
    MouseState  m_MouseLeft;            //!< マウスの左ボタンの状態です.
    MouseState  m_MouseRight;           //!< マウスの右ボタンの状態です.
    MouseState  m_MouseMiddle;          //!< マウスの右ボタンの状態です.

    //=======================================================================================
    // protected methods.
    //=======================================================================================

    //---------------------------------------------------------------------------------------
    //! @brief      マウスの状態からカメライベントを生成します.
    //---------------------------------------------------------------------------------------
    CameraEvent MakeEventFromMouse( s32 wheelDelta );

    //---------------------------------------------------------------------------------------
    //! @brief      キーの状態からカメライベントを作成します.
    //---------------------------------------------------------------------------------------
    CameraEvent MakeEventFromKey( u32 nChar, bool isKeyDown, bool isAltDown );

public:
    //=======================================================================================
    // public variables.
    //=======================================================================================
    /* NOTHING */

    //=======================================================================================
    // public methods.
    //=======================================================================================

    //---------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------
    CameraUpdater();

    //---------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------
    virtual ~CameraUpdater();

    //---------------------------------------------------------------------------------------
    //! @brief      初期設定を行います.
    //---------------------------------------------------------------------------------------
    void Init(
        asdx::Vector3 cameraPos,
        asdx::Vector3 cameraAim,
        asdx::Vector3 cameraUp,
        f32        nearClip,
        f32        farClip
    );

    //---------------------------------------------------------------------------------------
    //! @brief      マウスの処理です.
    //---------------------------------------------------------------------------------------
    void OnMouse ( s32 x, s32 y, s32 wheelDelta, bool isLeftButtonDown, bool isRightButtonDown, bool isMiddleButtonDown, bool isSideButton1Down, bool isSideButton2Down );

    //---------------------------------------------------------------------------------------
    //! @brief      キーの処理です.
    //---------------------------------------------------------------------------------------
    void OnKey   ( u32 nChar, bool isKeyDown, bool isAltDown );

    //---------------------------------------------------------------------------------------
    //! @brief      カメラを取得します.
    //---------------------------------------------------------------------------------------
    Camera& GetCamera();

    //---------------------------------------------------------------------------------------
    //! @brief      ビュー行列を取得します.
    //---------------------------------------------------------------------------------------
    asdx::Matrix GetView() const;
};


} // namespace asdx

#endif//__ASDX_CAMERA_UPDATER_H__