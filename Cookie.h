#ifndef ___COOKIE_H___
#define ___COOKIE_H___

#include "UrlParser.h"
#include "Sync.h"

class Cookie  
	{
	public:		
		std::string GetCookie(const Url& url);

		void Insert(const std::vector<std::string>& cookieArr, const Url& url);
		void Insert(const std::string& cookie, const Url& url);

	private:
		struct _st_cookie {
			std::string	value_;
			std::string	expires_;
			std::string	path_;
			std::string	domain_;
			std::string	secure_;
		};

		typedef std::multimap<std::string, _st_cookie> CookieContainer;

		void Parse(std::string cookie, const Url& url);

		bool IsExpired(const std::string& date);
		time_t Date2Time(const std::string& date);
		std::string SearchCookieValue(const std::string& cookie, const std::string& delimeter);

		CookieContainer	cookie_;
		gimmesilver::SyncObj sync_;
	};

#endif

