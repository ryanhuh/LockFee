// LockFree.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <process.h>
#include <Windows.h>
#include <future>
#include "Lock.h"
#include <string>
using namespace std;
using namespace std::chrono;


CSLockWrap _cs;
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

	void Init() {
		NODE *ptr;
		while (head->pNext != nullptr) {
			ptr = head->pNext;
			head->pNext = head->pNext->pNext;
			delete ptr;
		}
		tail = head;
	}

	bool CAS(NODE* volatile* addr, NODE *expectedValue, NODE *newValue) {

		return atomic_compare_exchange_strong(
			reinterpret_cast<volatile atomic_int*>(addr),
			reinterpret_cast<int *> (&expectedValue),
			reinterpret_cast<int>(newValue));
	}

	void Display() {
		std::cout << "EndQ :" << enQueueCount << " DeQ : " << deQueueCount <<  " Size : " << enQueueCount - deQueueCount <<   endl;
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
		if(enQueueCount % 100000 == 0)
			Display();
	}

	T* DeQ() {
		if (!head)
			return nullptr;
				
		while (true) {
			AutoLockT<CSLockWrap> lock(_cs, TEXT(__FILE__), __LINE__);

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
					//return nullptr;
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
			//Display();
			return pRc;
		}
	}

	int GetLength() {
		return enQueueCount - deQueueCount;
	}
};


LockFreeQ<std::future<bool>> lockFreeQ;


class FutureClass {
public:
	FutureClass() {

	};
	FutureClass(int n) {
		a = n;
	};
	~FutureClass() {};

	bool ThreadTask(LPVOID pParam);
	bool ThreadEndQ(LPVOID pParam);
	int a;
};

bool FutureClass::ThreadTask(LPVOID pParam) {

	int n = *(int*)pParam;
	if( n % 10000 == 0)
		cout << "ThreadID : " << GetCurrentThreadId() << " Job : " << pParam << " value : " << n << endl;
	
	delete pParam;
	return true;
};

bool FutureClass::ThreadEndQ(LPVOID pParam) {
	for (int n = 0;;++n) {

		if (lockFreeQ.GetLength() < 10000)
		{
			int *p = new int;
			*p = n;
			std::future<bool> *Task = new std::future<bool>(async(launch::deferred, &FutureClass::ThreadTask, this, (void*)p));

			lockFreeQ.EnQ(Task);
		}
	}
	return true;
}

unsigned __stdcall DnQThread(LPVOID param) {
	for (;;)
	{
		std::future<bool> *pFuture = (std::future<bool> *)lockFreeQ.DeQ();
		if (pFuture) {
			pFuture->get();
			delete pFuture;
			pFuture = nullptr;
		}		
	}
	return 0;
}

int main(int argc, TCHAR* argv[])
{	
	FutureClass fc;

	lockFreeQ.Init();

	unsigned int webThreadID;
	HANDLE hThread;

	
	for (int n = 0; n < 2; n++) {
		hThread = (HANDLE)_beginthreadex(NULL, 0, DnQThread, nullptr, 0, &webThreadID);
		CloseHandle(hThread);
	}




	fc.ThreadEndQ(nullptr);

	//std::system("pause");
	std::getchar();
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
