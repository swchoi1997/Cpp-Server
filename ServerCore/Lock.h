#pragma once
#include "Types.h"


/*---------------------------
		RW SpinLock
---------------------------*/

/*
	32 ��Ʈ�� ����ϴµ�, 
	[WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR]
	���� 16 ��Ʈ -> Read Flag ( Shared Lock Count )
								-> �����ؼ� ����ϰ� �ִ� Read Count

	���� 16 ��Ʈ -> Write Flag ( Exclusive Lock Owner ThreadId)	
								-> �ش� Lock�� ȹ���ϰ� �ִ� ThreadId�� ����

	Write���� ���¿��� Read ����
	Read ���� ���¿��� Write �Ұ���
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