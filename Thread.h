#ifndef ___THREAD_H___
#define ___THREAD_H___

namespace gimmesilver {

class Thread
	{
	public:
		enum { MAX_TIME = 5 };
		Thread();
		virtual ~Thread() { Terminate(MAX_TIME); }
		bool Initiate();
		void Terminate(unsigned long timeOut);
		bool IsRunning() const { return continue_ && th_ != 0; }

	protected:
		static void* _threadfunction(void* arg);
		virtual int ThreadFunction() = 0;

#if FLATFORM & WIN32_FLATFORM
		HANDLE th_;
#else
		pthread_t th_;
#endif
		bool continue_;
	};

}
#endif

