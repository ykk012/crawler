#ifndef ___PATH_H___
#define ___PATH_H___

namespace gimmesilver {
	class path  
		{
		public:
			enum FILE_SYSTEM_TYPE { WINDOWS, UNIX };
			enum PATH_TYPE { UNKNOWN, DIR_TYPE, FILE_TYPE };

			path(FILE_SYSTEM_TYPE type = UNIX, PATH_TYPE ptype = UNKNOWN);
			path(const path& others);		/// copy ctor

			path(const std::string& src, FILE_SYSTEM_TYPE type = UNIX, PATH_TYPE ptype = UNKNOWN);
			path(const char* src, FILE_SYSTEM_TYPE type = UNIX, PATH_TYPE ptype = UNKNOWN);
			~path();	/// do not inherit!!!

			/// assignment operator
			const path& operator=(const path& others);
			/*/path& operator=(path others);//*/	/// optimizing...

			enum FlagType { PATH_FULL_FILE_NAME,
							PATH_ONLY_FILE_NAME,
							PATH_ONLY_FILE_EXT };

			bool operator ==(const path& rhs) const;
			bool operator !=(const path& rhs) const;
			bool operator <(const path& rhs) const;

			/* 경로명 붙이기
			* path test("C:\\user");
			* test /= "temp";
			* ASSERT(test == "C:\\user\\temp");
			*/
			path& operator /= (const path& rhs);
			path operator / (const path& rhs) const;

			/* root path 구하기
			* path test("C:\\user\\temp");
			* path root = test.root_path();
			* ASSERT(root == "C:");
			*/
			std::string root_path() const;

			/* directory path 구하기
			* path test("C:\\user\\temp\\test.exe");
			* path dir = test.directory();
			* ASSERT(dir == "C:\\user\\temp");
			*/
			path directory() const;

			/* parent directory path 구하기
			* path test("C:\\user\\temp\\test.exe");
			* path parent = test.parent_path();
			* ASSERT(parent == "C:\\user");
			*/
			path parent_path() const;

			const std::string& string() const;
			const char* c_str() const;

			std::string directory_string() const;
			std::string file_string(FlagType flag = PATH_FULL_FILE_NAME) const;

			/* leaf path 구하기
			* path test("C:\\user\\temp\\test.exe");
			* ASSERT(test.leaf() == "temp");
			*/
			std::string leaf() const;

			/* has_file() == !file_string().empty() */
			bool has_file() const;
			bool empty() const { return pathString_.empty(); }

			/* directory depth 구하기
			* path test("C:\\user\\temp\\test.exe");
			* ASSERT(test.get_depth() == 3)
			* ASSERT(test.root_path().get_depth() == 1)
			* ASSERT(path(test.leaf()).get_depth() == 1)
			* ASSERT(path(test.file_string()).get_depth() == 0)
			*/
			int get_depth() const;
			void swap(path& others);

		private:
			void remove_last_sep();
			void replace_unix_path_to_win_path();
			void replace_win_path_to_unix_path();

			std::string	pathString_;
			FILE_SYSTEM_TYPE type_;
			PATH_TYPE pathType_;
		};

	//beacon::path operator / (const char* lhs, const beacon::path& rhs);
	gimmesilver::path operator / (const std::string& lhs, const path& rhs);
	//bool operator ==(const char* lhs, const beacon::path& rhs);
	bool operator ==(const std::string& lhs, const path& rhs);

#if FLATFORM & WIN32_FLATFORM
	inline path GetAppDirectory()
		{
		char temp[1024];
		::GetModuleFileName(0, temp, sizeof(temp));
		return path(temp);
		}

	inline path GetCurrentDirectory()
		{
		char temp[1024];
		::GetCurrentDirectory(sizeof(temp), temp);
		return path(temp);
		}
#endif	
	inline bool IsExistDir(path dir)
		{
#if FLATFORM & WIN32_FLATFORM
		dir /= "*.*";
		WIN32_FIND_DATA wfd;
		memset(&wfd, 0, sizeof(wfd));
		HANDLE handle = FindFirstFile(dir.c_str(), &wfd);
		if (handle == INVALID_HANDLE_VALUE)
			return false;

		FindClose(handle);
		return true;
#else
		DIR* dirptr = opendir(dir.c_str());
		if (!dirptr)
			return false;
		closedir(dirptr);
		return true;
#endif
		}

	inline int MakeDir(const char* dir, int mode)
		{
#if FLATFORM & WIN32_FLATFORM
		return mkdir(dir);
#else
		return mkdir(dir, mode);
#endif
		}
	}

#endif




