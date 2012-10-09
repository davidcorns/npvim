#ifndef __DEBUG_H__
#define __DEBUG_H__

#define DEBUG 1

#ifdef DEBUG

namespace debug {
	void log(char* msg, ...);
	inline void log(int i) { log("%i", i); }
}	//namespace debug
	
	
#endif	//DEBUG
	
#endif	//__DEBUG_H__

