#ifndef __UTIL_CYCLE_STACK_H__
#define __UTIL_CYCLE_STACK_H__

namespace util {

template<class T>
class CycleStack {
	static const int STACK_SIZE = 1024;
private:
	int _cur;
	T _a[STACK_SIZE];
	
public:
	CycleStack() : _cur(-1){}
	
	void push(const T& x) { 
		_cur = (_cur+1)%STACK_SIZE;
		_a[_cur] = x; 
	}
	
	T top() const { return _a[_cur]; }
	
};	//class CycleStack


}	//namespace util
	
#endif	//__UTIL_CYCLE_STACK_H__

