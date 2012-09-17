#ifndef __UTIL_H__
#define __UTIL_H__

class DMApp;

namespace util {
	int getEndLine(DMApp* app);
	int getCurLine(DMApp* app);
	int getCurCol(DMApp* app);
	
	inline bool isDigit(char ch) {
		return ch >= '0' && ch <= '9';
	}
	
} //namespace util

	
#endif	//__UTIL_H__

