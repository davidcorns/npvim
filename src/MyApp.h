#ifndef __MYAPP_H__
#define __MYAPP_H__

#include "DMApp.h"
#include "CommandMode.h"
#include "InsertMode.h"

//forward declaration
class AppInfo;


class MyApp : public DMApp{
	typedef DMApp Base;

private:
	AppInfo* info;
	
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
	
	/*	info	*/
	inline const AppInfo& getInfo() { return *info; }
	
};	//class MyApp
	
#endif	//__MYAPP_H__

