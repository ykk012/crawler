#include "StdAfx.h"
#include "HttpSocket.h"

bool DNS::GetSockAddr(const std::string& host, sockaddr_in& addr)
	{
	gimmesilver::AutoLock lock(sync_);
	CacheType::iterator pos = cache_.find(host);
	if (pos == cache_.end())
		{
		hostent* hp = gethostbyname(host.c_str());
		if (hp && hp->h_addr_list[0])
			{
			memcpy(&addr.sin_addr, hp->h_addr_list[0], hp->h_length);
			cache_.insert(std::make_pair(host, addr.sin_addr));			
			}
		else
			return false;
		}
	else
		memcpy(&addr.sin_addr, &pos->second, sizeof(addr.sin_addr));

	return true;
	}

bool HttpSocket::CheckRedir(const std::string& url)
	{
	bool ret = redirHistory_.find(url) == redirHistory_.end();
	if (ret)
		redirHistory_.insert(url);
	else
		err_ = "redirection looping!!!";

	return ret;
	}

bool HttpSocket::GetSockAddr(const Url& url, sockaddr_in& addr)
	{
	assert(err_.empty());	
	if (dns_.GetSockAddr(url.GetNetLoc(), addr))
		{
		addr.sin_port = htons(url.GetPort());
		addr.sin_family = AF_INET;
		}
	else
		err_ = "fail to query DNS server";

	return err_.empty();
	}

bool HttpSocket::Request(Url url)
	{
	if (url.empty())
		return false;

	bool ret = false;
	bool done = false;
	err_ = "";
	redirHistory_.clear();
	while (!done && err_.empty())
		{
		if (!CheckRedir(url.string()))
			break;

		sockaddr_in addr;
		if (GetSockAddr(url, addr))
			{
			int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (sock != SOCK_ERROR)
				{
				if (connect(sock, (sockaddr*)&addr, sizeof(addr)) != SOCK_ERROR)
					{
					std::string sendData;
					MakeHttpHeader(sendData, url.string(Url::PATH|Url::PARAM|Url::QUERY), url.GetNetLoc(), cookie_.GetCookie(url));
					send(sock, sendData.c_str(), sendData.size(), 0);
					ret = RecvData(sock);
					cookie_.Insert(parser_.GetCookie(), url);
					}
				else
					err_ = "fail to connect";

				CloseSocket(sock);
				}
			else
				err_ = "fail to create socket";
			}

		if (parser_.IsRedirect() && !parser_.GetLocation().empty())
			url.Update(parser_.GetLocation());
		else
			done = true;
		}

	return ret;
	}

void HttpSocket::MakeHttpHeader(std::string& hdr, const std::string& url, const std::string& host, const std::string& cookie)
	{
	hdr = "GET ";
	if (!url.empty())
		hdr += url;
	else
		hdr += "/";
	hdr += " HTTP/1.1\r\n";
	hdr += "Host: " + host + "\r\n";
	hdr += "User-Agent: TestCrawler\r\n";
	hdr += "Accept: */*\r\n";
	hdr += "Accept-Language: ko\r\n";
	hdr += "Cookie: " + cookie + "\r\n";
	hdr += "\r\n";
	}

int HttpSocket::IoctlSocket(int sock, int flag)
	{
#if FLATFORM & WIN32_FLATFORM
	unsigned long recvSize = 0;
	if (SOCK_SUCCESS == ioctlsocket(sock, flag, &recvSize))
		return recvSize;
#else
	int recvSize = 0;
	if (SOCK_SUCCESS == ioctl(sock, FIONREAD, &recvSize))
		return recvSize;
#endif
	return -1;
	}

void HttpSocket::CloseSocket(int sock)
	{
#if FLATFORM & WIN32_FLATFORM
	closesocket(sock);
#else
	close(sock);
#endif
	}

bool HttpSocket::RecvData(int sock)
	{
	fd_set readFd;
	std::vector<char> recvData;
	int ret = 0;
	int recvSize = 0;
	bool done = false;
	timeval timeout = timeout_;
	parser_.Clear();

	while (!done)
		{
		FD_ZERO(&readFd);
		FD_SET(sock, &readFd);
		ret = select(FD_SETSIZE, &readFd, 0, 0, &timeout);
		switch (ret)
			{
			case SOCK_ERROR: err_ = "socket error!!!"; return false;
			case TIME_OUT: err_ = "time out!!!"; return false;
			default:
				if (FD_ISSET(sock, &readFd))
					{
					recvSize = IoctlSocket(sock, FIONREAD);
					if (recvSize > 0)
						{
						recvData.resize(recvSize);
						recvSize = recv(sock, &recvData[0], recvSize, 0);
						if (recvSize <= 0)
							{
							err_ = "connection closed!!!";
							return false;
							}
						parser_.Parse(&recvData[0], recvSize);
						done = !parser_.IsPartial();
						recvData.clear();
						}
					else
						done = true;
					}
				break;
			}
		}

	if (!parser_.IsOK() && !parser_.IsRedirect())
		err_ = "HTTP Error(Response code:" + gimmesilver::ToString(parser_.GetRepCode()) + ")";

	return parser_.IsOK();
	}

/// HttpSocketThread
int HttpSocketThread::htmlCnt_ = 0;
gimmesilver::SyncObj HttpSocketThread::sync_;

HttpSocketThread::~HttpSocketThread()
	{
	/// semaphore full release...
	while (sem_.Release())
		{
		;
		}
	}

void HttpSocketThread::Release()
	{
	gimmesilver::AutoLock lock(sync_);
	int qSize = urlQ_.Size();
	for (int i = 0; i < qSize; ++i)
		{
		if (!sem_.Release())
			break;
		}

	alarm_.Release();
	}

int HttpSocketThread::ThreadFunction()
	{
	try {
		HttpSocket httpSock_(timeout_, dns_, cookie_);
		HtmlParser htmlParser_;
		while (continue_)
			{
			sem_.Wait();
			if (urlQ_.Full())
				break;

			Url url = urlQ_.GetNextUrl();
			if (httpSock_.Request(url))
				{
				SYNCHRONIZED_THIS_BLOCK(gimmesilver::AutoLock(sync_))
					{
					if (urlQ_.Full())
						break;
				//	urlQ_.Insert(htmlCnt_, url);
					output_ = output_.directory() / (gimmesilver::ToString(htmlCnt_++) + ".html");
					std::ofstream file(output_.c_str());
					file << httpSock_.GetBody();
					g_log << url.string() << " --> " << output_.string() << std::endl;
					htmlParser_.Parse(httpSock_.GetBody().c_str(), httpSock_.GetBody().size());
					urlQ_.Insert(url, htmlParser_.ExtractLinkUrlList());
					}
				}
			else
				{
				SYNCHRONIZED_THIS_BLOCK(gimmesilver::AutoLock(sync_))
					{
					g_log << url.string() << " --> " << httpSock_.GetErrMsg() << std::endl;
					}
				}

			Release();
			}
		}
	catch (QManager::QException&)
		{
		}

	return 0;
	}
