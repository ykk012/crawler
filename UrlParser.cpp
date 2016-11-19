#include "StdAfx.h"
#include "UrlParser.h"

Url::Url(const std::string& url)
	{
	Parse(url, url_);
	}

Url::Url(const Url& other)
	{
	url_ = other.url_;
	}

void Url::Update(const Url& newUrl)
	{
	*this = GetAbsPath(url_.GetUrl(), newUrl.url_.GetUrl());
	}

const Url& Url::operator=(const Url& others)
	{
	if (this != &others)
		{
		Url temp(others);
		swap(temp);
		}

	return *this;
	}

void Url::swap(Url& other)
	{
	url_.swap(other.url_);
	}

bool Url::operator==(const Url& other) const
	{
/*	if (url_.scheme_ != other.url_.scheme_)
		return false;

	if (!CompareNetloc(url_.net_loc_, other.url_.net_loc_))
		return false;

	if (StringCompare(url_.path_.c_str(), other.url_.path_.c_str(), url_.path_.size()) != 0)
		{
		/// to do -> encoding 처리...
		return false;
		}

	if (StringCompare(url_.param_.c_str(), other.url_.param_.c_str(), url_.param_.size()) != 0)
		return false;

	if (StringCompare(url_.query_.c_str(), other.url_.query_.c_str(), url_.query_.size()) != 0)
		return false;

	if (StringCompare(url_.frag_.c_str(), other.url_.frag_.c_str(), url_.frag_.size()) != 0)
		return false;

	return true;
*/
	return string() == other.string();
	}

bool Url::operator<(const Url& other) const
	{
	return string() < other.string();
	}

bool Url::CompareNetloc(const std::string& l_netloc, const std::string& r_netloc)
	{
	if (l_netloc != r_netloc)
		{
		if (("www." + l_netloc) != r_netloc
			&& l_netloc != "www." + r_netloc)
		return false;
		}
	return true;
	}

/*
<scheme>://<net_loc>/<path>;<param>?<query>#<fragment>
*/
bool Url::Parse(std::string url, _st_url& urlComposer)
	{
	assert(!url.empty());
	static struct _ParsingTail
		{
		void operator()(std::string& url, std::string& element, char c)
			{
			std::string::size_type pos = std::string::npos;
			pos = url.find(c);
			if (pos != std::string::npos)
				{
				element.assign(url, pos+1, url.size());
				url.erase(pos);
				}
			}
		} ParsingTail;

	std::string::size_type pos = std::string::npos;
	ParsingTail(url, urlComposer.frag_, '#');
	ParsingTail(url, urlComposer.query_, '?');
	ParsingTail(url, urlComposer.param_, ';');

	pos = url.find(':');
	if (pos != std::string::npos)
		{
		urlComposer.scheme_.assign(url, 0, pos);
		std::transform(urlComposer.scheme_.begin(), urlComposer.scheme_.end(),
					   urlComposer.scheme_.begin(),
					   std::ptr_fun(tolower));
		url.erase(0, pos+1);
		}

	if (url.find("//") == 0)
		{
		url.erase(0, 2);
		pos = url.find('/');
		if (pos == std::string::npos)
			pos = url.size();
		urlComposer.net_loc_.assign(url, 0, pos);
		url.erase(0, pos);	/// '/'를 제거하지 않음!
		pos = urlComposer.net_loc_.find(':');
		if (pos != std::string::npos)
			{
			urlComposer.port_ = atoi(urlComposer.net_loc_.substr(pos+1).c_str());
			urlComposer.net_loc_.erase(pos);
			}
		}

	urlComposer.path_ = url;
	if (!urlComposer.net_loc_.empty())
		{
		std::transform(urlComposer.net_loc_.begin(), urlComposer.net_loc_.end(), 
					   urlComposer.net_loc_.begin(), 
					   std::ptr_fun(tolower));
		if (urlComposer.path_.empty())
			urlComposer.path_ = '/';
		}

//	urlComposer.path_ = EncodeUrl(urlComposer.path_);

	return true;
	}
/*
std::string Url::EncodeUrl(const std::string& path)
	{
	std::string result;
	std::string::const_iterator b = path.begin(), e = path.end();
	for (; b != e; ++b)
		{
		if (isalpha(*b) || isxdigit(*b) || *b == '/' || *b == '%')
			result += *b;
		else
			{
			static char buf[10];
			result += '%';
			result += itoa((unsigned char)*b, buf, 16);
			}
		}

	return result;
	}
*/
/*
상대 path에서 절대 path를 얻는 알고리즘
 1) 기본 url이 없다면 현재 url이 절대 path -> end
 2) 현재 url이 scheme부터 시작된다면 이것이 절대 path( -> end), 없다면 기본 url에서 상속
 3) 현재 url이 net_loc이 있다면 7단계로, net_loc이 없다면 기본 url에 포함된 net_loc을 상속
 4) 현재 url이 '/'로 시작한다면 7단계로,
 5) 1. 현재 url이 있으면 6단계로, 비어있으면 기본 url을 상속
	2. 만약 params가 있다면 7단계로, 없다면 기본 url의 params를 상속
	3. 만약 query info가 있다면 7단계로, 없으면 기본 url의 query를 상속
 6) 기본 url의 마지막 segment(마지막 '/'부터 그 이후 스트링들)을 제거하고 현재 url을 뒤에 덧붙힌다. 최종 url에 아래와 같은 작업을 수행한다.
	1. "./"은 제거
	2. "."이 마지막 segment라면 제거
	3. "<segment>/../"이 있으면 제거
	4. "<segment>/.."이 있으면 제거
 7) 최종 결과 url
*/
Url Url::GetAbsPath(const Url& baseUrl, const Url& curUrl)
	{
	// 1)
	if (baseUrl.empty())
		return curUrl;

	_st_url baseComposer = baseUrl.url_, curComposer = curUrl.url_;
	assert(!baseComposer.scheme_.empty() && !baseComposer.net_loc_.empty());

	// 2)
	if (!curComposer.scheme_.empty())
		return curUrl;
	curComposer.scheme_ = baseComposer.scheme_;

	// 3)
	if (curComposer.net_loc_.empty())
		{
		curComposer.net_loc_ = baseComposer.net_loc_;

		// 4)
		if (curComposer.path_.empty() || curComposer.path_[0] != '/')
			{
			if (curComposer.path_.empty())	// 5)
				{
				curComposer.path_ = baseComposer.path_;
				if (curComposer.param_.empty())
					{
					curComposer.param_ = baseComposer.param_;
					if (curComposer.query_.empty())
						curComposer.query_ = baseComposer.query_;
					}
				}
			else // 6)
				{
				gimmesilver::Splitter baseSplit(baseComposer.path_, '/'), 
									  curSplit(curComposer.path_, '/');
				baseSplit.pop_back();
				baseSplit += curSplit;
				baseSplit.RemoveAll(".");	// 6)-1,2
				gimmesilver::Splitter::SplitType::iterator pos = baseSplit.begin();
				while ((pos = std::find(pos, baseSplit.end(), "..")) 
						!= baseSplit.end())
					{
					if (pos != baseSplit.begin())
						pos = baseSplit.erase(baseSplit.erase(--pos));	// 6)-3,4
					else
						pos = baseSplit.erase(pos);
					}
				curComposer.path_ = baseSplit.string();
				if (curComposer.path_.empty() || curComposer.path_[0] != '/')
					curComposer.path_.insert(curComposer.path_.begin(), '/');
				}
			}
		}

	// 7)
	return curComposer.GetUrl();
	}

