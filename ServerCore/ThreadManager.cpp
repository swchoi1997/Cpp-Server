#include "pch.h"
#include "ThreadManager.h"
#include "CoreGlobal.h"
#include "CoreTLS.h"

ThreadManager::ThreadManager()
{
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(function<void(void)> callback)
{							// function을 사용하면 lambda, function ptr 등등 다 받을 수있음
	LockGuard guard(_mutex);
	
	_threads.push_back(thread([=]() {
		InitTLS();
		callback();
		DestoryTLS();
		}
	));
	
}

void ThreadManager::Join()
{
	for (std::thread& t : _threads)
	{
		if (t.joinable())
			t.join();
	}
	_threads.clear();
}

void ThreadManager::InitTLS()
{
	static atomic<uint32> SThreadId = 0;
	LThreadId = SThreadId.fetch_add(1);
}

void ThreadManager::DestoryTLS()
{

}
