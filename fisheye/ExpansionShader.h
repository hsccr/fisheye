#pragma once

//
// 평면 전개에 사용 쉐이더
//

// 쉐이더 세트 및 매개 변수
struct ExpansionShader
{
  // 버텍스 쉐이더 소스 프로그램의 파일 이름
  const char * vsrc;

  // 조각 쉐이더 소스 프로그램의 파일 이름
  const char * fsrc;

  // 카메라의 해상도
  const int width, height;

  // 이미지 서클의 반지름과 중심 위치
  const float circle [4];
};

// 쉐이더의 종류
constexpr ExpansionShader shader_type [] =
{
  // 0 : 일반 카메라
  { "fixed.vert", "normal.frag", 640, 480, 1.0f, 1.0f, 0.0f, 0.0f},

  // 1 : 일반 카메라 시점을 회전
  { "rectangle.vert", "normal.frag", 640, 480, 1.0f, 1.0f, 0.0f, 0.0f},

  // 2 : 등장 방형 도법의 이미지 (세로 선을 지우려면 GL_CLAMP_TO_BORDER을 GL_REPEAT합니다)
  { "panorama.vert", "panorama.frag", 1280, 720, 1.0f, 1.0f, 0.0f, 0.0f},

  // 3 : 180 ° 어안 카메라 : 3.1415927 / 2 (≒ 180 ° / 2)
  { "fisheye.vert", "normal.frag", 1280, 720, 1.570796327f, 1.570796327f, 0.0f, 0.0f},

  // 4 : 180 ° 어안 카메라 (FUJINON FE185C046HA-1 + SENTECH STC-MCE132U3V) : 3.5779249 / 2 (≒ 205 ° / 2)
  { "fisheye.vert", "normal.frag", 1280, 1024, 1.797689129f, 1.797689129f, 0.0f, 0.0f},

  // 5 : 206 ° 어안 카메라 (Kodak PIXPRO SP360 4K 손떨림 보정 있음) : 3.5953783 / 2 (≒ 206 ° / 2)
  { "fisheye.vert", "normal.frag", 1440, 1440, 1.797689129f, 1.797689129f, 0.0f, 0.0f},

  // 6 : 235 ° 어안 카메라 (Kodak PIXPRO SP360 4K 손떨림 보정 없음) : 4.1015237 / 2 (≒ 235 ° / 2)
  { "fisheye.vert", "normal.frag", 1440, 1440, 2.050761871f, 2.050761871f, 0.0f, 0.0f},

  // 7 : RICHO THETA의 USB 라이브 스트리밍 영상 : (수동 조정에서 결정한 값)
  { "theta.vert", "theta.frag", 1280, 720, 1.003f, 1.003f, 0.0f, -0.002f},

  // 8 : RICHO THETA의 HDMI 라이브 스트리밍 영상 : (수동 조정에서 결정한 값)
  { "theta.vert", "theta.frag", 1920, 1080, 1.003f, 1.003f, 0.0f, -0.002f}
};