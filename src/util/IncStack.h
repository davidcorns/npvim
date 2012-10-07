#ifndef __UTIL_INCSTACK_H__
#define __UTIL_INCSTACK_H__

namespace util {

const int FRAG_SIZE = 1024;

template<class T>
class IncStack {
	
private:
	struct Data {
		T ary[FRAG_SIZE];
		Data* next;
	} *dhead, **ttail;
	
	T* tnext;
	
public:
	IncStack();
	~IncStack();
	
	void push(const T& x);
	T top() const;
	
};	//class IncStack

typedef IncStack<int> IntIncStack;

}	//namespace util
	
#endif	//__UTIL_INCSTACK_H__

