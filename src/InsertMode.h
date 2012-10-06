#ifndef __INSERTMODE_H__
#define __INSERTMODE_H__

#include <AppMode.h>

class InsertMode : public AppMode {
	typedef AppMode Base;
	
public:
	InsertMode(MyApp& app);
	~InsertMode();
	
	void Notify(SCNotification* notification);
	void init();
};	//class InsertMode
	
#endif	//__INSERTMODE_H__

