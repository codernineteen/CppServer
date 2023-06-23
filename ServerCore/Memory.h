#pragma once
#include "Allocator.h"

template<typename Type, typename ...Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(XALLOC(sizeof(Type)));

	//placement new
	new(memory) Type(std::forward<Args>(args)...); //we already have memory space, just call constructor on the space.
	//...Arg achieves Polymorphism by taking arguments dynamically
	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type(); //call destructor
	XRELEASE(obj);
}
