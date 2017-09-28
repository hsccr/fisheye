#pragma once

/*
** 게임 그래픽 특론 보조 프로그램 GLFW3 판
**

Copyright (c) 2011-2017 Kohe Tokoi. All Rights Reserved.

Permission is hereby granted, free of charge,  to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction,  including without limitation the rights
to use, copy,  modify, merge,  publish, distribute,  sublicense,  and/or sell
copies or substantial portions of the Software.

The above  copyright notice  and this permission notice  shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO THE WARRANTIES  OF MERCHANTABILITY,
FITNESS  FOR  A PARTICULAR PURPOSE  AND NONINFRINGEMENT.  IN  NO EVENT  SHALL
KOHE TOKOI  BE LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY,  WHETHER IN
AN ACTION  OF CONTRACT,  TORT  OR  OTHERWISE,  ARISING  FROM,  OUT OF  OR  IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**
*/

// 보조 프로그램
#include "gg.h"
using namespace gg;

// Oculus Rift SDK 라이브러리 (LibOVR) 내장
#if defined(USE_OCULUS_RIFT)
#  if defined(_WIN32)
#    define GLFW_EXPOSE_NATIVE_WIN32
#    define GLFW_EXPOSE_NATIVE_WGL
#    include <GLFW/glfw3native.h>
#    undef APIENTRY
#    define OVR_OS_WIN32
#    pragma comment(lib, "LibOVR.lib")
#    define NOTIFY(msg) MessageBox(NULL, TEXT(msg), TEXT("HMD Sample"), MB_ICONERROR | MB_OK)
#  else
#    define NOTIFY(msg) std::cerr << msg << '\n'
#  endif
#  include <OVR_CAPI_GL.h>
#  include <Extras/OVR_Math.h>
#  if OVR_PRODUCT_VERSION > 0
#    include <dxgi.h> // GetDefaultAdapterLuid のため
#    pragma comment(lib, "dxgi.lib")

// 기본 그래픽 어댑터 LUID를 얻을
inline ovrGraphicsLuid GetDefaultAdapterLuid()
{
  ovrGraphicsLuid luid = ovrGraphicsLuid();

#    if defined(_WIN32)
  IDXGIFactory *factory(nullptr);

  if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory))))
  {
    IDXGIAdapter *adapter(nullptr);

    if (SUCCEEDED(factory->EnumAdapters(0, &adapter)))
    {
      DXGI_ADAPTER_DESC desc;

      adapter->GetDesc(&desc);
      memcpy(&luid, &desc.AdapterLuid, sizeof luid);
      adapter->Release();
    }

    factory->Release();
  }
#    endif

  return luid;
}

// LUID 비교하기
inline int Compare(const ovrGraphicsLuid& lhs, const ovrGraphicsLuid& rhs)
{
  return memcmp(&lhs, &rhs, sizeof (ovrGraphicsLuid));
}

#  endif
#endif

// 표준 라이브러리
#include <cstdlib>
#include <iostream>

//
// 창 관련 작업
//
class Window
{
  // 윈도우의 식별자
  GLFWwindow *window;

  // 뷰포트의 폭과 높이
  GLsizei width, height;

  // 뷰포트의 종횡비
  GLfloat aspect;

  // Shift 키
  bool shift_key;

  // 컨트롤 키
  bool control_key;

  // 화살표 키
  int arrow[2];

  // Shift 키를 누른 상태에서 화살표 키
  int shift_arrow[2];

  // 컨트롤 키를 누른 상태에서 화살표 키
  int control_arrow[2];

  // 마우스의 현재 위치
  double mouse_x, mouse_y;

  // 마우스 휠의 회 전량
  double wheel_rotation;

  // 시프트를 누른 상태에서 마우스 휠의 회 전량
  double shift_wheel_rotation;

  // 컨트롤을 누른 상태에서 마우스 휠의 회 전량
  double control_wheel_rotation;

  // 드래그에 의한 트랙볼
  GgTrackball trackball_left;

  // 오른쪽 드래그하여 트랙볼
  GgTrackball trackball_right;

#if defined(USE_OCULUS_RIFT)
  //
  // Oculus Rift
  //

  // Oculus Rift 세션
  ovrSession session;

  // Oculus Rift 상태
  ovrHmdDesc hmdDesc;

  // Oculus Rift 화면 크기
  GLfloat screen[ovrEye_Count][4];

  // Oculus Rift 표시 용 FBO
  GLuint oculusFbo[ovrEye_Count];

  // 거울보기위한 FBO
  GLuint mirrorFbo;

#  if OVR_PRODUCT_VERSION > 0
  // Oculus Rift에 쓰기 그리기 데이터
  ovrLayerEyeFov layerData;

  // Oculus Rift 렌더링 프레임 번호
  long long frameIndex;

  // Oculus Rift 표시 용 FBO의 깊이 텍스처
  GLuint oculusDepth[ovrEye_Count];

  // 거울보기위한 FBO의 크기
  int mirrorWidth, mirrorHeight;

  // 거울보기위한 FBO의 색상 질감
  ovrMirrorTexture mirrorTexture;
#  else
  // Oculus Rift에 쓰기 그리기 데이터
  ovrLayer_Union layerData;

  // Oculus Rift 렌더링 정보
  ovrEyeRenderDesc eyeRenderDesc[ovrEye_Count];

  // Oculus Rift 관점 정보
  ovrPosef eyePose[ovrEye_Count];

  // 거울보기위한 FBO의 색상 질감
  ovrGLTexture *mirrorTexture;
#  endif
#endif

  // 복사 생성자를 봉쇄
  Window(const Window &w);

  // 대입을 봉쇄
  Window &operator=(const Window &w);

public:

  //
  // 생성자
  //
  Window(const char *title = "GLFW Window", int width = 1920, int height = 1080,
    int fullscreen = 0, GLFWwindow *share = nullptr)
    : window(nullptr)
  {
    // 초기화 된 경우 true
    static bool initialized(false);

    // GLFW가 초기화되어 있지 않으면
    if (!initialized)
    {
      // GLFW 초기화
      if (glfwInit() == GL_FALSE) return;

      // 프로그램 종료시 처리 등록
      atexit(glfwTerminate);

#if defined(USE_OCULUS_RIFT)
      // Oculus Rift (LibOVR) 를 초기화
      if (OVR_FAILURE(ovr_Initialize(nullptr)))
      {
        NOTIFY("LibOVR can not be initialized");
        return;
      }

      // 프로그램 종료시에는 LibOVR 종료
      atexit(ovr_Shutdown);

      // LUID는 OpenGL에서 사용하지 않는 것
      ovrGraphicsLuid luid;

      // Oculus Rift 세션을 만들
      session = nullptr;
      if (OVR_FAILURE(ovr_Create(&session, &luid)))
      {
        NOTIFY("Oculus Rift is not connected");
        return;
      }

      // Oculus Rift에 렌더링에 사용 FBO의 초기 값을 설정
      for (int eye = 0; eye < ovrEye_Count; ++eye) oculusFbo[eye] = 0;

      // 거울 표시에 사용 FBO의 초기 값을 설정
      mirrorFbo = 0;
      mirrorTexture = nullptr;

#  if OVR_PRODUCT_VERSION > 0
      // 기본 그래픽 어댑터를 사용하고 있는지 확인
      if (Compare(luid, GetDefaultAdapterLuid())) return;

      // Asynchronous TimeWarp 처리에 사용하는 프레임 번호의 초기 값을 설정
      frameIndex = 0LL;

      // Oculus Rift에 렌더링에 사용 FBO의 깊이 텍스처의 초기 값을 설정
      for (int eye = 0; eye < ovrEye_Count; ++eye) oculusDepth[eye] = 0;
#  endif

      // Oculus Rift는 더블 버퍼링하지
      glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);

      // Oculus Rift로 렌더링
      glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
#endif

      // OpenGL Version 3.2 Core Profile 을 선택
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

      // 화살표 키와 마우스 조작의 초기 값을 설정
      arrow[0] = arrow[1] = 0;
      shift_arrow[0] = shift_arrow[1] = 0;
      control_arrow[0] = control_arrow[1] = 0;
      wheel_rotation = shift_wheel_rotation = control_wheel_rotation = 0.0;

      // 초기화 된 표시한다
      initialized = true;
    }

    // 디스플레이 정보
    GLFWmonitor *monitor(nullptr);

    // 전체 화면
    if (fullscreen > 0)
    {
      // 연결되어있는 모니터의 수를 계산
      int mcount;
      GLFWmonitor **const monitors = glfwGetMonitors(&mcount);

      // 보조 모니터가 있으면 그것을 사용
      if (fullscreen > mcount) fullscreen = mcount;
      monitor = monitors[fullscreen - 1];

      // 모니터 모드를 조사
      const GLFWvidmode *mode(glfwGetVideoMode(monitor));

      // 윈도우의 크기를 디스플레이의 크기가
      width = mode->width;
      height = mode->height;
    }

    // GLFW 창을 만들
    window = glfwCreateWindow(width, height, title, monitor, share);

    // 창을 만들 수 있어야 돌아 가기
    if (!window) return;

    // 현재 윈도우를 처리 대상으로하는
    glfwMakeContextCurrent(window);

    // 게임 그래픽 특론의 사정에 의한 초기화를 할
    ggInit();

    // 이 인스턴스의 this 포인터를 기록해 두는
    glfwSetWindowUserPointer(window, this);

    // 키보드를 조작했을 때의 처리를 등록하기
    glfwSetKeyCallback(window, keyboard);

    // 마우스 버튼을 조작했을 때의 처리를 등록하기
    glfwSetMouseButtonCallback(window, mouse);

    // 마우스 휠 조작시 호출 처리를 등록하기
    glfwSetScrollCallback(window, wheel);

    // 창 크기 변경시에 호출 처리를 등록하기
    glfwSetFramebufferSizeCallback(window, resize);

#if defined(USE_OCULUS_RIFT)
    // Oculus Rift 정보를 추출
    hmdDesc = ovr_GetHmdDesc(session);

#  if defined(_DEBUG)
    // Oculus Rift 정보보기
    std::cerr
      << "\nProduct name: " << hmdDesc.ProductName
      << "\nResolution:   " << hmdDesc.Resolution.w << " x " << hmdDesc.Resolution.h
      << "\nDefault Fov:  (" << hmdDesc.DefaultEyeFov[ovrEye_Left].LeftTan
      << "," << hmdDesc.DefaultEyeFov[ovrEye_Left].DownTan
      << ") - (" << hmdDesc.DefaultEyeFov[ovrEye_Left].RightTan
      << "," << hmdDesc.DefaultEyeFov[ovrEye_Left].UpTan
      << ")\n              (" << hmdDesc.DefaultEyeFov[ovrEye_Right].LeftTan
      << "," << hmdDesc.DefaultEyeFov[ovrEye_Right].DownTan
      << ") - (" << hmdDesc.DefaultEyeFov[ovrEye_Right].RightTan
      << "," << hmdDesc.DefaultEyeFov[ovrEye_Right].UpTan
      << ")\nMaximum Fov:  (" << hmdDesc.MaxEyeFov[ovrEye_Left].LeftTan
      << "," << hmdDesc.MaxEyeFov[ovrEye_Left].DownTan
      << ") - (" << hmdDesc.MaxEyeFov[ovrEye_Left].RightTan
      << "," << hmdDesc.MaxEyeFov[ovrEye_Left].UpTan
      << ")\n              (" << hmdDesc.MaxEyeFov[ovrEye_Right].LeftTan
      << "," << hmdDesc.MaxEyeFov[ovrEye_Right].DownTan
      << ") - (" << hmdDesc.MaxEyeFov[ovrEye_Right].RightTan
      << "," << hmdDesc.MaxEyeFov[ovrEye_Right].UpTan
      << ")\n" << std::endl;
#  endif

    // Oculus Rift에 전송하는 렌더링 데이터를 생성하는
#  if OVR_PRODUCT_VERSION > 0
    layerData.Header.Type = ovrLayerType_EyeFov;
#  else
    layerData.Header.Type = ovrLayerType_EyeFovDepth;
#  endif
    layerData.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // OpenGL 이므로 왼쪽이 원점

    // Oculus Rift 표시 용의 FBO를 만들
    for (int eye = 0; eye < ovrEye_Count; ++eye)
    {
      // Oculus Rift 시각을 얻을
      const auto &fov(hmdDesc.DefaultEyeFov[ovrEyeType(eye)]);

      // Oculus Rift 표시 용 FBO의 크기를 요구
      const auto textureSize(ovr_GetFovTextureSize(session, ovrEyeType(eye), fov, 1.0f));

      // Oculus Rift 표시 용 FBO의 화면 비율을 요구
      aspect = static_cast<GLfloat>(textureSize.w) / static_cast<GLfloat>(textureSize.h);

      // Oculus Rift의 화면 크기를 저장
      screen[eye][0] = -fov.LeftTan;
      screen[eye][1] = fov.RightTan;
      screen[eye][2] = -fov.DownTan;
      screen[eye][3] = fov.UpTan;

#  if OVR_PRODUCT_VERSION > 0
      // 그리기 데이터에 시각을 설정
      layerData.Fov[eye] = fov;

      // 그리기 데이터에 뷰포트를 설정
      layerData.Viewport[eye].Pos = OVR::Vector2i(0, 0);
      layerData.Viewport[eye].Size = textureSize;

      // Oculus Rift 표시 용 FBO의 색상 버퍼로 사용할 텍스처 세트의 특성
      const ovrTextureSwapChainDesc colorDesc =
      {
        ovrTexture_2D,                    // Type
        OVR_FORMAT_R8G8B8A8_UNORM_SRGB,   // Format
        1,                                // ArraySize
        textureSize.w,                    // Width
        textureSize.h,                    // Height
        1,                                // MipLevels
        1,                                // SampleCount
        ovrFalse,                         // StaticImage
        0, 0
      };

      // Oculus Rift 표시 용 FBO 렌더 타겟으로 사용할 텍스처 체인을 만들
      layerData.ColorTexture[eye] = nullptr;
      if (OVR_SUCCESS(ovr_CreateTextureSwapChainGL(session, &colorDesc, &layerData.ColorTexture[eye])))
      {
        // 생성 한 텍스처 체인의 길이를 얻을
        int length(0);
        if (OVR_SUCCESS(ovr_GetTextureSwapChainLength(session, layerData.ColorTexture[eye], &length)))
        {
          // 텍스처 체인의 각 요소에 대해
          for (int i = 0; i < length; ++i)
          {
            // 텍스처의 매개 변수를 설정
            GLuint texId;
            ovr_GetTextureSwapChainBufferGL(session, layerData.ColorTexture[eye], i, &texId);
            glBindTexture(GL_TEXTURE_2D, texId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
          }
        }

        // Oculus Rift표시 용 FBO의 깊이 버퍼로 사용할 텍스처를 만들
        glGenTextures(1, &oculusDepth[eye]);
        glBindTexture(GL_TEXTURE_2D, oculusDepth[eye]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, textureSize.w, textureSize.h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
#  else
      // 그리기 데이터에 시각을 설정
      layerData.EyeFov.Fov[eye] = fov;

      // 그리기 데이터에 뷰포트를 설정
      layerData.EyeFov.Viewport[eye].Pos = OVR::Vector2i(0, 0);
      layerData.EyeFov.Viewport[eye].Size = textureSize;

      // Oculus Rift 표시 용 FBO의 색상 버퍼로 사용할 텍스처 세트를 만들
      ovrSwapTextureSet *colorTexture;
      ovr_CreateSwapTextureSetGL(session, GL_SRGB8_ALPHA8, textureSize.w, textureSize.h, &colorTexture);
      layerData.EyeFov.ColorTexture[eye] = colorTexture;

      // Oculus Rift 표시 용 FBO의 깊이 버퍼로 사용할 텍스처 세트를 만들
      ovrSwapTextureSet *depthTexture;
      ovr_CreateSwapTextureSetGL(session, GL_DEPTH_COMPONENT32F, textureSize.w, textureSize.h, &depthTexture);
      layerData.EyeFovDepth.DepthTexture[eye] = depthTexture;

      // Oculus Rift 렌즈 보정 등의 설정 값을 얻을
      eyeRenderDesc[eye] = ovr_GetRenderDesc(session, ovrEyeType(eye), fov);
#  endif
    }

#  if OVR_PRODUCT_VERSION > 0
    // 자세 추적의 원점을 눈 위치로 설정
    // (원점 바닥의 높이로 설정하면 ovrTrackingOrigin_FloorLevel 지정)
    ovr_SetTrackingOriginType(session, ovrTrackingOrigin_EyeLevel);

    // HMD의 현재 위치를 기준으로하는
    // (ovrTrackingOrigin_FloorLevel의 경우 높이가 재설정되지 않음)
    ovr_RecenterTrackingOrigin(session);

    // 거울보기위한 FBO를 만들
    const ovrMirrorTextureDesc mirrorDesc =
    {
      OVR_FORMAT_R8G8B8A8_UNORM_SRGB,   // Format
      mirrorWidth = width,              // Width
      mirrorHeight = height,            // Height
      0                                 // Flags
    };

    // 거울보기위한 FBO의 색상 버퍼로 사용할 텍스처를 만들
    if (OVR_SUCCESS(ovr_CreateMirrorTextureGL(session, &mirrorDesc, &mirrorTexture)))
    {
      // 생성 한 텍스처의 텍스처 이름을 얻는다
      GLuint texId;
      if (OVR_SUCCESS(ovr_GetMirrorTextureBufferGL(session, mirrorTexture, &texId)))
      {
        // 생성 된 텍스처를 거울보기위한 FBO 색상 버퍼로 통합
        glGenFramebuffers(1, &mirrorFbo);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFbo);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
        glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
      }
    }
#  else
    // 거울보기위한 FBO를 만들
    if (OVR_SUCCESS(ovr_CreateMirrorTextureGL(session, GL_SRGB8_ALPHA8, width, height, reinterpret_cast<ovrTexture **>(&mirrorTexture))))
    {
      glGenFramebuffers(1, &mirrorFbo);
      glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFbo);
      glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTexture->OGL.TexId, 0);
      glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
      glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }
#  endif

    // Oculus Rift 렌더링 용의 FBO를 만들
    glGenFramebuffers(ovrEye_Count, oculusFbo);

    // Oculus Rift 렌더링 할 때 sRGB 색 공간을 사용
    glEnable(GL_FRAMEBUFFER_SRGB);

    // Oculus Rift에 표시는 스왑 간격을 기다리지 않는다
    glfwSwapInterval(0);
#else
    // 스왑 간격을 기다리는
    glfwSwapInterval(1);
#endif

    // 뷰포트와 투영 변환 행렬을 초기화하는
    resize(window, width, height);
  }

  //
  // 소멸자
  //
  virtual ~Window()
  {
    // 윈도우가 생성되어 있지 않으면 돌아 가기
    if (!window) return;

#if defined(USE_OCULUS_RIFT)
    // 거울보기위한 FBO를 제거
    if (mirrorFbo) glDeleteFramebuffers(1, &mirrorFbo);

    // 거울보기에 사용한 텍스처를 개방
    if (mirrorTexture)
    {
#  if OVR_PRODUCT_VERSION > 0
      ovr_DestroyMirrorTexture(session, mirrorTexture);
#  else
      glDeleteTextures(1, &mirrorTexture->OGL.TexId);
      ovr_DestroyMirrorTexture(session, reinterpret_cast<ovrTexture *>(mirrorTexture));
#  endif
    }

    // Oculus Rift 렌더링 용의 FBO를 제거
    glDeleteFramebuffers(ovrEye_Count, oculusFbo);

    // Oculus Rift 표시 용의 FBO를 제거
    for (int eye = 0; eye < ovrEye_Count; ++eye)
    {
#  if OVR_PRODUCT_VERSION > 0
      // 렌더링 타겟으로 사용한 텍스처를 개방
      if (layerData.ColorTexture[eye])
      {
        ovr_DestroyTextureSwapChain(session, layerData.ColorTexture[eye]);
        layerData.ColorTexture[eye] = nullptr;
      }

      // 깊이 버퍼로 사용한 텍스처를 개방
      glDeleteTextures(1, &oculusDepth[eye]);
      oculusDepth[eye] = 0;
#  else
      // 렌더링 타겟으로 사용한 텍스처를 개방
      auto *const colorTexture(layerData.EyeFov.ColorTexture[eye]);
      for (int i = 0; i < colorTexture->TextureCount; ++i)
      {
        const auto *const ctex(reinterpret_cast<ovrGLTexture *>(&colorTexture->Textures[i]));
        glDeleteTextures(1, &ctex->OGL.TexId);
      }
      ovr_DestroySwapTextureSet(session, colorTexture);

      // 깊이 버퍼로 사용한 텍스처를 개방
      auto *const depthTexture(layerData.EyeFovDepth.DepthTexture[eye]);
      for (int i = 0; i < depthTexture->TextureCount; ++i)
      {
        const auto *const dtex(reinterpret_cast<ovrGLTexture *>(&depthTexture->Textures[i]));
        glDeleteTextures(1, &dtex->OGL.TexId);
      }
      ovr_DestroySwapTextureSet(session, depthTexture);
#  endif
    }

    // Oculus Rift 세션을 삭제
    ovr_Destroy(session);
    session = nullptr;
#endif

    // 윈도우를 파기
    glfwDestroyWindow(window);
  }

#if defined(USE_OCULUS_RIFT)
  //
  // Oculus Rift 의한 그리기 시작
  //
  bool start()
  {
#  if OVR_PRODUCT_VERSION > 0
    // 세션의 상태를 얻을
    ovrSessionStatus sessionStatus;
    ovr_GetSessionStatus(session, &sessionStatus);

    // 응용 프로그램이 종료를 요구하고있을 때는 창 닫기 플래그를
    if (sessionStatus.ShouldQuit) glfwSetWindowShouldClose(window, GL_TRUE);

    // Oculus Rift에 표시되지 않을 때는 돌아 가기
    if (!sessionStatus.IsVisible) return false;

    // 현재 상태를 추적 원점으로하는
    if (sessionStatus.ShouldRecenter) ovr_RecenterTrackingOrigin(session);

    // Oculus Remote의 Back 버튼을 누르면 시점의 위치를 ​​재설정
    ovrInputState inputState;
    if (OVR_SUCCESS(ovr_GetInputState(session, ovrControllerType_Remote, &inputState)))
    {
      if (inputState.Buttons & ovrButton_Back) ovr_RecenterTrackingOrigin(session);
    }

    // HmdToEyeOffset 등은 실행시에 변화하기 때문에 매 프레임 ovr_GetRenderDesc ()에서 ovrEyeRenderDesc를 얻을
    const ovrEyeRenderDesc eyeRenderDesc[] =
    {
      ovr_GetRenderDesc(session, ovrEyeType(0), hmdDesc.DefaultEyeFov[0]),
      ovr_GetRenderDesc(session, ovrEyeType(1), hmdDesc.DefaultEyeFov[1])
    };

    // Oculus Rift 스크린의 헤드 트래킹 위치에서의 변위를 얻을
    const ovrVector3f hmdToEyeOffset[] =
    {
      eyeRenderDesc[0].HmdToEyeOffset,
      eyeRenderDesc[1].HmdToEyeOffset
    };

    // 관점의 자세 정보를 얻을
    ovr_GetEyePoses(session, ++frameIndex, ovrTrue, hmdToEyeOffset, layerData.RenderPose, &layerData.SensorSampleTime);
#  else
    // 프레임의 타이밍 측정 시작
    const auto ftiming(ovr_GetPredictedDisplayTime(session, 0));

    // sensorSampleTime의 취득은 가능한 ovr_GetTrackingState () 근처에서 할
    layerData.EyeFov.SensorSampleTime = ovr_GetTimeInSeconds();

    // 헤드 트래킹 상태를 얻을
    const auto hmdState(ovr_GetTrackingState(session, ftiming, ovrTrue));

    // Oculus Rift 스크린의 헤드 트래킹 위치에서의 변위를 얻을
    const ovrVector3f hmdToEyeViewOffset[] =
    {
      eyeRenderDesc[0].HmdToEyeViewOffset,
      eyeRenderDesc[1].HmdToEyeViewOffset
    };

    // 관점의 자세 정보 요청
    ovr_CalcEyePoses(hmdState.HeadPose.ThePose, hmdToEyeViewOffset, eyePose);
#  endif

    return true;
  }

  //
  // Oculus Rift의 그리기 눈 지정
  //
  void select(int eye, GLfloat *screen, GLfloat *position, GLfloat *orientation)
  {
#  if OVR_PRODUCT_VERSION > 0
    // Oculus Rift 렌더링 FBO로 전환
    if (layerData.ColorTexture[eye])
    {
      // FBO의 색상 버퍼로 사용하는 현재의 텍스처 인덱스를 얻을
      int curIndex;
      ovr_GetTextureSwapChainCurrentIndex(session, layerData.ColorTexture[eye], &curIndex);

      // FBO의 색상 버퍼로 사용할 텍스처를 얻을
      GLuint curTexId;
      ovr_GetTextureSwapChainBufferGL(session, layerData.ColorTexture[eye], curIndex, &curTexId);

      // FBO를 설정
      glBindFramebuffer(GL_FRAMEBUFFER, oculusFbo[eye]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, oculusDepth[eye], 0);

      // 뷰포트를 설정
      const auto &vp(layerData.Viewport[eye]);
      glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
    }

    // Oculus Rift의 한쪽의 정도
    const auto &p(layerData.RenderPose[eye].Position);
    const auto &o(layerData.RenderPose[eye].Orientation);
#  else
    // 렌더 타겟에 그리기 전에 렌더 타겟의 인덱스를 증가
    auto *const colorTexture(layerData.EyeFov.ColorTexture[eye]);
    colorTexture->CurrentIndex = (colorTexture->CurrentIndex + 1) % colorTexture->TextureCount;
    auto *const depthTexture(layerData.EyeFovDepth.DepthTexture[eye]);
    depthTexture->CurrentIndex = (depthTexture->CurrentIndex + 1) % depthTexture->TextureCount;

    // 렌더 타겟을 전환
    glBindFramebuffer(GL_FRAMEBUFFER, oculusFbo[eye]);
    const auto &ctex(reinterpret_cast<ovrGLTexture *>(&colorTexture->Textures[colorTexture->CurrentIndex]));
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctex->OGL.TexId, 0);
    const auto &dtex(reinterpret_cast<ovrGLTexture *>(&depthTexture->Textures[depthTexture->CurrentIndex]));
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dtex->OGL.TexId, 0);

    // 뷰포트를 설정
    const auto &vp(layerData.EyeFov.Viewport[eye]);
    glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);

    // Oculus Rift 한눈의 위치와 회전을 얻을
    const auto &p(eyePose[eye].Position);
    const auto &o(eyePose[eye].Orientation);
#  endif

    // Oculus Rift 스크린의 크기를 반환
    screen[0] = this->screen[eye][0];
    screen[1] = this->screen[eye][1];
    screen[2] = this->screen[eye][2];
    screen[3] = this->screen[eye][3];

    // Oculus Rift 위치를 변경하여 반환
    position[0] = -p.x;
    position[1] = -p.y;
    position[2] = -p.z;
    position[3] = 1.0f;

    // Oculus Rift 방향을 반전하여 반환
    orientation[0] = -o.x;
    orientation[1] = -o.y;
    orientation[2] = -o.z;
    orientation[3] = o.w;
  }

  //
  // 도형 그리기를 완료
  //
  void Window::commit(int eye)
  {
#if OVR_PRODUCT_VERSION > 0
    // GL_COLOR_ATTACHMENT0에 할당 된 텍스처가 wglDXUnlockObjectsNV ()에 의해
    // 잠금 해제되기 위해서는 다음 프레임의 처리에서 잘못된 GL_COLOR_ATTACHMENT0가
    // FBO에 결합되는 것을 피하기
    glBindFramebuffer(GL_FRAMEBUFFER, oculusFbo[eye]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

    // 보류중인 변경 내용을 layerData.ColorTexture [eye]에 반영 인덱스를 업데이트하는
    ovr_CommitTextureSwapChain(session, layerData.ColorTexture[eye]);
#endif
  }

  //
  // Time Warp 처리에 사용하는 투영 변환 행렬의 성분의 설정
  //
  void timewarp(const GgMatrix &projection)
  {
    // TimeWarp 사용 변환 행렬의 성분을 설정
#  if OVR_PRODUCT_VERSION < 1
    auto &posTimewarpProjectionDesc(layerData.EyeFovDepth.ProjectionDesc);
    posTimewarpProjectionDesc.Projection22 = (projection.get()[4 * 2 + 2] + projection.get()[4 * 3 + 2]) * 0.5f;
    posTimewarpProjectionDesc.Projection23 = projection.get()[4 * 2 + 3] * 0.5f;
    posTimewarpProjectionDesc.Projection32 = projection.get()[4 * 3 + 2];
#  endif
  }
#endif

  //
  // 윈도우의 식별자의 취득
  //
  GLFWwindow *get() const
  {
    return window;
  }

  //
  // 창 닫기인지 여부를 판단
  //
  bool shouldClose()
  {
    // 창을 닫거나 ESC 키 타자가 있으면 true를 반환
    return glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE);
  }

  //
  // 뷰포트를 바탕으로 되 돌린다
  //
  void restoreViewport()
  {
    glViewport(0, 0, width, height);
  }

  //
  // 컬러 버퍼를 교체하여 이벤트를 검색
  //
  void swapBuffers()
  {
    // 오류 검사
    ggError(__FILE__, __LINE__);

#if defined(USE_OCULUS_RIFT)
#  if OVR_PRODUCT_VERSION > 0
    // 그리기 데이터를 Oculus Rift로 전송
    const auto *const layers(&layerData.Header);
    if (OVR_FAILURE(ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1)))
#  else
    // Oculus Rift 위 드로잉 위치와 확대를 추구
    ovrViewScaleDesc viewScaleDesc;
    viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
    viewScaleDesc.HmdToEyeViewOffset[0] = eyeRenderDesc[0].HmdToEyeViewOffset;
    viewScaleDesc.HmdToEyeViewOffset[1] = eyeRenderDesc[1].HmdToEyeViewOffset;

    // 그리기 데이터를 업데이트하는
    layerData.EyeFov.RenderPose[0] = eyePose[0];
    layerData.EyeFov.RenderPose[1] = eyePose[1];

    // 그리기 데이터를 Oculus Rift로 전송
    const auto *const layers(&layerData.Header);
    if (OVR_FAILURE(ovr_SubmitFrame(session, 0, &viewScaleDesc, &layers, 1)))
#  endif
    {
      // 전송에 실패하면 Oculus Rift 설정을 처음부터 다시 시작해야 할 것
      // 하지만 귀찮은 때문에 창을 닫고 만다
      NOTIFY("Oculus Rift has been disconnected");
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // 렌더링 결과를 거울보기위한 프레임 버퍼에 전송
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
#  if OVR_PRODUCT_VERSION > 0
    const auto w(mirrorWidth), h(mirrorHeight);
#  else
    const auto w(mirrorTexture->OGL.Header.TextureSize.w);
    const auto h(mirrorTexture->OGL.Header.TextureSize.h);
#  endif
    glBlitFramebuffer(0, h, w, 0, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // 남아있는 OpenGL 명령을 실행
    glFlush();
#else
    // 컬러 버퍼를 교체
    glfwSwapBuffers(window);
#endif

    // Shift 키와 컨트롤 키의 상태를 재설정
    shift_key = control_key = false;

    // 이벤트를 검색
    glfwPollEvents();

    // 마우스의 위치를 결정
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    const GLfloat x(static_cast<GLfloat>(mouse_x));
    const GLfloat y(static_cast<GLfloat>(mouse_y));

    // 왼쪽 버튼 드래그
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) trackball_left.motion(x, y);

    // 오른쪽 버튼 드래그
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) trackball_right.motion(x, y);
  }

  //
  // 창 크기 변경시의 처리
  //
  static void resize(GLFWwindow *window, int width, int height)
  {
    // 이 인스턴스의 this 포인터를 얻을
    Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

    if (instance)
    {
      // 창의 크기를 저장
      instance->width = width;
      instance->height = height;

      // 트랙볼 처리의 범위를 설정하는
      instance->trackball_left.region(width, height);
      instance->trackball_right.region(width, height);

#if !defined(USE_OCULUS_RIFT)
      // 윈도우의 화면 비율을 저장
      instance->aspect = static_cast<GLfloat>(width) / static_cast<GLfloat>(height);

      // 전체 창 그릴
      instance->restoreViewport();
#endif
    }
  }

  //
  // 키보드를 입력했을 때의 처리
  //
  static void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods)
  {
    // 이 인스턴스의 this 포인터를 얻을
    Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

    if (instance)
    {
      if (action == GLFW_PRESS)
      {
        switch (key)
        {
        case GLFW_KEY_R:
          // 마우스 휠의 회 전량을 재설정
          instance->wheel_rotation = 0.0;
          instance->shift_wheel_rotation = 0.0;
          instance->control_wheel_rotation = 0.0;

          // 화살표 키 설정을 재설정
          instance->arrow[0] = instance->arrow[1] = 0;
          instance->shift_arrow[0] = instance->shift_arrow[1] = 0;
          instance->control_arrow[0] = instance->control_arrow[1] = 0;

        case GLFW_KEY_O:
          // 트랙볼을 재설정
          instance->trackball_left.reset();
          instance->trackball_right.reset();
          break;

        case GLFW_KEY_SPACE:
          break;

        case GLFW_KEY_BACKSPACE:
        case GLFW_KEY_DELETE:
          break;

        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_SHIFT:
          instance->shift_key = true;
          break;

        case GLFW_KEY_LEFT_CONTROL:
        case GLFW_KEY_RIGHT_CONTROL:
          instance->control_key = true;
          break;

        case GLFW_KEY_UP:
          if (instance->shift_key)
            instance->shift_arrow[1]++;
          else if (instance->control_key)
            instance->control_arrow[1]++;
          else
            instance->arrow[1]++;
          break;

        case GLFW_KEY_DOWN:
          if (instance->shift_key)
            instance->shift_arrow[1]--;
          else if (instance->control_key)
            instance->control_arrow[1]--;
          else
            instance->arrow[1]--;
          break;

        case GLFW_KEY_RIGHT:
          if (instance->shift_key)
            instance->shift_arrow[0]++;
          else if (instance->control_key)
            instance->control_arrow[0]++;
          else
            instance->arrow[0]++;
          break;

        case GLFW_KEY_LEFT:
          if (instance->shift_key)
            instance->shift_arrow[0]--;
          else if (instance->control_key)
            instance->control_arrow[0]--;
          else
            instance->arrow[0]--;
          break;

        default:
          break;
        }
      }
    }
  }

  //
  // 마우스 버튼을 조작했을 때의 처리
  //
  static void mouse(GLFWwindow *window, int button, int action, int mods)
  {
    // 이 인스턴스의 this 포인터를 얻을
    Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

    if (instance)
    {
      // 마우스의 현재 위치를 얻을
      const GLfloat x(static_cast<GLfloat>(instance->mouse_x));
      const GLfloat y(static_cast<GLfloat>(instance->mouse_y));

      switch (button)
      {
      case GLFW_MOUSE_BUTTON_1:
        if (action)
        {
          // 드래그 시작
          instance->trackball_left.start(x, y);
        }
        else
        {
          // 드래그 종료
          instance->trackball_left.stop(x, y);
        }
        break;

      case GLFW_MOUSE_BUTTON_2:
        if (action)
        {
          // 오른쪽 드래그 시작
          instance->trackball_right.start(x, y);
        }
        else
        {
          // 오른쪽 드래그 종료
          instance->trackball_right.stop(x, y);
        }
        break;

      case GLFW_MOUSE_BUTTON_3:
        break;

      default:
        break;
      }
    }
  }

  //
  // 마우스 휠을 조작했을 때의 처리
  //
  static void wheel(GLFWwindow *window, double x, double y)
  {
    // 이 인스턴스의 this 포인터를 얻을
    Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));

    if (instance)
    {
      if (instance->shift_key)
        instance->shift_wheel_rotation += y;
      else if (instance->control_key)
        instance->control_wheel_rotation += y;
      else
      {
        instance->wheel_rotation += y;
        if (instance->wheel_rotation < -100.0)
          instance->wheel_rotation = -100.0;
        else if (instance->wheel_rotation > 49.0)
          instance->wheel_rotation = 49.0;
      }
    }
  }

  //
  // 창의 너비를 얻을
  //
  GLsizei getWidth() const
  {
    return width;
  }

  //
  // 창의 높이를 얻을
  //
  GLsizei getHeight() const
  {
    return height;
  }

  //
  // 창의 크기를 얻을
  //
  void getSize(GLsizei *size) const
  {
    size[0] = getWidth();
    size[1] = getHeight();
  }

  //
  // 윈도우의 화면 비율을 얻을
  //
  GLfloat getAspect() const
  {
    return aspect;
  }

  //
  // 화살표 키 현재의 X 값을 얻을
  //
  GLfloat getArrowX() const
  {
    return static_cast<GLfloat>(arrow[0]);
  }

  //
  // 화살표 키 현재의 Y 값을 얻을
  //
  GLfloat getArrowY() const
  {
    return static_cast<GLfloat>(arrow[1]);
  }

  //
  // 화살표 키의 현재 값을 얻을
  //
  void getArrow(GLfloat *arrow) const
  {
    arrow[0] = getArrowX();
    arrow[1] = getArrowY();
  }

  //
  // Shift 키를 누른 상태에서 화살표 키 현재의 X 값을 얻을
  //
  GLfloat getShiftArrowX() const
  {
    return static_cast<GLfloat>(shift_arrow[0]);
  }

  //
  // Shift 키를 누른 상태에서 화살표 키 현재의 Y 값을 얻을
  //
  GLfloat getShiftArrowY() const
  {
    return static_cast<GLfloat>(shift_arrow[1]);
  }

  //
  // Shift 키를 누른 상태에서 화살표 키의 현재 값을 얻을
  //
  void getShiftArrow(GLfloat *shift_arrow) const
  {
    shift_arrow[0] = getShiftArrowX();
    shift_arrow[1] = getShiftArrowY();
  }

  //
  // 컨트롤 키를 누른 상태에서 화살표 키 현재의 X 값을 얻을
  //
  GLfloat getControlArrowX() const
  {
    return static_cast<GLfloat>(control_arrow[0]);
  }

  //
  // 컨트롤 키를 누른 상태에서 화살표 키 현재의 Y 값을 얻을
  //
  GLfloat getControlArrowY() const
  {
    return static_cast<GLfloat>(control_arrow[1]);
  }

  //
  // 컨트롤 키를 누른 상태에서 화살표 키의 현재 값을 얻을
  //
  void getControlArrow(GLfloat *control_arrow) const
  {
    control_arrow[0] = getControlArrowX();
    control_arrow[1] = getControlArrowY();
  }

  //
  // 마우스의 X 좌표를 얻을
  //
  GLfloat getMouseX() const
  {
    return static_cast<GLfloat>(mouse_x);
  }

  //
  // 마우스의 Y 좌표를 얻을
  //
  GLfloat getMouseY() const
  {
    return static_cast<GLfloat>(mouse_y);
  }

  //
  // 마우스의 현재 위치를 얻을
  //
  void getMouse(GLfloat *position) const
  {
    position[0] = getMouseX();
    position[1] = getMouseY();
  }

  //
  // 마우스 휠의 현재 회전 각도를 얻을
  //
  GLfloat getWheel() const
  {
    return static_cast<GLfloat>(wheel_rotation);
  }

  //
  // 시프트를 누른 상태에서 마우스 휠의 현재 회전 각도를 얻을
  //
  GLfloat getShiftWheel() const
  {
    return static_cast<GLfloat>(shift_wheel_rotation);
  }

  //
  // 컨트롤을 누른 상태에서 마우스 휠의 현재 회전 각도를 얻을
  //
  GLfloat getControlWheel() const
  {
    return static_cast<GLfloat>(control_wheel_rotation);
  }

  //
  // 왼쪽 버튼으로 트랙볼의 회전 변환 행렬을 얻는다
  //
  const GgMatrix getLeftTrackball() const
  {
    return trackball_left.getMatrix();
  }

  //
  // 오른쪽 버튼으로 트랙볼의 회전 변환 행렬을 얻는다
  //
  const GgMatrix getRightTrackball() const
  {
    return trackball_right.getMatrix();
  }
};
