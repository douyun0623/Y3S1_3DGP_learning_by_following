// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>

// 윈도우의 클라이언트 영역의 크기를 나타내는 상수
constexpr int FRAMEBUFFER_WIDTH = 640;
constexpr int FRAMEBUFFER_HEIGHT = 480;

//#define FRAME_BUFFER_WIDTH 640
//#define FRAME_BUFFER_HEIGHT 480

// 각도(Degree)를 라디언(Radian)으로 변환하는 매크로를 다음과 같이 정의한다. 
#define DegreeToRadian(x) float((x)*3.141592654f/180.0f)


