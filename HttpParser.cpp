#include "StdAfx.h"
#include "HttpParser.h"

void HttpParser::Parse(const char* data, size_t size)
	{
	switch (state_)
		{
		case INIT: Clear();
		case HEADER_PARTIAL: ParseHeader(data, data+size); break;
		case BODY_PARTIAL:	 ParseBody(data, data+size); break;
		}
	}

void HttpParser::ParseHeader(const char* b, const char* e)
	{
	const char* cur = b;
	const char* prev = b;
	bool hdrPartial = true;

//	dump_.assign(b, e);
	while (hdrPartial && cur != e)
		{
		if (cur != b && *(cur-1) == '\r' && *cur == '\n')
			{
			if (!buf_.empty())
				{
				buf_.append(prev, cur);
				const char* temp = buf_.c_str();
				if (method_ == NONE)
					method_ = ParseMethod(temp, temp+buf_.size());
				else
					hdrPartial = ParseField(temp, temp+buf_.size());
				buf_ = "";
				}
			else
				{
				if (method_ == NONE)
					method_ = ParseMethod(prev, cur-1);
				else
					hdrPartial = ParseField(prev, cur-1);
				}
			prev = cur+1;
			}
		++cur;
		}

	if (hdrPartial)
		{
		state_ = HEADER_PARTIAL;
		buf_.append(prev, e);
		}
	else
		{
		state_ = BODY_PARTIAL;
		if (contentsLen_ > 0)
			body_.reserve(contentsLen_);

		ParseBody(prev, e);
		}
	}

HttpParser::Method HttpParser::ParseMethod(const char* b, const char* e)
	{
	static struct _MethodList {
	HttpParser::Method code_;
	const char* str_;
	} methodList[] = {
		{ GET, "GET" },
		{ POST, "POST" },
		{ RESPONSE, "HTTP" }
		};

	const char* temp = b;
	while (!isspace(*temp) && temp != e)
		++temp;

	std::string method;
	std::transform(b, temp, std::inserter(method, method.end()), std::ptr_fun(toupper));
	for (int i = 0; i < COUNT_OF(methodList); ++i)
		{
		if (method.find(methodList[i].str_) != std::string::npos)
			{
			assert(method.find(methodList[i].str_) == 0);
			if (methodList[i].code_ == RESPONSE)
				{
				while (isspace(*temp) && temp != e)
					++temp;

				if (temp + 3 < e && isdigit(temp[0]) && isdigit(temp[1]) && isdigit(temp[2]) 
					&& isspace(temp[3]))
					repCode_ = atoi(temp);
				}
			return methodList[i].code_;
			}
		}

	return INVALID;
	}

inline void GetFieldData(const char *s, const char *e, std::string& data, int n)
	{
	data = "";
	s += n;
	while (s < e && (*s == ':' || *s == ' '))
		++s;

	while (s < e && *s != '\r' && *s != '\n')
		{
		data += *s;
		++s;
		}
	}

/*
ParseField() : header의 끝이면 return false
*/
bool HttpParser::ParseField(const char* b, const char* e)
	{
	static const std::string conLen = "content-length";
	static const std::string host = "host";
	static const std::string location = "location";
	static const std::string cookie = "set-cookie";
	static const std::string transEnc = "transfer-encoding";
	static const std::string chunk = "chunked";

	std::string temp;
	switch (*b)
		{
		case 'c':
		case 'C':
			if (b+conLen.size() < e && StringCompare(b, conLen.c_str(), conLen.size()) == 0)
				{
				GetFieldData(b, e, temp, conLen.size());
				contentsLen_ = atoi(temp.c_str());
				}
			break;

		case 'h':
		case 'H':
			if (b+host.size() < e && StringCompare(b, host.c_str(), host.size()) == 0)
				GetFieldData(b, e, host_, host.size());
			break;

		case 'l':
		case 'L':
			if (b+location.size() < e && StringCompare(b, location.c_str(), location.size()) == 0)
				GetFieldData(b, e, location_, location.size());
			break;

		case 's':
		case 'S':
			if (b+cookie.size() < e && StringCompare(b, cookie.c_str(), cookie.size()) == 0)
				{
				GetFieldData(b, e, temp, cookie.size());
				cookie_.push_back(temp);
				}
			break;
		
		case 't':
		case 'T':
			if (b+transEnc.size() < e && StringCompare(b, transEnc.c_str(), transEnc.size()) == 0)
				{
				GetFieldData(b, e, temp, transEnc.size());
				if (StringCompare(temp.c_str(), chunk.c_str(), chunk.size()) == 0)
					isChunk_ = true;
				}
			break;

		case '\r': return false;
		}

	return true;
	}

void HttpParser::ParseBody(const char* b, const char* e)
	{
	if (isChunk_)
		ParseChunk(b, e);
	else
		{
		body_.append(b, e); 
		if (body_.size() >= contentsLen_)
			state_ = INIT;
		}
	}

void HttpParser::ParseChunk(const char* b, const char* e)
	{
	while (b < e)
		{
		if (chunkState_ == CHUNK_INIT)
			{
			b = GetChunkSize(b, e, chunkSize_);
			if (chunkSize_ == -1)
				{
				buf_.append(b, e);
				chunkState_ = CHUNK_PARTIAL;
				break;
				}
			else if (chunkSize_ == 0)
				{
				state_ = INIT;
				break;
				}
			else
				{
				b = AppendBody(b, e);
				}
			}
		else if (chunkState_ == CHUNK_PARTIAL)
			{
			if (std::distance(b, e) < 2 && chunkSize_ < 0)
				buf_.append(b++, e);
			else
				{
				if (!buf_.empty())
					{
					const char* prev = b;
					while (b < e && *b != '\r' && *(b+1) != '\n')
						++b;
					if (b < e)
						{
						b += 2;
						buf_.append(prev, b);
						const char* temp = buf_.c_str();
						GetChunkSize(temp, temp+buf_.size(), chunkSize_);
						buf_ = "";
						if (chunkSize_ > 0)
							b = AppendBody(b, e);
						else if (chunkSize_ == 0)
							{
							state_ = INIT;
							break;
							}
						else
							assert(false);
						}
					else
						buf_.append(prev, b);
					}
				else
					{
					if (chunkSize_ < 0)
						b = GetChunkSize(b, e, chunkSize_);
					b = AppendBody(b, e);
					}
				}
			}
		}
	}

const char* HttpParser::AppendBody(const char* b, const char* e)
	{
	if (chunkSize_ < 0)
		{
		buf_.append(b, e);
		return e;
		}

	int dataSize = std::distance(b, e);
	if (dataSize > chunkSize_)
		{
		body_.append(b, b+chunkSize_);
		b += chunkSize_;
		chunkState_ = CHUNK_INIT;
		}
	else
		{
		body_.append(b, e);
		chunkState_ = CHUNK_PARTIAL;
		chunkSize_ -= dataSize;
		b = e;
		}
	return b;
	}

const char* HttpParser::GetChunkSize(const char* b, const char* e, int& len)
	{
	while (b < e && !isxdigit(*b))
		++b;

	const char* prev = b;
	while (b < e && isxdigit(*b))
		++b;

	len = gimmesilver::Hexa2Int(std::string(prev, b));

	/// remove chunk-extension
	while (b < e && *(b-1) != '\r' && *b != '\n')
		++b;

	if (b >= e)
		{
		len = -1;
		b = prev;
		}
	else
		++b;

	return b;
	}

void HttpParser::Clear()
	{
	state_ = INIT;
	chunkState_ = CHUNK_INIT;
	repCode_ = contentsLen_ = chunkSize_ = 0;
	method_ = NONE;

	host_ = "";
	location_ = "";
	body_ = "";
	buf_ = "";

	cookie_.clear();
//	dump_ = "";

	isChunk_ = false;
	}

