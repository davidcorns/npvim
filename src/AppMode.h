#ifndef __APP_MODE_H__
#define __APP_MODE_H__


class MyApp;
class SCNotification;

//abstract class AppMode
class AppMode {
protected:
	MyApp& app;

	AppMode(MyApp& a) : app(a) {}
	
	/*	
		not use virutla functions for polymorphism 
		because one virtual function cost 30 kb	
	*/
	//virtual void Notify(SCNotification*) = 0;
	//virtual void init() {};
	//virtual void fin() {};
	
};	//class Mode
	
#endif	//__APP_MODE_H__

