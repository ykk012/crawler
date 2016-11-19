#ifndef ___URLPARSER_H___
#define ___URLPARSER_H___

/// refer to RFC 1808
class Url
	{
	public:
		enum Range { SCHEME = 0x01, NETLOC = 0x02, PATH = 0x04, PARAM = 0x08, QUERY = 0x10, FRAGMENT = 0x20, ALL = 0xFF };

		Url() {}
		Url(const std::string& url);
		Url(const Url& other);

		const Url& operator=(const Url& others);	/// URL 단순 교체
		void Update(const Url& newUrl);	/// absolute path algorithm 적용하여 변경

		bool operator==(const Url& other) const;
		bool operator<(const Url& other) const;
		bool empty() const { return url_.GetUrl().empty(); }
		std::string string(int r = ALL) const { return url_.GetUrl(r); }

		const std::string& GetScheme() const { return url_. scheme_; }
		const std::string& GetNetLoc() const { return url_.net_loc_; } 
		const std::string& GetPath() const { return url_.path_; }
		const std::string& GetParam() const { return url_.param_; }
		const std::string& GetQuery() const { return url_.query_; }
		const std::string& GetFragment() const { return url_.frag_; }

		void SetScheme(const std::string& scheme) { url_.scheme_ = scheme; }
		void SetNetLoc(const std::string& netloc) { url_.net_loc_ = netloc; }
		void SetPath(const std::string& path) { url_.path_ = path; }
		void SetParam(const std::string& param) { url_.param_ = param; }
		void SetQuery(const std::string& query) { url_.query_ = query; }
		void SetFragment(const std::string& frag) { url_.frag_ = frag; }

		int GetPort() const { return url_.port_; }

		static Url GetAbsPath(const Url& base, const Url& url);
		static bool CompareNetloc(const std::string& l_netloc, const std::string& r_netloc);

	private:
		struct _st_url
			{
			enum { DEFAULT_PORT = 80 };
			
			_st_url() : port_(DEFAULT_PORT) {}
			std::string scheme_;
			std::string net_loc_;
			std::string path_;
			std::string param_;
			std::string query_;
			std::string frag_;
			int port_;

			std::string GetUrl(int range = ALL) const
				{
				std::string url;
				if (range & SCHEME && !scheme_.empty())
					url = scheme_ + ":";
				if (range & NETLOC && !net_loc_.empty())
					url += "//" + net_loc_;
				if (range & NETLOC && port_ != DEFAULT_PORT)
					url += ":" + gimmesilver::ToString(port_);
				if (range & PATH)
					{
					url += path_;
					if (path_.size() > 1 && *path_.rbegin() == '/' 
						&& path_.find('.') != std::string::npos)
						url.erase(url.rfind('/'));
					}
				if (range & PARAM && !param_.empty())
					url += ";" + param_;
				if (range & QUERY && !query_.empty())
					url += "?" + query_;
				if (range & FRAGMENT && !frag_.empty())
					url += "#" + frag_;
				return url;
				}

			void swap(_st_url& other)
				{
				scheme_.swap(other.scheme_);
				net_loc_.swap(other.net_loc_);
				path_.swap(other.path_);
				param_.swap(other.param_);
				query_.swap(other.query_);
				frag_.swap(other.frag_);
				std::swap(port_, other.port_);
				}
			};

		void swap(Url& other);
		static bool Parse(std::string urlString, _st_url& urlStruct);
		//static std::string EncodeUrl(const std::string& path);

		_st_url url_;
	};

#endif

