#include <iostream>
#include "pch.h"
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include <algorithm>
#include <numeric>

#include "CoreMacro.h"
#include "ThreadManager.h"

class Primes
{
	USE_LOCK;

private:
	int32			_size;
	vector<bool>	_primes;

public:
	Primes(const int32 num) : _size(num), _primes(vector<bool>(num + 1, true))
	{
		_primes[0] = _primes[1] = 0;
	}

	~Primes()
	{
		_primes.clear();
	}

	const vector<bool>& GetPrimes() const
	{
		return _primes;
	}

	int32 Find(int32 threadCount)
	{
		if (threadCount >= _size)
			CRASH("INVALID_THREAD_COUNT");

		int32 quotient = _size / threadCount;
		for (int32 i = 0; i < threadCount; i++)
		{	
			int32 start = i * quotient;
			int32 end = i * quotient + quotient - 1;

			if (i == threadCount - 1)
				end = _size;

			GThreadManager->Launch([this, start, end]() {
				CheckPrime(start, end);
				}
			);
		}
		GThreadManager->Join();

		return CalculateResult();
	}

private:
	void CheckPrime(int32 start, int32 end)
	{
		
		for (int64 i = 2; i * i <= end; i++)
		{
			CheckPrimeActual(start, end, i);
		}
	}

	void CheckPrimeActual(int32 start, int32 end, int64 index)
	{
		for (int32 j = max(index * index, (start + index - 1) / index * index); j <= end; j += index)
		{
			WRITE_LOCK;
			_primes[j] = false;
		}
	}

	int32 CalculateResult()
	{	
		/*for (int i = 0; i <= _size; i++)
		{
			cout << i << "(" << _primes[i] << ")  ";
			if (i != 0 && i % 10 == 0)
				cout << endl;
		}*/
		return accumulate(_primes.begin(), _primes.end(), 0);

	}
};


int main()
{
	const int32 MAX_NUMBER = 1'000'000;
	

	Primes* prime = new Primes(MAX_NUMBER);
	int32 result = prime->Find(10);

	cout << result << endl;
}

	