#version 150 core
#extension GL_ARB_explicit_attrib_location : enable

//
// 텍스처 좌표 위치의 화소 색상을 그대로 사용
//

// 배경 텍스처
uniform sampler2D image;

// 텍스처 좌표
in vec2 texcoord;

// 조각의 색
layout (location = 0) out vec4 fc;

void main(void)
{
  // 픽셀의 음영을 요구
  fc = texture(image, texcoord);
}
