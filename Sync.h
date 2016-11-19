#ifndef ___SYNC_H___
#define ___SYNC_H___

#include "QManager.h"
namespace gimmesilver {

struct SyncException : public std::exception
	{
	SyncException(const char* msg) : msg_(msg) {}
	~SyncException() throw() {}
	const char* what() { return msg_.c_str(); }
	std::string msg_;
	};

class SyncObj
	{
#if FLATFORM & WIN32_FLATFORM
	public:
		SyncObj() { ::InitializeCriticalSection(&sync_); }
		~SyncObj() { ::DeleteCriticalSection(&sync_); }

		void Lock() { ::EnterCriticalSection(&sync_); }
		void Unlock() { ::LeaveCriticalSection(&sync_); }

	private:
		CRITICAL_SECTION sync_;
#else
	public:
		SyncObj() { pthread_mutex_init(&sync_, 0); }
		~SyncObj() { pthread_mutex_destroy(&sync_); }

		void Lock() { pthread_mutex_lock(&sync_); }
		void Unlock() { pthread_mutex_unlock(&sync_); }

	private:
		pthread_mutex_t sync_;
#endif
	};

/// refer to http://ricanet.com/new/view.php?id=blog/050807
class AutoLock
	{
	public:
		AutoLock(SyncObj& sync) : sync_(sync)
			{ sync_.Lock(); }

		~AutoLock() { sync_.Unlock(); }
		operator bool() { return false; }

	private:
		SyncObj&  sync_;
	};

#define SYNCHRONIZED_THIS_BLOCK(A) if (gimmesilver::AutoLock _syncVar = A) assert(0); else
///...

class Semaphore
	{
	public:
		Semaphore(int limit);
		~Semaphore();

		int Wait();
		bool Release();

	private:
#if FLATFORM & WIN32_FLATFORM
		HANDLE sem_;
#else
		sem_t sem_;
		int limit_;
		SyncObj sync_;
#endif
	};

class SyncQ// : public QManager
	{
	public:
		SyncQ(QManager* q, Semaphore& httpSem);

		Url GetNextUrl();
		void Insert(const Url& baseUrl, const std::vector<std::string>& urlList);
		int Size();
		bool Empty();
		bool Full();

	private:
		Semaphore& httpSem_;
		SyncObj sync_;
		QManager* urlQ_;
	};

}
#endif
