#pragma once

#include <thread>
#include <functional>



/*-----------------------
     ThreadManager
-----------------------*/

class ThreadManager
{
public:
	ThreadManager();

	~ThreadManager();

	void Launch(function<void(void)> callback);
	void Join();

	static void InitTLS();
	static void DestoryTLS();

	

private:
	Mutex			_mutex;
	vector<thread>	_threads;
};

