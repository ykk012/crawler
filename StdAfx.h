#ifndef ___STDAFX_H___
#define ___STDAFX_H___

#define WIN32_FLATFORM 1
#define LINUX_FLATFORM 2

#define FLATFORM WIN32_FLATFORM

#if FLATFORM & WIN32_FLATFORM

#include <winsock2.h>
#pragma comment(lib, "WS2_32.lib")

#include <windows.h>
#include <direct.h>
#pragma warning(disable:4786)
#pragma warning(disable:4503)

#define StringCompare strnicmp

#else

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netdb.h>
#include <semaphore.h>
#include <pthread.h>

#define StringCompare strncasecmp
#endif

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <algorithm>
#include <functional>
#include <assert.h>
#include <fstream>
#include <iterator>
#include <sstream>

#include "gimmesilver.h"
#include "path.h"
#include "Log.h"

#define COUNT_OF(x) sizeof(x) / sizeof(x[0])

extern LogStream& g_log;

#endif

