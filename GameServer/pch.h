#pragma once
//���� Ȱ��Ǵ� ����� ���Խ��� �̸� ���带 ����, ������ ����� �� �ִ� ����
//����: pch�� ������ �����Ǹ� ���� ������ ��ħ

//pch ������
//solution properties -> C/C++ -> precompiled-header tab -> change no using to 'Use' and ��� ���� ����̸����� ����
//pch.cpp������ ���� -> precompiled header -> use���� create�� ���� 

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#ifdef _DEBUG
#pragma comment(lib, "Debug\\ServerCore.lib") // Debug��� �϶� ���� �ھ� ���̺귯�� ��ġ ����
#else
#pragma comment(lib, "Release\\ServerCore.lib") // Debug��� �϶� ���� �ھ� ���̺귯�� ��ġ ����
#endif

#include "CorePch.h"

