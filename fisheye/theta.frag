#version 150 core
#extension GL_ARB_explicit_attrib_location : enable

//
//   RICOH THETA S 라이브 스트리밍 영상의 평면 전개
//

// 배경 텍스처
uniform sampler2D image;

// 텍스처 좌표
in vec2 texcoord_b;
in vec2 texcoord_f;

// 전후 질감의 혼합비
in float blend;

// 조각의 색
layout (location = 0) out vec4 fc;

void main(void)
{
  // 전후 텍스처의 색상을 샘플링
  vec4 color_b = texture(image, texcoord_b);
  vec4 color_f = texture(image, texcoord_f);

  // 샘플링 한 색상을 혼합하여 조각의 색깔을 추구
  fc = mix(color_f, color_b, blend);
}
