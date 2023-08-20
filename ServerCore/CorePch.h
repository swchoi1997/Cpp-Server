#pragma once

//Pch 는 Pre Compile Header라는 의미
// ServerCore에서 공용으로 참조할 애들을 다 넣어두는 곳이다.
// 이미 pch가 있는데 새로 하나를 더 파는 이유는 , DummyClient, GameServer에서도 참조할 수 있게 만들기 위해서이다.

#include "Types.h"
#include "CoreGlobal.h"
#include "CoreTLS.h"
#include "CoreMacro.h"

#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <Windows.h>
#include <iostream>
using namespace std;

#include "Lock.h"



