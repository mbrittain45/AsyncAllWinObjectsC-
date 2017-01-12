
#include "stdafx.h"
#include "Defines.h"
#include "Message.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// static vars
mutex Utils::mtx;
vector<WStr> Utils::errs;


// get a substring 
WStr Utils::getSubStr(WStr sMsg, wchar_t wc)
{
	WStr t;
	size_t p = sMsg.find( wc );
	if ( p == -1 ) {
		t = sMsg;
		sMsg.empty();
	}
	else {
		WStr t = sMsg.substr( 0, p );
		sMsg.erase( 0, p + 1 );
	}
	return t;
}

WStr Utils::getSubStr(WStr sMsg, WStr wstr )
{
	WStr t;
	size_t p = sMsg.find( wstr );
	if (p == -1) {
		t = sMsg;
		sMsg.empty();
	}
	else {
		t = sMsg.substr(0, p);
		sMsg.erase(0, p + 1);
	}
	return t;
}

WStr Utils::format(const char* format, ...)
{
	char buffer[512] = { 0 };
	va_list args;
	va_start(args, format);
	vsprintf_s(buffer, format, args);
	va_end(args);
	
	return charToWStr(buffer);
}

// string conversion
WStr Utils::charToWStr(char* _str)
{
	size_t szConverted, len = strlen( _str );
	wchar_t* wbuf = new wchar_t[len + 1];
	mbstowcs_s( &szConverted, wbuf, (len + 1), _str, len );
	WStr wstr = wbuf;
	wstr.erase(len, wstr.length());
	delete [] wbuf;
	return wstr;
}

void Utils::wstrToChar( WStr wstr, size_t len, char* buf)
{
	size_t szConverted;
	wcstombs_s( &szConverted, buf, len, wstr.data(), len );
}

WStr Utils::formatError(uint32_t errCode)
{
	wchar_t err[1024] = {0};
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errCode, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), err, 1023, NULL);
	return err;
}

int Utils::logSocketError(WStr use, Message& msg)
{
	int err = WSAGetLastError();
	if (err != WSA_IO_PENDING)
	{
		msg.data = use + L"failed with error: " + formatError(err);
		Utils::captureError(msg.data);
	}
	return err;
}


void Utils::captureError(WStr s)
{
	lock_guard<mutex> lck(mtx);
	errs.push_back(s);
}


void Utils::captureError(const wchar_t * format, ...)
{
	wchar_t buf[512] = { 0 };
	va_list args;
	va_start(args, format);
	vswprintf_s(buf, format, args);
	va_end(args);

	{
		lock_guard<mutex> lck(mtx);
		TRACE(buf);
		errs.push_back(buf);
	}
}


// write errors out to the log file
void Utils::writeOutErrors()
{
	vector<WStr> logErrs;

	// copy items for logging
	{
		lock_guard<mutex> lck(mtx);
		logErrs.swap(errs);
	}

	// make day portion of file names
	CTime ct = CTime::GetCurrentTime();
	WStr day = Utils::formatW("%d-%d-%d", ct.GetMonth(), ct.GetDay(), ct.GetYear() - 2000);
	
	// set the file names
	WStr file = L"c:\\temp\\NodeWorkerLog" + day + L".txt";
	WStr tmpFile = L"c:\\temp\\NodeWorkerLog" + day + L".tmp";

	// copy the file to a temp while we work
	::CopyFile( file.data(), tmpFile.data(), false);

	// write to the file
	wfstream fstrm( tmpFile, wfstream::out);
	if (fstrm.fail())
	{
		logError(L"Unable to open temp log file");
		return;
	}

	// write out the entries
	WStr msg;
	size_t size = logErrs.size();
	for (size_t i = 0; i < size; ++i) {
		msg = (logErrs[i] + L" \n");
		fstrm.write(msg.data(), msg.length());
	}

	// close the file
	fstrm.close();

	// rename to the normal file
	::DeleteFile( file.data() );
	::MoveFile( tmpFile.data(), file.data() );
}


// a precision timer class
Timer::Timer() 
{
	start = ChronoHiRes::now();
}


chrono::duration<__int64,nano> Timer::Stop()
{
	ChronoHiRes::time_point end = ChronoHiRes::now();
	chrono::duration<__int64, nano> ticks = (end - start);
	return ticks;
}


// usage:
//Timer tmr; 
//int j = 0; for (int i = 0; i < 100000000; ++i) j = i;
//wcout << Utils::toWStr( L"It took %f seconds", (tmr.Stop().count() / 1000000000.0) );
//chrono::duration<long, ratio<1, 1000>> oneMillisecond(1);

/*
template<class rep, class period>
void blink_led(duration<rep, period> time_to_blink) // good -- accepts any unit
{
// assuming that millisecond is the smallest relevant unit
auto milliseconds_to_blink = duration_cast<milliseconds>(time_to_blink);
// ...
// do something with milliseconds_to_blink
// ...
}
*/


// thread pair
ThreadPair::~ThreadPair()
{
	if (hnd != BAD_HAND)
		CloseHandle(hnd);
}


// bitwise - SWAR algorithm
int NumberOfSetBits(int i)
{
	i = i - ((i >> 1) & 0x55555555);
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
	return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

