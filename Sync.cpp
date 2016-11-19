#include "StdAfx.h"
#include "Sync.h"

namespace gimmesilver {

Semaphore::Semaphore(int limit)
	{
#if FLATFORM & WIN32_FLATFORM
	sem_ = ::CreateSemaphore(0, 0, limit, 0);
	if (sem_ == 0)
		throw SyncException("fail to create semaphore");
#else
	limit_ = limit;
	sem_init(&sem_, 0, 0);
#endif
	}

Semaphore::~Semaphore()
	{
#if FLATFORM & WIN32_FLATFORM
	CloseHandle(sem_);
#else
	sem_destroy(&sem_);
#endif
	}

int Semaphore::Wait()
	{
#if FLATFORM & WIN32_FLATFORM
	return ::WaitForSingleObject(sem_, INFINITE);
#else
	sem_wait(&sem_);
	AutoLock lock(sync_);
	return ++limit_;
#endif
	}

bool Semaphore::Release()
	{
#if FLATFORM & WIN32_FLATFORM
	return FALSE != ::ReleaseSemaphore(sem_, 1, 0);
#else
	AutoLock lock(sync_);
	if (--limit_)
		return false;
	return -1 != sem_post(&sem_);
#endif
	}

SyncQ::SyncQ(QManager* q, Semaphore& httpSem) 
	: httpSem_(httpSem), urlQ_(q)
	{
	}

Url SyncQ::GetNextUrl()
	{
	AutoLock lock(sync_);
	Url url = urlQ_->Front();
	urlQ_->PopFront();
	return url;
	}

void SyncQ::Insert(const Url& baseUrl, const std::vector<std::string>& urlList)
	{
	AutoLock lock(sync_);
	urlQ_->Insert(baseUrl, urlList);
	}

int SyncQ::Size()
	{
	AutoLock lock(sync_);
	return urlQ_->Size();
	}

bool SyncQ::Empty()
	{
	AutoLock lock(sync_);
	return urlQ_->Empty();
	}

bool SyncQ::Full()
	{
	AutoLock lock(sync_);
	return urlQ_->Full();
	}

}
