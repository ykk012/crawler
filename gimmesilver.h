#ifndef ___GIMMESILVER_H___
#define ___GIMMESILVER_H___

#include <string>
#include <iostream>
#include "path.h"
namespace gimmesilver {

class Splitter
	{
	public:
		typedef std::list<std::string>	SplitType;
		Splitter(const std::string& str, char sep) : sep_(sep) { Split(str, sep); }
		void Split(const std::string& str, char sep)
			{
			std::string::const_iterator b = str.begin(), e = str.end();
			std::string::const_iterator prev = b;
			for (; b != e; ++b)
				{
				if (*b == sep)
					{
					arr_.push_back(std::string(prev, b));
					prev = b+1;
					}
				}
			if (prev != b)
				arr_.push_back(std::string(prev, b));
			}

		SplitType::iterator begin() { return arr_.begin(); }
		SplitType::iterator end() { return arr_.end(); }
		void pop_front() { arr_.pop_front(); }
		void pop_back() { arr_.pop_back(); }
		const std::string& front() const { arr_.front(); }
		const std::string& back() const { arr_.back(); }
		const Splitter& operator+=(const Splitter& other)
			{
			std::copy(other.arr_.begin(), other.arr_.end(), std::back_inserter(arr_));
			return *this;
			}
		void RemoveAll(const std::string& val) { arr_.remove(val); }
		SplitType::iterator erase(SplitType::iterator pos) { return arr_.erase(pos); }

		std::string string()
			{
			std::string str;
			SplitType::iterator b = arr_.begin(), e = arr_.end();
			for (; b != e; ++b)
				str += *b + sep_;
			return str;
			}

	private:
		char sep_;
		SplitType arr_;
	};

inline int Hexa2Int(char c)
	{
	switch (c)
		{
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
			return c - 'a' + 10;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			return c - 'A' + 10;
		default:
			if (isdigit(c))
				return c - '0';
			break;
		}

	return -1;
	}

inline int Hexa2Int(const std::string& hex)
	{
	int val = 0;
	int tmp;
	std::string::const_iterator b = hex.begin(), e = hex.end();
	while (b != e)
		{
		tmp = Hexa2Int(*b);
		if (tmp < 0)
			break;

		val <<= 4;
		val += tmp;
		++b;
		}

	return val;
	}

/// refer to Exceptional C++ Style by Herb Sutter...
template <typename T>
std::string ToString(T value)
	{
	std::ostringstream temp;
	temp << value;
	return temp.str();
	}

struct TypeString {
	int type_;
	const char* str_;
};

enum { UNKNOWN = -1 };

inline int GetTypeForString(const std::string& val, const TypeString* typeStrArr, int arrCnt)
	{
	for (int i = 0; i < arrCnt; ++i)
		{
		if (StringCompare(val.c_str(), typeStrArr[i].str_, strlen(typeStrArr[i].str_)) == 0)
			return typeStrArr[i].type_;
		}
	return -1;
	}

struct nocase_traits : public std::char_traits<char>
	{
	static bool eq(char c1, char c2)
		{ return toupper(c1) == toupper(c2); }

	static bool lt(char c1, char c2)
		{ return toupper(c1) < toupper(c2); }

	static int compare(const char* s1, const char* s2, size_t n)
		{
		for (size_t i = 0; i < n; ++i)
			{
			if (!eq(s1[i], s2[i]))
				return lt(s1[i],s2[i]) ? -1 : 1;
			}
		return 0;
		}

	static const char* find(const char* s, size_t n, char c)
		{
		for (size_t i = 0; i < n; ++i)
			if (eq(s[i], c))
				return &s[i];

		return 0;
		}
	};

typedef std::basic_string<char, nocase_traits> istring;

inline bool IsSpace(char c)
	{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
	}

template <int T>
struct Int2Type { enum { VAL = T }; };

inline int ReadFile(std::string& text, const gimmesilver::path& file)
	{
#if FLATFORM & WIN32_FLATFORM
	HANDLE handle = CreateFile(file.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	if (handle == INVALID_HANDLE_VALUE)
		return 0;
	DWORD fileSize = GetFileSize(handle, 0);
	if (text.size() < fileSize)
		text.resize(fileSize);
//	return FALSE != ::ReadFile(handle, &text[0], fileSize, &fileSize, 0);
	if (::ReadFile(handle, &text[0], fileSize, &fileSize, 0))
		return fileSize;

	return 0;
#else
	int handle = open(file.c_str(), O_RDONLY);
	if (handle <= 0)
		return 0;
	int fileSize = lseek(handle, 0, SEEK_END);
	lseek(handle, 0, SEEK_SET);
	if (text.size() < fileSize)
		text.resize(fileSize);
	fileSize = read(handle, &text[0], fileSize);
	close(handle);
	return fileSize;
#endif
	}

}

#endif

