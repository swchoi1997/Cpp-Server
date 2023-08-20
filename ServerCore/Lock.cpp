#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"

void Lock::WriteLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// 동일한 쓰레드가 소유하고 있다면 무조건 성공.
	// 현재 쓰레드 ID는 32비트의 int형으로 되어있고, 
	// WRITE_THREAD_MASK = 0xFFFF0000이라서 Write하려고 시도중인 threadId를 가져옴
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId) 
	{
		// 만약 동일한 쓰레드가 접근한다면, 
		// 동일한 쓰레드가 중복해서 락을 잡기 위해 writecount를 1을 늘려줌
		_writeCount++;
		return;
	}

	// 아무도 소유 및 공유하고 있지 않을 떄
	//  -> 경합해서 소유권을 얻는다.( 상호배타적 )
	//  -> 아무도 쓰지도않고, 읽지도 않고있을 때, 즉 EMPLTY_FLAG일때(쓰려는 시도가 1개도 없을때

	const int64 beginTick = ::GetTickCount64();
	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(OUT expected, desired))
			{
				_writeCount++;
				return;
			}
		}

		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK) 
			CRASH("LOCK TIMEOUT");
		

		this_thread::yield();
	}

}

void Lock::WriteUnLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif
	// ReadLock 다 풀기 전에는 WriteUnlock을 불가능하다.
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
		CRASH("INVALID_UNLOCK_ORDER");

	const int32 lockCount = --_writeCount;
	if (lockCount == 0)
		_lockFlag.store(EMPTY_FLAG);
}

void Lock::ReadLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif
	// 동일한 쓰레드가 Write Lock을 소유하고 있어도 Read는 성공
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId) 
	{
		_lockFlag.fetch_add(1);
		return;
	}

	// 아무도 소유하고 있지않을 때( Write중이 아닐 때) 경합해서 공유 카운트를 올린다.
	const int64 beginTick = ::GetTickCount64();

	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
				return;
		}

		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK TIMEOUT");

		this_thread::yield();

	}
}

void Lock::ReadUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif
	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		CRASH("MULTIPLE_UNLOCK");
}
