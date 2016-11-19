#ifndef ___ARGV_H___
#define ___ARGV_H___

#include "UrlParser.h"
#include "gimmesilver.h"

struct Argv
	{
	struct Exception
		{
		Exception(const std::string& err) : err_(err) {}
		const std::string& what() const { return err_; }
		std::string err_;
		};

	virtual bool Parse(std::string arg) = 0;

	std::string::size_type ParseOpt(const std::string& arg, int& opt, const gimmesilver::TypeString* typeStrArr, int arrCnt)
		{
		std::string::const_iterator b = arg.begin(), e = arg.end();
		if (*b != '-')
			throw Exception("Invalid argument syntax(-<arg>:<value>): " + arg);

		++b;
		std::string::size_type valPos = arg.find(':');
		if (valPos == std::string::npos)
			throw Exception("Invalid argument syntax(-<arg>:<value>): " + arg);

		opt = gimmesilver::GetTypeForString(std::string(b, b+valPos), typeStrArr, arrCnt);
		if (opt == -1)
			throw Exception("Unknown argument: " + arg);

		return ++valPos;
		}
	};
/* 
parameter
 -qlimit:<num>
 -seed:<string>
 -prior:<keyword>
 -output:<dir path>
 -timeout:<num>
*/
struct CrawlerArgv : public Argv
	{
	enum ArgType { Q_LIMIT, SEED, PRIOR, OUTPUT, INPUT, TIMEOUT, SOCK_CNT };
	enum Policy { BFS, };
	int qLimit_;
	int timeout_;
	int sockCnt_;
	Url seed_;
	Policy policy_;
	gimmesilver::path output_;

	CrawlerArgv() : qLimit_(0), timeout_(5), policy_(BFS), seed_("http://www.openmaru.com"), sockCnt_(10) {} 
	bool Parse(std::string arg)
		{
		static gimmesilver::TypeString typeStr[] = {
			{ Q_LIMIT, "qlimit" },
			{ SEED, "seed" },
			{ PRIOR, "sort" },
			{ OUTPUT, "output" },
			{ TIMEOUT, "timeout" },
			{ SOCK_CNT, "sock" },
		};
		static std::string defaultScheme = "http://";

		int opt = 0;
		std::string::size_type val = Argv::ParseOpt(arg, opt, typeStr, COUNT_OF(typeStr));
		std::string::const_iterator b = arg.begin(), e = arg.end();
		std::advance(b, val);
		switch (opt)
			{
			case Q_LIMIT: qLimit_ = atoi(&*b); break;
			case SEED: 
				{
				std::string temp = std::string(b, e);
				if (temp.size() < defaultScheme.size() 
					|| StringCompare(temp.c_str(), defaultScheme.c_str(), defaultScheme.size()) != 0)
					seed_ = defaultScheme + temp;
				else
					seed_ = temp;
				break;
				}

			case PRIOR: break;
			case OUTPUT:
				{
				std::string temp(b, e);
				if (!gimmesilver::IsExistDir(temp))
					{
					if (0 != gimmesilver::MakeDir(temp.c_str(), 0755))
						throw Exception("fail to open output directory: " + temp);
					}
				output_ = temp;
				break;
				}
			case TIMEOUT: timeout_ = atoi(&*b); break;
			case SOCK_CNT: sockCnt_ = atoi(&*b); break;
			default: assert(false); break;
			}

		return true;
		}
	};

struct IdxExtArgv : public Argv
	{
	gimmesilver::path output_;
	gimmesilver::path input_;

	enum ArgType { OUTPUT, INPUT };
	bool Parse(std::string arg)
		{
		static gimmesilver::TypeString typeStr[] = {
			{ OUTPUT, "output" },
			{ INPUT, "input" },
		};
		int opt = 0;
		std::string::size_type val = Argv::ParseOpt(arg, opt, typeStr, COUNT_OF(typeStr));
		std::string::const_iterator b = arg.begin(), e = arg.end();
		std::advance(b, val);
		switch (opt)
			{
			case OUTPUT: output_ = std::string(b, e); break;
			case INPUT:
				{
				std::string temp(b, e);
				if (!gimmesilver::IsExistDir(temp))
					throw Exception("invalid input directory: " + temp);
				input_ = temp;
				break;
				}
			default: assert(false); break;
			}

		return true;
		}
	};

#endif

