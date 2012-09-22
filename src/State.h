#ifndef __STATE_H__
#define __STATE_H__

//forward declaration
class MyApp;

namespace State {
	typedef int(*Func)(MyApp*,char);
	int Start(MyApp* app, char ch);
}	//namespace State 

	
#endif	//__STATE_H__

