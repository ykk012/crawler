#ifndef ___HTTPSOCKET_H___
#define ___HTTPSOCKET_H___

#include "HttpParser.h"
#include "HtmlParser.h"
#include "UrlParser.h"
#include "Cookie.h"
#include "Thread.h"
#include "Sync.h"

class DNS
	{
	public:
		bool GetSockAddr(const std::string& host, sockaddr_in& addr);
	private:
		typedef std::map<std::string,in_addr> CacheType;
		CacheType cache_;
		gimmesilver::SyncObj sync_;
	};

class HttpSocket
	{
	public:
		HttpSocket(int timeout, DNS& dns, Cookie& cookie) : dns_(dns), cookie_(cookie) 
			{ timeout_.tv_sec = timeout; timeout_.tv_usec = 0; }
		bool Request(Url url);
		const std::string& GetBody() const { return parser_.GetBody(); }
		const std::string& GetErrMsg() const { return err_; }

		enum { SOCK_SUCCESS = 0, SOCK_ERROR = -1, TIME_OUT = 0 };

	private:
		int IoctlSocket(int sock, int flag);		
		bool RecvData(int sock);
		void MakeHttpHeader(std::string& hdr, const std::string& url, const std::string& host, const std::string& cookie);
		bool CheckRedir(const std::string& url);
		bool GetSockAddr(const Url& url, sockaddr_in& addr);
		void CloseSocket(int sock);

		DNS& dns_;
		Cookie& cookie_;
		HttpParser parser_;
		timeval timeout_;

		std::string err_;
		std::set<std::string> redirHistory_;
	};

class HttpSocketThread : public gimmesilver::Thread
	{
	public:
		HttpSocketThread(gimmesilver::path& output, gimmesilver::Semaphore& sem, gimmesilver::Semaphore& alarm,
						 gimmesilver::SyncQ& urlQ, int timeout, Cookie& cookie, DNS& dns) 
			: output_(output), sem_(sem), alarm_(alarm), urlQ_(urlQ), timeout_(timeout), cookie_(cookie), dns_(dns) {}
		~HttpSocketThread();

	protected:
		int ThreadFunction();

	private:
		void Release();

		static gimmesilver::SyncObj sync_;
		static int htmlCnt_;

		gimmesilver::path output_;
		gimmesilver::Semaphore& sem_;
		gimmesilver::Semaphore& alarm_;
		gimmesilver::SyncQ& urlQ_;
		Cookie& cookie_;
		DNS& dns_;
		int timeout_;
	};

#endif
