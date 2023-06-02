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
#include "RefCounting.h"

// disadvantages of custom shared pointer
// 1. can't use already-made class (need to inherit reference countable class)
// 2. Cycle problem 
/**
	In PVP system, there are two knights who can refer to each other
	knight A -> knight B
	knight B -> knight A

	k A = nullptr;
	k B = nullptr

	Ref count can never be zero because of reference cycle

	One of the ways to prevent this cycle is that use raw pointer while only one stakeholder increase/decrease ref count.

*/

class Dummy
{
public:

private:

};

int main() {
	// unique ptr : similar to raw pointer , put doesn't allow copy operation of the pointer. we can use std::move to move its ownership
	// 
	// shared Ptr :

	//Create [T* | Reference counting block*] at the same time
	shared_ptr<Dummy> sptr = make_shared<Dummy>();
	//if we create shared_ptr with constructor,
	// it allocates two memory spaces separately
	//Create [T*], [RefCountingBlock*]
	shared_ptr<Dummy> sptr2 = sptr;

	// weak ptr : 
	weak_ptr<Dummy> wpr = sptr; //weak pointer can interoperate with shared pointer

	//The difference is that we need to check the existence of pointer by calling a expired method
	bool exists = wpr.expired();
	// or we can just cast to shared pointer again
	shared_ptr<Dummy> spr2 = wpr.lock();
	// The purpose of using weak pointer is to check whether the object is destructed or not by refering to ref count block

}