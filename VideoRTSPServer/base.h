#pragma once
#include <string>
#include "windows.h"
#include "EdoyunThread.h"
typedef unsigned char BYTE;

class CBuffer :public std::string
{
public:
	CBuffer(const char* str)
	{
		resize(strlen(str));
		memcpy((char*)c_str(), str, size());
	}

	CBuffer(size_t size = 0) :std::string()
	{
		if (size > 0) {
			resize(size);
			memset((void*)c_str(), 0, size);
		}
	}

	CBuffer(void* buffer, size_t size)
	{
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}

	~CBuffer()
	{
		std::string::~basic_string();
	}

	operator char* ()const { return (char*)c_str(); }
	operator const char* ()const { return c_str(); }
	operator BYTE* ()const { return (BYTE*)c_str(); }
	operator void* ()const { return (void*)c_str(); }
	void Update(void* buffer, size_t size)
	{
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}

	void Zero() {
		if (size() > 0)
			memset((void*)c_str(), 0, size());
	}

	CBuffer& operator<<(const CBuffer& buf)
	{
		if (this != &buf) {
			*this += buf;
		}
		else
		{
			CBuffer tmp = buf;
			*this += tmp;
		}
		return *this;
	}

	CBuffer& operator<<(int num)
	{
		char s[16] = "";
		snprintf(s, sizeof(s), "%d", num);
		*this += s;
		return *this;
	}

	const CBuffer& operator>>(int& num)const
	{
		num = atoi(c_str());
		return *this;
	}
};
