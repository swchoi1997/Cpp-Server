#pragma once

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

#ifdef _DEBUG
// Debug����� ServerCore���̺귯���� �����´�.
#pragma comment(lib, "Debug\\ServerCore.lib")
#else
// Release����� ServerCore���̺귯���� �����´�.
#pragma comment(lib, "Release\\ServerCore.lib")
#endif

#include "CorePch.h"