#version 150 core
#extension GL_ARB_explicit_attrib_location : enable

//
// 등장 방형 도법의 텍스처를 샘플링
//

// 배경 텍스처의 반지름과 중심 위치
uniform vec4 circle;

// 배경 텍스처
uniform sampler2D image;

// 배경 텍스처 크기
vec2 size = textureSize(image, 0);

// 배경 텍스처의 텍스처 공간상의 스케일
vec2 scale = vec2(-0.15915494, -0.31830989) / circle.st;

// 배경 텍스처의 텍스처 공간의 중심 위치
vec2 center = circle.pq + 0.5;

// 시선 벡터
in vec4 vector;

// 조각의 색
layout (location = 0) out vec4 fc;

void main(void)
{
  // 시선 벡터를 정규화하기
  vec4 orientation = normalize(vector);

  // 텍스처 좌표를 구하는
  vec2 u = orientation.xy;
  vec2 v = vec2(orientation.z, length(orientation.xz));
  vec2 texcoord = atan(u, v) * scale + center;

  // 픽셀의 음영을 요구
  fc = texture(image, texcoord);
}
