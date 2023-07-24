#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
// Debug모드의 ServerCore라이브러리를 가져온다.
#pragma comment(lib, "Debug\\ServerCore.lib")
#else
// Release모드의 ServerCore라이브러리를 가져온다.
#pragma comment(lib, "Release\\ServerCore.lib")
#endif

#include "CorePch.h"