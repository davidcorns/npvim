#ifndef __UTIL_H__
#define __UTIL_H__

class DMApp;

namespace util {
	int getEndLine(DMApp& app);
	int getCurLine(DMApp& app);
	int getCurCol(DMApp& app);
	
	inline bool isDigit(char ch) {
		return ch >= '0' && ch <= '9';
	}
	
	/*	
		name colision?? with std lib max & min (though there is namespace-.-)	
		used max2, min2 instead of max & min
	*/
	template<class T>
	inline const T& max2(const T& a, const T& b) {
		return a>b?a:b;
	}

	template<class T>
	inline const T& min2(const T& a, const T& b) {
		return a<b?a:b;
	}

	
} //namespace util

	
#endif	//__UTIL_H__

