#pragma once

/**
	Reference count
*/

class RefCountable
{
public:
	RefCountable() : _refCount(1) {};
	virtual ~RefCountable() {};

	int32 GetRefCount() { return _refCount.load(); }

	int32 AddRef() { return ++_refCount;}
	int32 ReleaseRefCount() 
	{
		int32 refCount = --_refCount;
		if (refCount == 0)
		{
			delete this;
		}
		return refCount;
	}

protected:
	atomic<int32> _refCount; // for multi threads env
};


/**
	Smart Pointer
*/

template<typename T>
class TSharedPtr
{
public:
	TSharedPtr() {}
	TSharedPtr(T* ptr) { Set(ptr); }
	 
	//copy
	TSharedPtr(const TSharedPtr& rhs) { Set(rhs._ptr); }
	//move
	TSharedPtr(TSharedPtr&& rhs)
	{
		_ptr = rhs._ptr;
		rhs._ptr = nullptr;
	}
	//copy between inherited objects
	template<typename U>
	TSharedPtr(const TSharedPtr<U>& rhs) { Set(static_cast<T*>(rhs._ptr)); }

	~TSharedPtr()
	{
		Release();
	}

public:
	//operator overloading
	//copy
	TSharedPtr& operator=(const TSharedPtr& rhs)
	{
		if (_ptr != rhs._ptr)
		{
			Release();
			Set(rhs._ptr);
		}
		return *this; //return ref
	}

	//move
	TSharedPtr& operator=( TSharedPtr&& rhs)
	{
		Release();
		_ptr = rhs._ptr;
		rhs._ptr = nullptr;

		return *this;
	}

	//compare
	bool		operator==(const TSharedPtr& rhs) const { return _ptr == rhs._ptr; }
	bool		operator==(T* ptr) { return _ptr == ptr; }
	bool		operator!=(const TSharedPtr& rhs) const { return _ptr != rhs._ptr; }
	bool		operator!=(T* ptr) { return _ptr != ptr; }
	bool		operator<(const TSharedPtr& rhs) { return _ptr < rhs._ptr; }

	//return raw pointer
	T*			operator*() { return _ptr; }
	const T*	operator*() const { return _ptr; }
				operator T* () const { return _ptr; }
	T*			operator->() { return _ptr; }
	const T*	operator->() const { return _ptr; }





	//null check
	bool isNull()
	{
		return _ptr == nullptr;
	}

private:
	inline void Set(T* ptr)
	{
		_ptr = ptr;
		if (ptr)
		{
			ptr->AddRef();
		}
	}

	inline void Release()
	{
		if (_ptr != nullptr)
		{
			_ptr->ReleaseRefCount();
			_ptr = nullptr;
		}
	}

private:
	T* _ptr = nullptr;
};
