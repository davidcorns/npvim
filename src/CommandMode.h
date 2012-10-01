#ifndef __COMMANDMODE_H__
#define __COMMANDMODE_H__


#include "AppMode.h"
#include <State.h>

class CommandMode :public AppMode {
	typedef AppMode Base;
	
private:
	State::Func state;
	
public:
	CommandMode(MyApp& app);
	~CommandMode();
	
	void Notify(SCNotification* notification);
	void init();
	int toState(State::Func s);
	
private:
	void initState();
	
};	//class CommandMode
	
#endif	//__COMMANDMODE_H__

