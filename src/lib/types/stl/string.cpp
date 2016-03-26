#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

extern "C" {
	#include <types.h>
}

StoreDynamicString StoreCreateDynamicString()
{
	return new std::string();
}

void StoreAppendDynamicString(StoreDynamicString string, const char *format, ...)
{
	va_list vp;
	va_start(vp, format);
	StoreAppendDynamicStringV(string, format, vp);
	va_end(vp);
}

void StoreAppendDynamicStringV(StoreDynamicString string, const char *format, va_list vp)
{
	std::string& stlString = *static_cast<std::string *>(string);

	int formatSize = strlen(format);
	int size = formatSize + 128;
	std::string printfString;
	while(true) {
		va_list currentVp;
		va_copy(currentVp, vp);

		printfString.resize(size);
		int n = vsnprintf(const_cast<char *>(printfString.data()), size, format, currentVp);
		va_end(currentVp);

		if(n > -1 && n < size) { // buffer was large enough
			printfString.resize(n);
			break;
		} else {
			size *= 2; // try again with larger buffer
		}
	}

	stlString += printfString;
}

const char *StoreReadDynamicString(StoreDynamicString string)
{
	std::string& stlString = *static_cast<std::string *>(string);
	return stlString.data();
}

void StoreFreeDynamicString(StoreDynamicString string)
{
	delete static_cast<std::string *>(string);
}
