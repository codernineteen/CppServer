#pragma once

#include <mutex>
#include <atomic>

template<typename T>
class LockStack
{
public:
	LockStack(){}

	//prevent copy about LockStack instance
	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);
		_stack.push(std::move(value));
		_condVar.notify_one();
	}

	bool TryPop(T& value)
	{
		//In multi-thread environment, it is meaningless to separate empty check into other subroutine
		//Because the result always can be different even if it was empty or not empty at the moment.

		lock_guard<mutex> lock(_mutex);
		if (_stack.empty())
			return false;

		// empty -> top -> pop
		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}

	void WaitPop(T& value)
	{
		unique_lock<mutex> lock(_mutex);
		_condVar.wait(lock, [this] {return _stack.empty() == false; });
		value = std::move(_stack.top());
		_stack.pop();
	}

private:
	stack<T> _stack;
	mutex _mutex;
	condition_variable _condVar; //To wake a blocked process up
};

//template<typename T>
//class LockFreeStack
//{
//	struct Node
//	{
//		Node(const T& value) : data(make_shared<T>(value)), next(nullptr) {}
//		shared_ptr<T> data;
//		shared_ptr<Node> next;
//	};
//
//public:
//	void Push(const T& value)
//	{
//		shared_ptr<Node> node = make_shared<Node>(value);
//		node->next = std::atomic_load(&_head);
//
//		while (std::atomic_compare_exchange_weak(&_head, &node->next, node) == false)
//		{
//
//		}
//	}
//
//	shared_ptr<T> TryPop()
//	{
//		shared_ptr<Node> oldHead = std::atomic_load(&_head);
//
//		while (oldHead && std::atomic_compare_exchange_weak(&_head, &oldHead, oldHead->next) == false)
//		{
//
//		}
//
//		if (oldHead == nullptr)
//		{
//			return shared_ptr<T>();
//		}
//
//		return oldHead->data;
//	}
//
//private:
//	shared_ptr<Node> _head;
//};



template<typename T>
class LockFreeStack
{
	struct Node;

	struct CountedNodePtr
	{
		int32 externalCount = 0;
		Node* ptr = nullptr;
	};

	struct Node
	{
		Node(const T& value) : data(make_shared<T>(value)) 
		{
		
		}
		shared_ptr<T> data;
		atomic<int32> internalCount = 0;
		CountedNodePtr next;
	};

public:
	void Push(const T& value)
	{
		CountedNodePtr node;
		node.ptr = new Node(value);
		node.externalCount = 1;
		 
		node.ptr->next = _head;
		while (_head.compare_exchange_weak(node.ptr->next, node) == false)
		{
			
		}
	}

	shared_ptr<T> TryPop()
	{
		CountedNodePtr prevHead = _head;
		while (true)
		{
			//gain reference 
			IncreaseHeadCount(prevHead);
			//At this moment, external count is larger or equal than 2 => safe
			Node* ptr = prevHead.ptr;

			if (ptr == nullptr)
				return  shared_ptr<T>();

			//gain ownership(winner is who exchanges ptr->next with _head)
			if (_head.compare_exchange_strong(prevHead, ptr->next))
			{
				shared_ptr<T> res;
				res.swap(ptr->data);

				//external : 1 -> 2(+1) when only one reference occur
				//internal : 0 -> 

				//check whether winner only refer to ptr
				const int32 countIncrease = prevHead.externalCount - 2;
				//fetch_add return a state before operation done
				if (ptr->internalCount.fetch_add(countIncrease) == -countIncrease)
				{ 
					delete ptr;
				}

				//if there was more reference to ptr, just return res

				return res;
			}
			else if(ptr->internalCount.fetch_sub(1) == 1) //loser decerements internalcount
				//when it is 1, it means that the loser is last thread who referred to this pointer
				//so it delete ptr and go out
			{
				//gain reference, but not ownership
				delete ptr;
			}

		}
	}

	void IncreaseHeadCount(CountedNodePtr& prevCounter)
	{
		while (true)
		{
			CountedNodePtr newCounter = prevCounter; //copy previous counter
			newCounter.externalCount++; //interuppt possible

			if (_head.compare_exchange_strong(prevCounter, newCounter))
			{
				//If compare is success
				prevCounter.externalCount = newCounter.externalCount;
				break;
			}
			
		}
	}

private:
	atomic<CountedNodePtr> _head;
};