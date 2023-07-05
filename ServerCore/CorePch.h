#pragma once
//서버 코드 내부에서만 활용할 pre-compiled header
//Std
#include "Types.h"
#include "CoreGlobal.h"
#include "CoreMacro.h"
#include "CoreTLS.h"
#include "Lock.h"
#include "Container.h"

#include <windows.h>
#include <iostream>
using namespace std;

//WinSock
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

//Memory
#include "ObjectPool.h"
#include "TypeCast.h"
#include "Memory.h"

//Network
#include "SendBuffer.h"


