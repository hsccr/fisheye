#version 150 core

//
// 시선 회전하는
//

// 관점과 텍스처 이미지 평면까지의 거리
const float distance = 5.0;

// 스크린의 격자 간격
uniform vec2 gap;

// 스크린의 크기와 중심 위치
uniform vec4 screen;

// 스크린까지의 초점 거리
uniform float focal;

// 화면을 회전 변환 행렬
uniform mat4 rotation;

// 배경 텍스처의 반지름과 중심 위치
uniform vec4 circle;

// 배경 텍스처
uniform sampler2D image;

// 배경 텍스처 크기
vec2 size = textureSize(image, 0);

// 배경 텍스처의 텍스처 공간상의 스케일
vec2 scale = vec2(-0.5 * size.y / size.x, 0.5) / circle.st;

// 배경 텍스처의 텍스처 공간의 중심 위치
vec2 center = circle.pq + 0.5;

// 텍스처 좌표
out vec2 texcoord;

void main(void)
{
  // 정점 위치
  // 각 정점에서 gl_VertexID가 0, 1, 2, 3, ...과 같이 지정되기 때문에,
  // x = gl_VertexID >> 1 = 0, 0, 1, 1, 2, 2, 3, 3, ...
  // y = 1 - (gl_VertexID & 1) = 1, 0, 1, 0, 1, 0, 1, 0, ...
  //처럼 GL_TRIANGLE_STRIP위한 정점 좌표 값을 얻을 수있다.
  // y에 gl_InstaceID을 더하면 glDrawArrayInstanced ()의 인스턴스마다 y가 변화한다.
  // 여기에 격자 간격 gap을 들여 1을 빼면 화면 [-1, 1] 범위의 점군 position이 얻어진다.
  int x = gl_VertexID >> 1;
  int y = gl_InstanceID + 1 - (gl_VertexID & 1);
  vec2 position = vec2(x, y) * gap - 1.0;

  // 정점 위치를 그대로 래스터 라이저에 보내면 클리핑 공간 전면 그릴
  gl_Position = vec4(position, 0.0, 1.0);

  // 시선 벡터
  // position로 스크린의 크기 screen.st을 들여 중심 위치 screen.pq을 더하면
  // 스크린상의 점의 위치 p가 얻을 수 있기 때문 원점의 관점에서이 점을 향하는 시선은
  // 초점 거리 focal을 Z 좌표를 사용하여 (p, -focal)가된다.
  // 이것을 회전 한 후 정규화하여 그 방향의 시선 단위 벡터를 얻는다.
  vec2 p = position * screen.st + screen.pq;
  vec4 vector = normalize(rotation * vec4(p, -focal, 0.0));

  // 텍스처 좌표 (vector.z 대신 scale의 부호를 반전하고있다)
  texcoord = vector.xy * scale / vector.z + center;
}
