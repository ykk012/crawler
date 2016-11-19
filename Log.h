#ifndef ___LOG_H___
#define ___LOG_H___

class LogStream
	{
	public:
		enum ObjType { STD_OUT, FILE_OUT };
		static LogStream& GetObj(ObjType type);

		typedef std::ostream& (*_F)(std::ostream&);

		virtual LogStream& operator<<(const std::string& s) = 0;
		virtual LogStream& operator<<(int n) = 0;
		virtual LogStream& operator<<(_F) = 0;
	};

class NullStream : public LogStream
	{
	public:
		virtual LogStream& operator<<(const std::string& s) { assert(false); return *this; }
		virtual LogStream& operator<<(int n) { assert(false); return *this; }
		virtual LogStream& operator<<(_F f) { assert(false); return *this; }
	};

class StdOut : public LogStream
	{
	public:
		virtual LogStream& operator<<(const std::string& s) { std::cout << s; return *this; }
		virtual LogStream& operator<<(int n) { std::cout << n; return *this; }
		virtual LogStream& operator<<(_F f) { f(std::cout); return *this; }

		static LogStream& GetObj();
	};

class FileOut : public LogStream
	{
	public:
		FileOut(const char* file) : logFile_(file) {}
		virtual LogStream& operator<<(const std::string& s) { logFile_ << s; return *this; }
		virtual LogStream& operator<<(int n) { logFile_ << n; return *this; }
		virtual LogStream& operator<<(_F f) { f(logFile_); return *this; }

		static LogStream& GetObj();
	private:
		std::ofstream logFile_;
	};

#endif

