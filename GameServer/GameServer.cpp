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

using TL = TypeList<class Player, class Mage, class Knight, class Archer>;

class Player
{
public:
	Player() {
		INIT_TL(Player)
	};

	DECLARE_TL
};

class Knight : public Player
{
public:
	Knight() {
		INIT_TL(Knight)
	};
};

class Mage : public Player
{
public:
	Mage() {
		INIT_TL(Mage)
	};
};

class Archer : public Player
{
public:
	Archer() {
		INIT_TL(Archer)
	};
};

int main() 
{
	{
		Player* player = new Knight();
		bool canCast = CanCast<Knight*>(player);
		Knight* knight = TypeCast<Knight*>(player);

		delete player;
	}
	
	{
		shared_ptr<Knight> knight = MakeShared<Knight>();
		shared_ptr<Player> p1 = TypeCast<Player>(knight);
	}

	return 0;
} 