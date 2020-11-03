#ifndef THREAD_SAFE_QUEUE
#define THREAD_SAFE_QUEUE
#include <queue>
#include <stack>
#include <mutex>
#include <vector>
#include <condition_variable>

template <class T>
class ThreadSafeQueue
{
private:
	std::queue<T> q;
	std::mutex m;
	std::condition_variable c;
public:
	ThreadSafeQueue(void)
		: q()
		, m()
		, c()
	{}

	~ThreadSafeQueue(void)
	{}

	size_t size()
	{
		std::unique_lock<std::mutex> lock(m);
		return q.size();
	}

	void push(T t)
	{
		std::lock_guard<std::mutex> lock(m);
		q.push(t);
		c.notify_one();
	}

	T pop(void)
	{
		std::unique_lock<std::mutex> lock(m);
		while(q.empty())
		{
			c.wait(lock);
		}
		T val = q.front();
		q.pop();
		return val;
	}

	T peek(void)
	{
		std::unique_lock<std::mutex> lock(m);
		while(q.empty())
		{
			c.wait(lock);
		}
		T val = q.front();
		return val;
	}

	void clear()
	{
		std::unique_lock<std::mutex> lock(m);
		queue<T> empty;
		std::swap(q, empty);
	}
};

template <class T>
class ThreadSafeStack
{
private:
	std::stack<T> s;
	std::mutex m;
	std::condition_variable c;
public:
	ThreadSafeStack(void)
		: s()
		, m()
		, c()
	{}

	~ThreadSafeStack(void)
	{}

	size_t size()
	{
		std::unique_lock<std::mutex> lock(m);
		return s.size();
	}

	void push(T t)
	{
		std::lock_guard<std::mutex> lock(m);
		s.push(t);
		c.notify_one();
	}

	T pop(void)
	{
		std::unique_lock<std::mutex> lock(m);
		while(s.empty())
		{
			c.wait(lock);
		}
		T val = s.front();
		s.pop();
		return val;
	}

	T peek(void)
	{
		std::unique_lock<std::mutex> lock(m);
		while(s.empty())
		{
			c.wait(lock);
		}
		T val = s.front();
		return val;
	}

	void clear()
	{
		std::unique_lock<std::mutex> lock(m);
		std::stack<T> empty;
		std::swap(s, empty);
	}
};

template <class T>
class ThreadSafeVec
{
private:
	std::vector<T> v;
	std::mutex m;
	std::condition_variable c;
public:
	ThreadSafeVec(void)
		: v()
		, m()
		, c()
	{}

	~ThreadSafeVec(void)
	{}

	ThreadSafeVec( ThreadSafeVec const & vIn) 
	{
		v = vIn.v;
	}

	size_t size()
	{
		std::unique_lock<std::mutex> lock(m);
		return v.size();
	}

	void push_back(T t)
	{
		std::lock_guard<std::mutex> lock(m);
		v.push_back(t);
		c.notify_one();
	}

	T& operator[](int id)
	{
		std::unique_lock<std::mutex> lock(m);
		while(v.empty())
		{
			c.wait(lock);
		}		
		return v[id];
	}

	void clear()
	{
		std::unique_lock<std::mutex> lock(m);
		v.clear();
	}
};

template <class T>
class ThreadSafeMem
{
private:
	T* pMem;
	unsigned long sizeInBytes;
	std::timed_mutex m;
public:
	ThreadSafeMem(void)
		: pMem(NULL),
		sizeInBytes(0),
		m()
	{}

	~ThreadSafeMem(void)
	{
	}

	void ReleaseMem()
	{
		std::unique_lock<std::timed_mutex> lock(m);
		if(pMem)
		{
			std::free(pMem);
			pMem = NULL;
		}
		sizeInBytes = 0;
	}

	// must pair with UnlockMem
	void LockMem()
	{
		m.lock();
	}

	bool TryLockMem(int timeMS)
	{
		return m.try_lock_for(std::chrono::milliseconds(timeMS));
	}

	void UnlockMem()
	{
		m.unlock();
	}

	// must use Lock & Unlock functions
	long CopyMem(T* cMem, unsigned long cSizeInBytes)
	{
		if((0 == sizeInBytes) || (sizeInBytes < cSizeInBytes) || (NULL == cMem))
			return FALSE;

		memcpy_s(pMem, cSizeInBytes, cMem, cSizeInBytes);
		return TRUE;
	}

	// must use Lock & Unlock functions
	T* GetMem()
	{
		return pMem;
	}

	// allocate memory with size, release by ReleaseMem
	long SetMem(unsigned long assignSizeInBytes)
	{
		std::unique_lock<std::timed_mutex> lock(m);

		if(sizeInBytes == assignSizeInBytes)
			return TRUE;

		if(pMem)
		{
			std::free(pMem);
			pMem = NULL;
		}

		pMem = (T*)std::malloc(assignSizeInBytes);
		if((NULL == pMem))
		{
			sizeInBytes = 0;
			return FALSE;
		}
		else
		{
			std::memset(pMem, 0x0, assignSizeInBytes);
			sizeInBytes = assignSizeInBytes;
			return TRUE;
		}
	}

	unsigned long GetSizeInBytes()
	{
		std::unique_lock<std::timed_mutex> lock(m);
		return sizeInBytes;
	}
};

#endif