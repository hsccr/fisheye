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

    // 스왑 간격을 기다리는
    glfwSwapInterval(1);

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

    // 윈도우를 파기
    glfwDestroyWindow(window);
  }

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

    // 컬러 버퍼를 교체
    glfwSwapBuffers(window);

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

      // 윈도우의 화면 비율을 저장
      instance->aspect = static_cast<GLfloat>(width) / static_cast<GLfloat>(height);

      // 전체 창 그릴
      instance->restoreViewport();
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
