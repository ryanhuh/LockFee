/**
 * @file Lock.h
 * @brief 쓰레드 동기화에 필요한 클래스들
 */

#pragma once

//#include "Config.h"
#include <Windows.h>

#define ASSERT(f) void;

#ifndef SPIN_COUNT_FOR_SPIN_LOCK
#define SPIN_COUNT_FOR_SPIN_LOCK (0x1000)
#endif

#ifndef SLEEP_TIME_FOR_SPIN_LOCK
#define SLEEP_TIME_FOR_SPIN_LOCK (0x0)
#endif

#ifndef SIZE_LINE_BUFFER
#ifdef _EXPO
#define SIZE_LINE_BUFFER (10240 * 2)
#else
#define SIZE_LINE_BUFFER (10240)
#endif
#endif


#ifndef MAX_BUFFERRED_LOG_LINE
#define MAX_BUFFERRED_LOG_LINE (256)
#endif


#ifndef SIZE_LOG_BUFFER
#define SIZE_LOG_BUFFER (8 * 1024)
#endif

namespace LOCK {

/**
 * @class EmptyLock
 */
class EmptyLock
{
public:
	void Lock() {}
	void Unlock() {}
};

/**
 * @class CSLock
 */
class CSLock
{
public:
	CSLock() { ::InitializeCriticalSection(&_cs); }
	~CSLock() { ::DeleteCriticalSection(&_cs); }

	void Lock() { ::EnterCriticalSection(&_cs); }
	void Unlock() { ::LeaveCriticalSection(&_cs); }

private:
	CRITICAL_SECTION _cs;
};

/**
 * @class SpinLockT
 */
template<DWORD t_spinCount, DWORD t_sleepTime>
class SpinLockT
{
public:
	SpinLockT() : _lockCount(0), _lockThreadId(0) {}
	~SpinLockT() {}

	void Lock()
	{
		LONG currentThreadId = (LONG)::GetCurrentThreadId();

		if (currentThreadId != _lockThreadId)
		{
			DWORD spinCount = 0;

			while (::InterlockedCompareExchange(&_lockThreadId, currentThreadId, 0))
			{
				if (++spinCount == t_spinCount)
				{
					::Sleep(t_sleepTime);
					spinCount = 0;
				}
			}
		}

		++_lockCount;
	}

	void Unlock()
	{
		ASSERT((LONG)::GetCurrentThreadId() == _lockThreadId);
		ASSERT(_lockCount > 0);

		if (0 == --_lockCount)
			_lockThreadId = 0;
	}

private:
	volatile LONG _lockThreadId;
	LONG _lockCount;
};

/**
 * @typedef SpinLock
 */
typedef SpinLockT<SPIN_COUNT_FOR_SPIN_LOCK, SLEEP_TIME_FOR_SPIN_LOCK> SpinLock;

/**
 * @var g_lockIdCounter
 */
__declspec(selectany) volatile LONG g_lockIdCounter = -1;

/**
 * @class LockT
 */
template<typename TLockPrimitive>
class LockT
{
public:
	LockT(const wchar_t * desc = L"empty")
	:	_id(::InterlockedIncrement(&g_lockIdCounter)),
		_desc(desc) {
	}

	~LockT() {
	}

	void Lock(const wchar_t * filename, int line)
	{
		//LockStackEntry lse(_id, _desc, filename, line, true);

		//th_pThreadBase->_lockStack.push_back(lse);
		//th_pThreadBase->LockStackPush(GetCurrentThreadId(),lse);
				
		_lock.Lock();
		//th_pThreadBase->_lockStack.back()._trying = false;
		//th_pThreadBase->SetTrying(GetCurrentThreadId(),false);
	}

	void Unlock()
	{
		//th_pThreadBase->LockStackPop(GetCurrentThreadId());
		//th_pThreadBase->_lockStack.pop_back();
		_lock.Unlock();
	}

private:
	int _id;
	const wchar_t * _desc;

	TLockPrimitive _lock;
};

/**
 * @typedef EmptyLockWrap
 */
typedef LockT<EmptyLock>	EmptyLockWrap;

/**
 * @typedef CSLockWrap
 */
typedef LockT<CSLock>		CSLockWrap;

/**
 * @typedef SpinLockWrap
 */
typedef LockT<SpinLock>		SpinLockWrap;

/**
 * @class AutoLockT using LockClass directly
 */
template<typename TLockClass>
class AutoLockT
{
public:
	AutoLockT(TLockClass& lock) : _lock(lock)
	{
		_lock.Lock();
	}

	~AutoLockT()
	{
		_lock.Unlock();
	}

private:
	TLockClass & _lock;

	AutoLockT& operator=(const AutoLockT&);
};

/**
 * @class AutoLockT using LockT wrapper class
 */

template<class TLockClass>
class AutoLockT< LockT<TLockClass> >
{
public:
	AutoLockT(LockT<TLockClass>& lockWrap, const wchar_t* filename, int line) : _lockWrap(lockWrap)
	{
		_lockWrap.Lock(filename, line);
	}

	~AutoLockT()
	{
		_lockWrap.Unlock();
	}

private:
	LockT<TLockClass>& _lockWrap;

	AutoLockT& operator=(const AutoLockT&);
};

} /* LOCK */

using namespace LOCK;
