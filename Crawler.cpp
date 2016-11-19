// Crawler.cpp : Defines the entry point for the console application.
//

#include "StdAfx.h"
#include "QManager.h"
#include "HttpSocket.h"
#include "HtmlParser.h"
#include "argv.h"
#include "UrlParser.h"
#include "Sync.h"

#if FLATFORM & WIN32_FLATFORM
struct SocketInitializer
	{
	SocketInitializer()
		{
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		}

	~SocketInitializer() { WSACleanup(); }
	} sockInitializer;
#endif

LogStream& g_log = LogStream::GetObj(LogStream::STD_OUT);
int main(int argc, char* argv[])
	{
	assert(argc > 0);
	std::vector<HttpSocketThread*> sockArr;
	gimmesilver::Semaphore* httpSem = 0;
	gimmesilver::Semaphore* qSem = 0;
	gimmesilver::SyncQ* urlQ = 0;
	Cookie* cookie = 0;
	DNS* dns = 0;
	QManager* qManager = 0;
	try {
		int i = 0;
		CrawlerArgv arg;
		for (i = 1; i < argc; ++i)
			arg.Parse(argv[i]);

		g_log << "crawling start..." << std::endl;
		httpSem = new gimmesilver::Semaphore(arg.sockCnt_);
		qSem = new gimmesilver::Semaphore(arg.sockCnt_);
	//	urlQ = new gimmesilver::SyncQ(arg.qLimit_, arg.seed_, Priority::CreateObj(arg.policy_), arg.output_.directory() / "link.txt",
	//								  *httpSem);
		qManager = QManager::CreateObj(arg.policy_, arg.qLimit_, arg.seed_, arg.output_.directory() / "link.txt");
		urlQ = new gimmesilver::SyncQ(qManager, *httpSem);
		cookie = new Cookie;
		dns = new DNS;
		for (i = 0; i < arg.sockCnt_; ++i)
			{
			HttpSocketThread* sock = new HttpSocketThread(arg.output_, *httpSem, *qSem, *urlQ, arg.timeout_, *cookie, *dns);
			sock->Initiate();
			sockArr.push_back(sock);
			}

		httpSem->Release();
		while (true)
			{
			qSem->Wait();
			if (urlQ->Empty() || urlQ->Full())
				break;
			}
		}
	catch (Argv::Exception& e)
		{
		g_log << e.what() << std::endl;
		}
	catch (...)
		{
		g_log << "unknown exception!!!" << std::endl;
		}

	for (int i = 0; i < sockArr.size(); ++i)
		delete sockArr[i];

	if (urlQ->Full())
		g_log << "URL Queue is full! -> Q size is " << urlQ->Size() << std::endl;
	else if (urlQ->Empty())
		g_log << "URL Queue is empty!" << std::endl;

	delete dns;
	delete cookie;
	delete urlQ;
	delete qManager;
	delete qSem;
	delete httpSem;

	return 0;
	}

