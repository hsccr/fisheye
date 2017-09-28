// 윈도우 관련 작업
#include "Window.h"

// 평면 배포 설정 목록
#include "ExpansionShader.h"

// OpenCV 의한 비디오 캡처
#include "CamCv.h"

//
// 설정
//

// 배경 이미지를 가져 오는 데 사용하는 장치
// # define CAPTURE_INPUT 0 // 0 번 캡처 장치에서 입력
#define CAPTURE_INPUT "sp360.mp4"// Kodak SP360 4K의 Fish Eye 이미지
// # define CAPTURE_INPUT "theta.mp4"// THETA S의 Equirectangular 이미지

// 배경 이미지를 배포하는 방법 (ExpansionShader.h 참조)
// constexpr int shader_selection (6); // Kodak SP360 4K
// constexpr int shader_selection (7); // THETA S의 Dual Fisheye 이미지
constexpr int shader_selection (2); // THETA S의 Equirectangular 이미지

// 배경 이미지 배포에 사용되는 버텍스 쉐이더 소스 파일 이름
const char * const capture_vsrc (shader_type [shader_selection] .vsrc);

// 배경 이미지 배포에 사용되는 조각 쉐이더 소스 파일 이름
const char * const capture_fsrc (shader_type [shader_selection] .fsrc);

// 배경 이미지를 가져 오는 데 사용하는 카메라의 해상도 (0이라면 카메라에서 취득)
const int capture_width (shader_type [shader_selection] .width);
const int capture_height (shader_type [shader_selection] .height);

// 배경 이미지를 가져 오는 데 사용하는 카메라의 프레임 속도 (0이라면 카메라에서 취득)
constexpr int capture_fps (0);

// 배경 이미지의 관심 영역
const float * const capture_circle (shader_type [shader_selection] .circle);

// 배경 이미지를 그릴 때 사용하는 메쉬 격자 점수
constexpr int screen_samples (1271);

// 배경색은 표시되지 않지만 합성시에 0으로 해 둘 필요가있다
constexpr GLfloat background [] = {0.0f, 0.0f, 0.0f, 0.0f};

//
// 메인
//

int main ()
{
  // 카메라 사용하기
  CamCv camera;
  if (! camera.open (CAPTURE_INPUT, capture_width, capture_height, capture_fps))
  {
    std :: cerr << "Can not open capture device \n";
    return EXIT_FAILURE;
  }
  camera.start ();

  // 창을 만들
  Window window;

  // 창을 열었다 여부 확인
  if (! window.get ())
  {
    // 창을 열지 않았다
    std :: cerr << "Can not open GLFW window \n";
    return EXIT_FAILURE;
  }

  // 배경 그리기위한 쉐이더 프로그램을로드
  const GLuint expansion (ggLoadShader (capture_vsrc, capture_fsrc));
  if (! expansion)
  {
    // 쉐이더가 적재되지
    std :: cerr << "Can not create program object \n";
    return EXIT_FAILURE;
  }

  // uniform 변수의 위치를 ​​지정하는
  const GLuint gapLoc (glGetUniformLocation (expansion, "gap"));
  const GLuint screenLoc (glGetUniformLocation (expansion, "screen"));
  const GLuint focalLoc (glGetUniformLocation (expansion, "focal"));
  const GLuint rotationLoc (glGetUniformLocation (expansion, "rotation"));
  const GLuint circleLoc (glGetUniformLocation (expansion, "circle"));
  const GLuint imageLoc (glGetUniformLocation (expansion, "image"));

  // 배경의 질감을 만드는
  // 폴리곤으로 뷰포트 전체를 채우기 때문에 배경은 표시되지 않는다.
  // GL_CLAMP_TO_BORDER에두면 질감의 외부가 GL_TEXTURE_BORDER_COLOR되기 때문에, 이것이 배경색된다.
  const GLuint image ([] () {GLuint image; glGenTextures (1, & image); return image;} ());
  glBindTexture (GL_TEXTURE_2D, image);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, camera.getWidth (), camera.getHeight (), 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterfv (GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, background);

  // 배경 그리기위한 메쉬를 만들
  // 정점 좌표 값을 vertex shader에서 생성하기 때문에 VBO는 필요 없다
  const GLuint mesh([]() { GLuint mesh; glGenVertexArrays(1, &mesh); return mesh; } ());

  // 은면 소거를 설정
  glDisable (GL_DEPTH_TEST);
  glDisable (GL_CULL_FACE);

  // 윈도우가 열려있는 동안 반복
  while (! window.shouldClose ())
  {
    // 배경 이미지 배포에 사용 셰이더 프로그램 사용하기
    glUseProgram (expansion);

    // 화면의 사각형 격자 점수
    // 표본 점 수 (정점) n = x * y를 할 때, 이에 비율 a = x / y를 걸면
    // a * n = x * x가되기 때문 x = sqrt (a * n) y = n / x; 구할 수있다.
    //이 방법은 정점 속성을 가지고 있지 않기 때문에 실행 중에 표본 점의 수와 비율의 변경이 용이하다.
    const GLsizei slices (static_cast <GLsizei> (sqrt (window.getAspect () * screen_samples)));
    const GLsizei stacks (screen_samples / slices - 1); // 그리기 인스턴스의 수이므로 먼저 1을 뺀 둔다.

    // 스크린의 격자 간격
    // 클리핑 공간 전체를 채울 사각형은 [-1, 1]의 범위 즉 가로 세로 2의 크기이기 때문에,
    // 그것을 가로 세로 (격자 수 - 1)로 나누어 격자 간격을 구한다.
    glUniform2f (gapLoc, 2.0f / (slices - 1), 2.0f / stacks);

    // 화면 크기와 중심 위치
    // screen [0] = (right - left) / 2
    // screen [1] = (top - bottom) / 2
    // screen [2] = (right + left) / 2
    // screen [3] = (top + bottom) / 2
    const GLfloat screen [] = {window.getAspect (), 1.0f, 0.0f, 0.0f};
    glUniform4fv (screenLoc, 1, screen);

    // 스크린까지의 초점 거리
    // window.getWheel ()는 [-100, 49]의 범위를 돌려 준다.
    // 따라서 초점 거리 focal는 [1 / 3, 1]의 범위가된다.
    // 이것은 초점 거리가 길어짐에 따라 변화가 커진다.
    glUniform1f (focalLoc, -50.0f / (window.getWheel () - 50.0f));

    // 배경에 대한 시선의 회전 행렬
    glUniformMatrix4fv (rotationLoc, 1, GL_TRUE, window.getLeftTrackball (). get ());

    // 텍스처의 반지름과 중심 위치
    // circle [0] = 이미지 써클의 x 방향의 반경
    // circle [1] = 이미지 써클의 y 방향의 반경
    // circle [2] = 이미지 서클의 중심의 x 좌표
    // circle [3] = 이미지 서클의 중심의 y 좌표
    const GLfloat circle [] =
    {
        capture_circle [0] + window.getShiftWheel () * 0.001f,
        capture_circle [1] + window.getShiftWheel () * 0.001f,
        capture_circle [2] + (window.getShiftArrowX () - window.getControlArrowX ()) * 0.001f,
        capture_circle [3] + (window.getShiftArrowY () + window.getControlArrowY ()) * 0.001f
    };
    glUniform4fv (circleLoc, 1, circle);

    // 캡처 한 이미지를 배경의 텍스처로 전송
    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, image);
    camera.transmit ();

    // 텍스처 유닛을 지정
    glUniform1i (imageLoc, 0);

    // 메쉬 그릴
    glBindVertexArray (mesh);
    glDrawArraysInstanced (GL_TRIANGLE_STRIP, 0, slices * 2, stacks);

    // 컬러 버퍼를 교체하여 이벤트를 검색
    window.swapBuffers ();
   }
}