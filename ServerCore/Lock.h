#pragma once
#include "Types.h"


/*---------------------------
		RW SpinLock
---------------------------*/

/*
	32 비트를 사용하는데, 
	[WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR]
	상위 16 비트 -> Read Flag ( Shared Lock Count )
								-> 공유해서 사용하고 있는 Read Count

	하위 16 비트 -> Write Flag ( Exclusive Lock Owner ThreadId)	
								-> 해당 Lock을 획득하고 있는 ThreadId를 기입

	Write중인 상태에서 Read 가능
	Read 중인 상태에서 Write 불가능
*/

class Lock
{
	enum : uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000,
		READ_COUNT_MASK = 0x0000'FFFF,
		EMPTY_FLAG = 0x0000'0000
	};

private:
	Atomic<uint32> _lockFlag = EMPTY_FLAG;
	uint16 _writeCount = 0;

public:
	void WriteLock(const char* name);
	void WriteUnLock(const char* name);
	void ReadLock(const char* name);
	void ReadUnlock(const char* name);

};

/*---------------------------
		Lock Guards
---------------------------*/

class ReadLockGuard
{
private:
	Lock& _lock;
	const char* _name;

public:
	ReadLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name)
	{
		_lock.ReadLock(name);
	}

	~ ReadLockGuard()
	{
		_lock.ReadUnlock(_name);
	}
};


class WriteLockGuard
{
private:
	Lock& _lock;
	const char* _name;

public:
	WriteLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name)
	{
		_lock.WriteLock(name);
	}

	~WriteLockGuard()
	{
		_lock.WriteUnLock(_name);
	}
};