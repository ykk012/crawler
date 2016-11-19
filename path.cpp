// path.cpp: implementation of the path class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "path.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace gimmesilver;

static const char sep_for_unix_c = '/';
static const char* sep_for_unix = "/";
static const char sep_for_win_c = '\\';
static const char* sep_for_win = "\\";
static const char* file_ext = ".";

path::path(FILE_SYSTEM_TYPE type /*= UNIX*/, PATH_TYPE ptype /*=UNKNOWN*/) : type_(type), pathType_(ptype)
	{
	}

path::path(const path& others) 
	: pathString_(others.pathString_), type_(others.type_), pathType_(others.pathType_)
	{
	}

path::path(const std::string& src, FILE_SYSTEM_TYPE type /*= UNIX*/, PATH_TYPE ptype /*=UNKNOWN*/) 
	: pathString_(src), type_(type), pathType_(ptype)
	{
	if (type_ == WINDOWS)
		replace_unix_path_to_win_path();
	else
		replace_win_path_to_unix_path();
	remove_last_sep();
	}

path::path(const char* src, FILE_SYSTEM_TYPE type /*= UNIX*/, PATH_TYPE ptype /*=UNKNOWN*/) 
	: pathString_(src), type_(type), pathType_(ptype)
	{
	if (type_ == WINDOWS)
		replace_unix_path_to_win_path();
	else
		replace_win_path_to_unix_path();
	remove_last_sep();
	}

const path& path::operator=(const path& others)
	{
	if (this != &others)
		{
		path tmp(others);
		swap(tmp);
		if (others.type_ != type_)
			{
			if (type_ == WINDOWS)
				replace_unix_path_to_win_path();
			else
				replace_win_path_to_unix_path();
			}
		}

	return *this;
	}
/*/
path& path::operator=(path others)
	{
	swap(others);
	return *this;
	}
//*/
path::~path()
	{
	}

void path::replace_unix_path_to_win_path()
	{
	std::string::iterator b = pathString_.begin(), e = pathString_.end();
	while (b != e)
		{
		if (*b == sep_for_unix_c)
			*b = sep_for_win_c;

		++b;
		}
	}

void path::replace_win_path_to_unix_path()
	{
	std::string::iterator b = pathString_.begin(), e = pathString_.end();
	while (b != e)
		{
		if (*b == sep_for_win_c)
			*b = sep_for_unix_c;

		++b;
		}
	}

void path::remove_last_sep()
	{
	std::string::reverse_iterator rb = pathString_.rbegin(), re = pathString_.rend(), cur;
	cur = rb;
	while (cur != re && (*cur == (type_ == WINDOWS ? sep_for_win_c : sep_for_unix_c) || *cur == '.'))
		++cur;

	if (cur != rb)
		pathString_.erase(re - cur);
	}

const std::string& path::string() const
	{
	return pathString_;
	}

const char* path::c_str() const
	{
	return pathString_.c_str();
	}

path& path::operator /= (const path& rhs)
	{
	if (this != &rhs)
		{
		this->pathString_ += type_ == WINDOWS ? sep_for_win : sep_for_unix;
		this->pathString_ += rhs.pathString_;
		}

	return *this;
	}

path path::operator / (const path& rhs) const
	{
	path temp(*this);
	return temp /= rhs;
	}

bool path::operator ==(const path& rhs) const
	{
	if (this != &rhs)
		return this->pathString_ == rhs.pathString_;

	return true;
	}

bool path::operator !=(const path& rhs) const
	{
	return !(*this == rhs);
	}

bool path::operator <(const path& rhs) const
	{
	return this->pathString_ < rhs.pathString_;
	}

bool path::has_file() const
	{
	return !file_string().empty();
	}

std::string path::root_path() const
	{
	std::string::size_type n = pathString_.find_first_of(type_ == WINDOWS ? sep_for_win : sep_for_unix);
	if (n == std::string::npos)
		return this->string();
	else
		return std::string(pathString_.begin(), pathString_.begin()+n);
	}

path path::directory() const
	{
	if (has_file())
		{
		std::string temp(string());
		return path( temp.erase(temp.find_last_of(type_ == WINDOWS ? sep_for_win : sep_for_unix)) );
		}
	else
		{
		return *this;
		}
	}

path path::parent_path() const
	{
	std::string sParent = directory_string();
	std::string::size_type n = sParent.find_last_of(type_ == WINDOWS ? sep_for_win : sep_for_unix);
	if (n == std::string::npos)
		return *this;
	else
		return path( sParent.erase(n) );
	}

std::string path::directory_string() const
	{
	return directory().string();
	}

std::string path::file_string(FlagType flag) const
	{
	std::string file;
	switch (pathType_)
		{
		case DIR_TYPE: break;
		case UNKNOWN:
			if (IsExistDir(pathString_))
				break;

		case FILE_TYPE:
			{
			std::string temp(string());
			file.assign(temp.begin() + temp.find_last_of(type_ == WINDOWS ? sep_for_win : sep_for_unix) + 1, temp.end());
			std::string::size_type index = file.find_last_of(file_ext);
			if (index == std::string::npos)
				index = file.size();

			switch (flag)
				{
				case PATH_ONLY_FILE_NAME:	file.erase(index);
				case PATH_ONLY_FILE_EXT:
					if (index < file.size())
						file.erase(0, index+1);
					break;
				}
			}
		}

	return file;
	}

std::string path::leaf() const
	{
	std::string temp(directory_string());
	return std::string(temp.begin() + temp.find_last_of(type_ == WINDOWS ? sep_for_win : sep_for_unix) + 1, temp.end());
	}

int path::get_depth() const
	{
	int cnt = has_file() ? 0 : 1;
	std::string::const_iterator b = pathString_.begin(), e = pathString_.end();
	while (b != e)
		{
		if (*b == (type_ == WINDOWS ? sep_for_win_c : sep_for_unix_c))
			++cnt;
		++b;
		}

	return cnt;
	}

void path::swap(path& others)
	{
	//std::swap(pathString_, others.pathString_);
	pathString_.swap(others.pathString_);
	}

path operator / (const std::string& lhs, const path& rhs)
	{
	path temp(lhs);
	temp /= rhs;
	return temp;
	}

bool operator ==(const std::string& lhs, const path& rhs)
	{
	path temp(lhs);
	return temp == rhs;
	}

