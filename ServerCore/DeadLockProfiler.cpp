#include "pch.h"
#include "DeadLockProfiler.h"

/*---------------------------
	   DeadLockProfiler
---------------------------*/

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock);
	//아이디를 찾거나 발급한다.

	int32 lockId = 0; // lockId초기화
	auto findIt = _nameToId.find(name); //찾아서 나오면, 이미 등록된 lock이다.
	if (findIt == _nameToId.end()) // 찾아서 발견하지 못하고 마지막을 리턴했으면, 새롭게 발급해준다.
	{
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else // 그렇지 않으면 lockId를 value 값으로 초기화 해줌
	{
		lockId = findIt->second;

	}

	if (_lockStack.empty() == false) // 잡고있는 락이 있었다면
	{
		// 중복락을 허용하니까 기존에 발견되지 않은 케이스라면 데드락 여부 확인
		const int32 prevId = _lockStack.top(); // 마지막에 잡고있던 lock번호를 체크하고
		if (lockId != prevId) // 만약 지금 잡으려고 하던 lock과 다른 lock이라면
		{
			set<int32>& history = _lockHistory[prevId];
			if (history.find(lockId) == history.end()) // 새로운 간선이라면?
			{
				history.insert(lockId);
				CheckCycle();
			}
		}

	}

	_lockStack.push(lockId);

}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard guard(_lock);

	if (_lockStack.empty())
		CRASH("MULTIPLE_UNLOCK");

	// pop하려는 친구의 lockid를 가져와서, _lockstack과 비교했을 때 , 가장 위에 값이 아니라면, push/pop 순서가 꼬인것을 나타낸다.
	int32 lockId = _nameToId[name];
	if (_lockStack.top() != lockId) 
		CRASH("INVALID_UNLOCK");

	_lockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	//사이클 체크 임시 변수 초기화
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoveredOrder = vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);

	for (int32 lockId = 0; lockId < lockCount; lockId++)
		Dfs(lockId);
	
	// 연산기 끝났으면 정리한다.
	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();

}

void DeadLockProfiler::Dfs(int32 here)
{
	if (_discoveredOrder[here] != -1) //이미 방문이 된 시점
		return;

	_discoveredOrder[here] = _discoveredCount++;

	//모든 인접한 정점을 순회한다.
	auto findIt = _lockHistory.find(here);
	if (findIt == _lockHistory.end())
	{
		// 아직 해당 lock을 잡은 상태에서 다른 lock 을 잡은 적이 없다.
		_finished[here] = true;
		return;
	}

	set<int32>& nextSet = findIt->second;
	for (int32 there : nextSet)
	{
		//아직 방문한 적이 없다면, 방문한다.
		if (_discoveredOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		// 이미 방문한 적이 있다면, 순방향 역방향 체크
		// here가  there보다 먼저 발견되었다면, there 가 here의 후손이다.(순방향 간선)
		if (_discoveredOrder[here] < _discoveredOrder[there])
			continue;

		//만약 순방향이 아닌 상태에서 Dfs(there)가 아직 종료하지 않았다면, there는 here의 선조(역방향 간선)
		if (_finished[there] == false)
		{
			printf("%s -> %s\n", _idToName[here], _idToName[there]);
			int32 now = here;
			while (true)
			{
				printf("%s -> %s\n", _idToName[_parent[now]], _idToName[now]);
				now = _parent[now];
				if (now == there)
					break;
			}
			CRASH("DEADLOCK_DETECTED");
		}
	}

	_finished[here] = true;
}

