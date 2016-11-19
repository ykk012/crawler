#include "StdAfx.h"
#include "Log.h"

LogStream& StdOut::GetObj()
	{
	static StdOut stdLog;
	return stdLog;
	}

LogStream& FileOut::GetObj()
	{
	static FileOut fileLog("log.txt");
	return fileLog;
	}

LogStream& LogStream::GetObj(ObjType type)
	{
	static NullStream nullLog;
	switch (type)
		{
		case STD_OUT: return StdOut::GetObj();
		case FILE_OUT: return FileOut::GetObj();
		}

	return nullLog;
	}
