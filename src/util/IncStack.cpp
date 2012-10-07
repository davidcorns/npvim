#include <util/IncStack.h>

namespace util {

// Template Instantiation	
template class IncStack<int>;

template<class T>
IncStack<T>::IncStack<T>() {
	dcur = dhead = new Data();
	dhead->next = 0;

	tnext = dhead->ary;
	ttail = dhead->ary + FRAG_SIZE;
}

template<class T>
IncStack<T>::~IncStack<T>() {
	for(Data* next, *cur = dhead;; cur = next) {
			next = cur->next;
			delete cur;
			if(next == 0) break;
	}
}


template<class T>
void IncStack<T>::push(const T& x) {
	if((void*)tnext == (void*)ttail) {
		Data*& next = *ttail;
		next  = new Data();
		next->next = 0;
		
		tnext = next->ary;
	}
	
	*tnext = x;
	++tnext;
}


template<class T>
T IncStack<T>::top() const {
	return *(tnext-1);
}


}	//namespace util
