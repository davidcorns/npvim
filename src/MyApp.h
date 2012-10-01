#ifndef __MYAPP_H__
#define __MYAPP_H__

#include "DMApp.h"
#include "CommandMode.h"
#include "InsertMode.h"
#include "State.h"

//forward declaration
class AppInfo;


class MyApp : public DMApp{
	typedef DMApp Base;
	
public:
	AppInfo* info; 
	
private:
	//TODO: may move to heap in the future
	CommandMode cmdMode;
	InsertMode insMode;
	int mode;

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
	
	//command mode
	void toCommandMode();
	int toState(State::Func s);
	
	/*	helper functions	*/
	inline void trace(char* msg) {::MessageBox(wMain, msg, msg, MB_OK);	}
	void trace(int i);

	
};	//class MyApp
	
#endif	//__MYAPP_H__

