#pragma once
//자주 활용되는 헤더를 포함시켜 미리 빌드를 시켜, 빠르게 사용할 수 있는 장점
//단점: pch의 내용이 수정되면 많은 영향을 미침

//pch 설정법
//solution properties -> C/C++ -> precompiled-header tab -> change no using to 'Use' and 사용 중인 헤더이름으로 변경
//pch.cpp설정에 들어가서 -> precompiled header -> use에서 create로 변경 

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#ifdef _DEBUG
#pragma comment(lib, "Debug\\ServerCore.lib") // Debug모드 일때 서버 코어 라이브러리 위치 설정
#else
#pragma comment(lib, "Release\\ServerCore.lib") // Debug모드 일때 서버 코어 라이브러리 위치 설정
#endif

#include "CorePch.h"

