#include <cstdio>
#include <math.h>
#include <resource.h>

#include <MyApp.h>
#include <util.h>
#include <StateEnum.h>
#include <AppInfo.h>

#define LINENUM_MARGIN_BASE 12


/////////////////////////////////////////////////////////////////////////////////////////////////
/*	helper functions of MyApp	*/

void updateLineNumMargin(MyApp* app) {
	//line number margin auto adjust
	const int margin = int(1 + log10(app->info->lineCount)) * LINENUM_MARGIN_BASE;
	app->SendEditor(SCI_SETMARGINWIDTHN, 0, margin);
}

void updateInfo(MyApp* app) {
	AppInfo* info = app->info;
	info->row = util::getCurLine(app);
	info->col = util::getCurCol(app);
	info->pos = app->SendEditor(SCI_GETCURRENTPOS);
	info->lineCount = app->SendEditor(EM_GETLINECOUNT);
	info->lineLen = app->SendEditor(SCI_LINELENGTH);
}



//////////////////////////////////////////////////////////////////////////////////////////////////
/*	Mode	*/
namespace {
enum Mode {
	INSERT,
	COMMAND,	
};
}	//namespace


/*	Insert Mode	*/
void MyApp::toInsertMode() {
	mode = INSERT;
	SendEditor(SCI_SETREADONLY, false);
}

void MyApp::InsertModeNotify(SCNotification* notification) {
	
	switch (notification->nmhdr.code) {
		case SCN_CHARADDED: {
			const char& ch = notification->ch;	
				
			switch(ch) {
				case '\n': {
					//auto indent
					const int lineNum = util::getEndLine(this);
					char* txt = new char[info->lineLen];
					SendEditor(SCI_GETLINE, lineNum, reinterpret_cast<LPARAM>(txt));
					char* p = txt;
					while(*p==' ' || *p=='\t') {
						++p;	
					}
					if(*p != '\0' && *p!='\r' && *p!='\n' && p!=txt) {
						SendEditor(SCI_ADDTEXT, p-txt, reinterpret_cast<LPARAM>(txt));
					}
					if(txt) delete txt;
				} break;
			}
		} break;
	}	//switch
}

////////////////////////////////////////////////////
/*	Command Mode	*/

#define DEFAULT_STATE State::Start

void MyApp::initState() {
	memset(info->cmdNum, 0, sizeof(info->cmdNum));
	toState(DEFAULT_STATE);
}

int MyApp::toState(State::Func s) {
	state = s;
	return State::CONTINUE;
}

void MyApp::toCommandMode() {
	mode = COMMAND;
	SendEditor(SCI_SETREADONLY, true);	
	this->initState();
}


void MyApp::CommandModeNotify(SCNotification* notification) {
	switch(notification->nmhdr.code) {
		case SCN_CHARADDED:
			const int res = state(this, notification->ch);
			switch(res) {
				case State::SUCCESS:
				case State::FAIL:
					this->initState();
					break;
			}
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/*	My App	*/
#define toDefaultMode toCommandMode

MyApp::MyApp() {
	info = new AppInfo();
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
				//cannot capture the esc key from Notify function in the Mode
				//capture directory from the msg instead
				case VK_ESCAPE:
					toCommandMode();
					break;
			}
	}
	
	updateLineNumMargin(this);
}


void MyApp::Notify(SCNotification *notification) {
	Base::Notify(notification);
	updateInfo(this);
	switch(mode) {
		case INSERT:
			InsertModeNotify(notification);
			break;
			
		case COMMAND:
			CommandModeNotify(notification);
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
	//SendEditor(SCI_SETMARGINWIDTHN, 1, 30);					//shows symbols
	toDefaultMode();
}



void MyApp::trace(int i) {
  char msg[256]; sprintf(msg, "%i", i); trace(msg);
}


