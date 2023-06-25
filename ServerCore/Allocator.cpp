#include "pch.h"
#include "Allocator.h"
#include "Memory.h"

void* BaseAllocator::Alloc(int32 size)
{
	return ::malloc(size);
}

void BaseAllocator::Release(void* ptr)
{
	::free(ptr);
}


/**
	Stomp Allocator
*/

void* StompAllocator::Alloc(int32 size)
{
	//Why we subtract one from size + PAGE_SIZE and then divide it again?
	// For example, assume that we got a 4 from size input
	// then we add size of page to it and subtract 1 -> 4096 + 4096 - 1
	// == 8195
	// if we divide it agian with page size -> 8195 / 2 = 1
	const int64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	const int64 dataOffset = pageCount * PAGE_SIZE - size;

	void* baseAddress = ::VirtualAlloc(NULL, pageCount, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	return static_cast<void*>(static_cast<int8*>(baseAddress) + dataOffset);
}

void StompAllocator::Release(void* ptr)
{
	const int64 address = reinterpret_cast<int64>(ptr);
	const int64 baseAddress = address - (address % PAGE_SIZE);
	::VirtualFree(reinterpret_cast<void*>(ptr), 0, MEM_RELEASE);
}


/**
	Pool Allocator
*/

void* PoolAllocator::Alloc(int32 size)
{
	return GMemory->Allocate(size);
}

void PoolAllocator::Release(void* ptr)
{
	GMemory->Release(ptr);
}
