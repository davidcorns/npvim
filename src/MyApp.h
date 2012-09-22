#ifndef __MYAPP_H__
#define __MYAPP_H__

#include <DMApp.h>
#include <State.h>
#define NUM_BUF_SIZE 2

//forward declaration
class AppInfo;


class MyApp : public DMApp{
	typedef DMApp Base;

public:	
	AppInfo* info;
	
private:
	int mode;
	State::Func state;

public:
	MyApp();
	~MyApp();
	
	void Process(MSG* msg);
	
	/*	override functions	*/
	void Notify(SCNotification *notification);
	void Command(int id);
	void InitialiseEditor();
	
	/*	Mode related functions	*/
		//insert mode
	void toInsertMode();
	void InsertModeNotify(SCNotification*);
	
		//command mode
	void toCommandMode();
	void CommandModeNotify(SCNotification*);
	int toState(State::Func s);
	
	/*	helper functions	*/

	inline void trace(char* msg) {::MessageBox(wMain, msg, msg, MB_OK);	}
	void trace(int i);
	
protected:
	void initState();
	
};	//class MyApp
	
#endif	//__MYAPP_H__

