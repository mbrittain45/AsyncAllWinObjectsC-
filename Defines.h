

#pragma once


#include "stdafx.h"

#include <cassert> 
#include <cstdint> 
#include <string> 
#include <cstdarg>
#include <ostream>
#include <istream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <mutex>
#include <map>
#include <list>
#include <vector>
#include <queue>
#include <codecvt>
#include <cstdio>
#include <atomic>


using namespace std;

class Message;


// shorten things up a bit
typedef wstring WStr;
typedef WStr Str;
typedef chrono::high_resolution_clock ChronoHiRes;


// bad handle
#define BAD_HAND INVALID_HANDLE_VALUE
#define BAD_VAL -1


// backup file extension 
#define BACKUP_EXT L".bkp"


// our thread memmory size - veverything must be allocated at this size
#define THREAD_STACK_SIZE	64000


// Io Op Return Codes
enum class IoRetCode
{
	eIoUnknown,	// nothing / unknown
	eIoSyncOk,	// sync operation and ok
	eIoASyncOk,	// oasync operation and ok and now waiting 
	eIoError,	// error along the way
	eIoComplete // completely done - return IoOperation to the application
};


// our Utilities
class Utils
{
public:

	static WStr format(const char* format, ...);
	
	static WStr charToWStr(char* str);
	static void wstrToChar(WStr wstr, size_t len, char* buf);

	static WStr getSubStr(WStr sMsg, wchar_t wc);
	static WStr getSubStr(WStr sMsg, WStr wstr);

	static WStr formatError(uint32_t errCode);
	static int  logSocketError(WStr use, Message& msg);

	static void captureError(WStr s);
	static void captureError(const char* format, ...);
	
	static void writeOutErrors();
	static bool hasErrors() {
		return errs.size() ? true : false;
	}

protected:

	static mutex mtx;
	static vector<WStr> errs;
};



// a precision timer class
class Timer
{
public:
	ChronoHiRes::time_point start;

	Timer();
	chrono::duration<__int64, nano> Stop();

	//Timer tmr; 
	//int j = 0; for (int i = 0; i < 100000000; ++i) j = i;
	//wcout << Utils::toWStr( L"It took %f seconds", (tmr.Stop().count() / 1000000000.0) );
	//chrono::duration<long, ratio<1, 1000>> oneMillisecond(1);
};



// a thread pair id and handle
class ThreadPair
{
public:
	HANDLE hnd = BAD_HAND;
	DWORD  id = 0;

	~ThreadPair();
};


