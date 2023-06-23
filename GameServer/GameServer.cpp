// GameServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread> 
#include <atomic>
#include <mutex>
#include <windows.h>
#include "CoreMacro.h"
#include "ThreadManager.h"
#include "Memory.h"
#include "Allocator.h"

class Knight
{
public:
	Knight()
	{
		cout << "knight constructed" << endl;
		_hp = 0;
	}

	Knight(int32 hp) : _hp(hp) {}

	~Knight()
	{
		cout << "Knight Destructed" << endl;
	}
	
	int32 _hp;
};

int main() 
{
	//There are more cases in that we allocates memory
	//like STL data structures
	//STL data structures basically allocated by default new and delete
	//But we can pass the Allocator parameter for it 
	//At the same time, we should satisfy the data structure which STL expects to

	Vector<Knight> v(100);
}