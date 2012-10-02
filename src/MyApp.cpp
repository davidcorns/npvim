#include <resource.h>
#include "MyApp.h"
#include "util.h"
#include "AppInfo.h"

#define LINENUM_MARGIN_BASE 12


/////////////////////////////////////////////////////////////////////////////////////////////////
/*	helper functions of MyApp	*/

inline void updateLineNumMargin(MyApp& app) {
	//this block of code is copied from notepad++
	int linesVisible = (int) app.SendEditor(SCI_LINESONSCREEN);
	if (linesVisible)
	{
		int firstVisibleLineVis = (int) app.SendEditor(SCI_GETFIRSTVISIBLELINE);
		int lastVisibleLineVis = linesVisible + firstVisibleLineVis + 1;
		int i = 0;
		while (lastVisibleLineVis)
		{
			lastVisibleLineVis /= 10;
			i++;
		}
		i = util::max2(i, 3);
		{
			int pixelWidth = int(8 + i * app.SendEditor(SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)"8"));
			app.SendEditor(SCI_SETMARGINWIDTHN, 0, pixelWidth);
		}
	}
}

inline void updateInfo(MyApp& app) {
	AppInfo* info = app.info;
	info->row = util::getCurLine(app);
	info->col = util::getCurCol(app);
	info->pos = app.SendEditor(SCI_GETCURRENTPOS);
	info->lineCount = app.SendEditor(EM_GETLINECOUNT);
	info->lineLen = app.SendEditor(SCI_LINELENGTH);
}



//////////////////////////////////////////////////////////////////////////////////////////////////
/*	Mode	*/
namespace {
enum Mode {
	INSERT,
	COMMAND,	
};
}	//namespace

////////////////////////////////////////////////////
/*	Insert Mode	*/
void MyApp::toInsertMode() {
	mode = INSERT;
	insMode.init();
}

////////////////////////////////////////////////////
/*	Command Mode	*/

void MyApp::toCommandMode() {
	mode = COMMAND;
	cmdMode.init();
}


//////////////////////////////////////////////////////////////////////////////////////////////////
/*	My App	*/
#define toDefaultMode toCommandMode

MyApp::MyApp():
	info(new AppInfo()),
	cmdMode(*this),
	insMode(*this)
{
}


MyApp::~MyApp() {
	if(info) delete info;
}


void MyApp::Process(MSG* pmsg) {
	TranslateMessage(pmsg);
	DispatchMessage(pmsg);
	
	switch(pmsg->message) {
		case WM_KEYDOWN	:
			switch(pmsg->wParam) {
			/*
				cannot capture the esc key from Notify function in the Mode
				capture directory from the msg instead
			*/
				case VK_ESCAPE:
					toCommandMode();
					break;
			}
	}
	
	updateLineNumMargin(*this);
}


void MyApp::Notify(SCNotification *notification) {
	Base::Notify(notification);
	updateInfo(*this);
	
	/*	
		not use virutla functions for polymorphism 
		because one virtual function cost 30 kb	
		use enum instead
	*/
	switch(mode) {
		case INSERT:
			insMode.Notify(notification);
			break;
			
		case COMMAND:
			cmdMode.Notify(notification);
			break;
	}
}

void MyApp::Command(int id) {
	switch (id) {
		case IDM_FILE_NEW:
		case IDM_FILE_OPEN:
		case IDM_FILE_SAVE:
		case IDM_FILE_SAVEAS:
			SendEditor(SCI_SETREADONLY, false);
			Base::Command(id);
			toDefaultMode();
			break;
			
		default:
			Base::Command(id);
	};	//switch (id)
	::SetFocus(wEditor);
}

void MyApp::InitialiseEditor() {
	Base::InitialiseEditor();
	SendEditor(SCI_SETMARGINWIDTHN, 0, LINENUM_MARGIN_BASE);	//shows line numbers
	toDefaultMode();
}



