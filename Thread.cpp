#include "StdAfx.h"
#include "Thread.h"

namespace gimmesilver {

Thread::Thread() : continue_(false)
	{
#if FLATFORM & WIN32_FLATFORM
	th_ = 0;
#endif
	}

bool Thread::Initiate()
	{
	continue_ = true;
#if FLATFORM & WIN32_FLATFORM
	unsigned long id;
	th_ = ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)_threadfunction, this, 0, &id);
	if (th_ == INVALID_HANDLE_VALUE)
		return false;
#else
	if (0 == pthread_create(&th_, 0, _threadfunction, this))
		return false;
	pthread_detach(th_);
#endif
	return true;
	}

void Thread::Terminate(unsigned long timeOut)
	{
	if (continue_ && th_ != 0)
		{
		continue_ = false;
#if FLATFORM & WIN32_FLATFORM
		Sleep(1000);//Sleep(timeOut*1000);
		::TerminateThread(th_, 0);
#else
		sleep(1);
		pthread_cancel(th_);
#endif
		th_ = 0;
		}
	}

void* Thread::_threadfunction(void* arg)
	{
	Thread* This = (Thread*)arg;
	This->ThreadFunction();
	This->continue_ = false;
	This->th_ = 0;
#if FLATFORM & LINUX_FLATFORM
	pthread_exit(0);
#endif
	return 0;
	}

}
