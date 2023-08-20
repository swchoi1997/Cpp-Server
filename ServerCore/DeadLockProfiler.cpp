#include "pch.h"
#include "DeadLockProfiler.h"

/*---------------------------
	   DeadLockProfiler
---------------------------*/

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock);
	//���̵� ã�ų� �߱��Ѵ�.

	int32 lockId = 0; // lockId�ʱ�ȭ
	auto findIt = _nameToId.find(name); //ã�Ƽ� ������, �̹� ��ϵ� lock�̴�.
	if (findIt == _nameToId.end()) // ã�Ƽ� �߰����� ���ϰ� �������� ����������, ���Ӱ� �߱����ش�.
	{
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else // �׷��� ������ lockId�� value ������ �ʱ�ȭ ����
	{
		lockId = findIt->second;

	}

	if (_lockStack.empty() == false) // ����ִ� ���� �־��ٸ�
	{
		// �ߺ����� ����ϴϱ� ������ �߰ߵ��� ���� ���̽���� ����� ���� Ȯ��
		const int32 prevId = _lockStack.top(); // �������� ����ִ� lock��ȣ�� üũ�ϰ�
		if (lockId != prevId) // ���� ���� �������� �ϴ� lock�� �ٸ� lock�̶��
		{
			set<int32>& history = _lockHistory[prevId];
			if (history.find(lockId) == history.end()) // ���ο� �����̶��?
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

	// pop�Ϸ��� ģ���� lockid�� �����ͼ�, _lockstack�� ������ �� , ���� ���� ���� �ƴ϶��, push/pop ������ ���ΰ��� ��Ÿ����.
	int32 lockId = _nameToId[name];
	if (_lockStack.top() != lockId) 
		CRASH("INVALID_UNLOCK");

	_lockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	//����Ŭ üũ �ӽ� ���� �ʱ�ȭ
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoveredOrder = vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);

	for (int32 lockId = 0; lockId < lockCount; lockId++)
		Dfs(lockId);
	
	// ����� �������� �����Ѵ�.
	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();

}

void DeadLockProfiler::Dfs(int32 here)
{
	if (_discoveredOrder[here] != -1) //�̹� �湮�� �� ����
		return;

	_discoveredOrder[here] = _discoveredCount++;

	//��� ������ ������ ��ȸ�Ѵ�.
	auto findIt = _lockHistory.find(here);
	if (findIt == _lockHistory.end())
	{
		// ���� �ش� lock�� ���� ���¿��� �ٸ� lock �� ���� ���� ����.
		_finished[here] = true;
		return;
	}

	set<int32>& nextSet = findIt->second;
	for (int32 there : nextSet)
	{
		//���� �湮�� ���� ���ٸ�, �湮�Ѵ�.
		if (_discoveredOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		// �̹� �湮�� ���� �ִٸ�, ������ ������ üũ
		// here��  there���� ���� �߰ߵǾ��ٸ�, there �� here�� �ļ��̴�.(������ ����)
		if (_discoveredOrder[here] < _discoveredOrder[there])
			continue;

		//���� �������� �ƴ� ���¿��� Dfs(there)�� ���� �������� �ʾҴٸ�, there�� here�� ����(������ ����)
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

