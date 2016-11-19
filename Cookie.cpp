// Cookie.cpp: implementation of the Cookie class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Cookie.h"
#include <time.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

void Cookie::Insert(const std::string& cookie, const Url& url)
	{
	Parse(cookie, url);
	}

void Cookie::Insert(const std::vector<std::string>& cookieArr, const Url& url)
	{
	std::vector<std::string>::const_iterator b = cookieArr.begin(), e = cookieArr.end();
	for (; b != e; ++b)
		Insert(*b, url);
	}

void Cookie::Parse(std::string cookie, const Url& url)
	{
	if (cookie.empty())
		return;

	int a = cookie.find(';');
	int b = cookie.find('=');
	if (b > a || b == std::string::npos)
		return;

	_st_cookie	cookieInfo;
	std::string name(cookie.begin(), cookie.begin()+b);
	cookieInfo.value_	= SearchCookieValue(cookie, name);
	cookieInfo.expires_	= SearchCookieValue(cookie, "expires");
	cookieInfo.path_	= SearchCookieValue(cookie, "path");
	cookieInfo.domain_	= SearchCookieValue(cookie, "domain");
	cookieInfo.secure_	= SearchCookieValue(cookie, "secure");

	if (cookieInfo.path_.empty())
		{
		std::string path(url.GetPath());
		if (path.find_last_of('.') != std::string::npos)
			{
			std::string::size_type end_path_pos = path.find_last_of('/');
			path.erase(end_path_pos);
			}

		cookieInfo.path_ = path;
		}

	if (cookieInfo.domain_.empty())
		cookieInfo.domain_ = url.GetNetLoc();

	SYNCHRONIZED_THIS_BLOCK(gimmesilver::AutoLock(sync_))
		{
		if (cookie_.find(name) == cookie_.end())
			{
			cookie_.insert(std::make_pair(name, cookieInfo));
			}
		else
			{
			std::pair<CookieContainer::iterator, CookieContainer::iterator> g = cookie_.equal_range(name);
			CookieContainer::iterator p = g.first;
			for (; g.first != g.second; ++g.first)
				{
				if (g.first->second.path_ == cookieInfo.path_ 
					&& Url::CompareNetloc(p->second.domain_, cookieInfo.domain_))
					{
					g.first->second.value_ = cookieInfo.value_;
					g.first->second.expires_ = cookieInfo.expires_;
					g.first->second.secure_ = cookieInfo.secure_;
					break;
					}
				}

			if (g.first == g.second)
				cookie_.insert(std::make_pair(name, cookieInfo));
			}
		}
	}

std::string Cookie::SearchCookieValue(const std::string& cookie, const std::string& delimeter)
	{
	std::string::size_type	beg_pos = 0;
	std::string::size_type	end_pos = 0;

	if (std::string::npos != (beg_pos = cookie.find(delimeter.c_str())))
		{
		if (std::string::npos != (beg_pos = cookie.find('=', beg_pos+delimeter.size())))
			{
			if (std::string::npos != (end_pos = cookie.find(';', ++beg_pos)))
				return std::string(cookie.begin()+beg_pos, cookie.begin()+end_pos);
			else
				return std::string(cookie.begin()+beg_pos, cookie.end());
			}
		}

	return "";
	}

std::string Cookie::GetCookie(const Url& url)
	{
	gimmesilver::AutoLock lock(sync_);
	std::string	result;
	if (url.empty())
		return result;

	CookieContainer::iterator b = cookie_.begin(), e = cookie_.end();
	for (; b != e;)
		{
		std::string temp = b->second.domain_, temp2 = b->second.path_;
		if ((Url::CompareNetloc(url.GetNetLoc(), b->second.domain_) 
				|| url.GetNetLoc().find(b->second.domain_) != std::string::npos)
			&& url.GetPath().find(b->second.path_) != std::string::npos)
			{
			if (b->second.expires_.empty() || !IsExpired(b->second.expires_))
				result += b->first + "=" + b->second.value_ + "; ";
			else
				{
				cookie_.erase(b++);
				continue;
				}
			}
		++b;
		}

	if (!result.empty())
		result.erase(result.find_last_of(";"));

	return result;
	}

bool Cookie::IsExpired(const std::string& date)
	{
	if (date.empty())
		return false;

	time_t expire = Date2Time(date);
	time_t cur = time(0);

	return expire < cur;
	}

time_t Cookie::Date2Time(const std::string& date)
	{
	// Monday, 01-Jan-2035 00:00:00 GMT
	static char* month[] = {
		"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
	};
	const int KOREA_TIME_ZONE = 9*3600;

	tm tm_time;
	memset(&tm_time, 0, sizeof(tm_time));
	std::string::const_iterator b = date.begin(), e = date.end();
	while (b < e && !isdigit(*b))
		++b;

	if (std::distance(b, e) < 20)
		return time(0);

	tm_time.tm_mday = atoi(&*b);
	b += 3;
	int i = 0;
	for (; i < COUNT_OF(month); ++i)
		{
		if (StringCompare(month[i], &*b, strlen(month[i])) == 0)
			break;
		}

	b += 4;
	if (i == COUNT_OF(month) || b >= e)
		return time(0);

	tm_time.tm_mon = i;
	tm_time.tm_year = atoi(&*b) - 1900;
	b += 5;
	tm_time.tm_hour = atoi(&*b);
	b += 3;
	tm_time.tm_min = atoi(&*b);
	b += 3;
	tm_time.tm_sec = atoi(&*b);

	return mktime(&tm_time) + KOREA_TIME_ZONE;
	}

