#include "StdAfx.h"
#include "QManager.h"
#include "argv.h"

QManager* QManager::CreateObj(int type, int limit, const Url& seed, const gimmesilver::path& file)
	{
	switch (type)
		{
		case CrawlerArgv::BFS: return new BfsQ(limit, seed, file);
		}

	assert(false);
	return 0;
	}

QManager::~QManager()
	{
	SaveLinkList(linkOutput_);
	}

void QManager::Insert(const Url& baseUrl, const std::vector<std::string>& urlList)
	{
	std::vector<std::string>::const_iterator b = urlList.begin(), e = urlList.end();
	for (; b != e; ++b)
		{
		Url url = Url::GetAbsPath(baseUrl, *b);
		if (!Filter(url))
			continue;

		Url temp = url.string(Url::SCHEME | Url::NETLOC | Url::PATH);
		if (urlHistory_.find(temp) == urlHistory_.end())
			{
			Insert(temp.string());
			urlHistory_.insert(temp);
			}
		}
	}

bool QManager::Filter(Url& url)
	{
	if (url.GetScheme() != "http")
		return false;

	const std::string& path = url.GetPath();
	std::string::size_type pos = path.rfind('.');
	if (pos != std::string::npos)
		return StringCompare(path.substr(pos).c_str(), "html", strlen("html")) == 0
				|| StringCompare(path.substr(pos).c_str(), "htm", strlen("htm")) == 0;

	return true;
	}

void QManager::SaveLinkList(const gimmesilver::path& output)
	{
	std::ofstream file(output.c_str());
	std::set<Url>::iterator b = urlHistory_.begin(), e = urlHistory_.end();
	for (; b != e; ++b)
		file << b->string() << std::endl;
	}

/// BFS Queue
const Url& BfsQ::Front() const
	{
	if (urlQ_.empty())
		throw QException();
	return urlQ_.front();
	}

void BfsQ::Insert(const std::string& url)
	{
	urlQ_.push_back(url);
	}
