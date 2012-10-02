#ifndef __COMMANDMODE_H__
#define __COMMANDMODE_H__


#include "AppMode.h"

namespace State {
	class StateView; 
}	//namespace State

class CommandMode :public AppMode {
	typedef AppMode Base;
	
private:
	State::StateView* sv;
	
public:
	CommandMode(MyApp& app);
	~CommandMode();
	
	void Notify(SCNotification* notification);
	void init();

	
};	//class CommandMode
	
#endif	//__COMMANDMODE_H__

