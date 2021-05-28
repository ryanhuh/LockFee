#pragma once

#include <thread>
#include <chrono>
#include <atomic>
#include <vector>

using namespace std;
using namespace std::chrono;

template<class T>
class LockFreeQ {

private:
	struct NODE {
		NODE(T *pPtr) {
			pNext = nullptr;
			pData = pPtr;
		}

		T *pData;
		NODE* pNext;
	};

	__int64 enQueueCount;
	__int64 deQueueCount;
	__int64 queueSize;

	NODE *volatile head;
	NODE *volatile tail;
public:
	LockFreeQ() {
		head = tail = new NODE(nullptr);
	}
	~LockFreeQ() {
		Init();
	}
	void Init() {
		NODE *ptr;
		while (head->pNext != nullptr) {
			ptr = head->pNext;
			head->pNext = head->pNext->pNext;
			delete ptr;
		}
		tail = head;
	}

	bool CAS(NODE* volatile*addr, NODE *expectedValue, NODE *newValue) {		
		return atomic_compare_exchange_strong( reinterpret_cast<volatile atomic_int*>(addr), reinterpret_cast<int *> (&expectedValue), reinterpret_cast<int>(newValue));
	}

	void EnQ(T *pPtr) {
		NODE *newData = new NODE(pPtr);
		while (true) {
			NODE *last = tail;
			NODE *next = last->pNext;
			if (last != tail) 
				continue;

			if (next != nullptr) {
				CAS(&tail, last, newData);
				continue;
			}

			if (CAS(&last->pNext, nullptr, newData) == false)
				continue;

			CAS(&tail, last, newData);
			break;
		}
		++enQueueCount;
	}

	T* DeQ() {
		if (!head)
			return nullptr;

		while (true) {
			NODE *first = head;
			NODE *next = first->pNext;
			NODE *last = tail;
			NODE *lastNext = last->pNext;
			
			if (last == first) {
				if (lastNext == nullptr) {
					return nullptr;
				}
				else {
					CAS(&tail, last, lastNext);
					continue;
				}
			}
			if (next == nullptr)
				continue;

			T *pRc = next->pData;
			if (CAS(&head, first, next) == false)
				continue;
			first->pNext = nullptr;
			delete first;
			++deQueueCount;
			return pRc;
		}
	}

	__int64 GetCount() {
		return enQueueCount - deQueueCount;
	}
};