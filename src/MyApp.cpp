#include <cstdio>
#include <math.h>
#include <resource.h>

#include <MyApp.h>
#include <util.h>
#include <KeyMap.h>

#define KEYCODE(x)	x
#define LINENUM_MARGIN_BASE 12


static const int TEXT_BUF_SIZE = 1000;

/////////////////////////////////////////////////////////////////////////////////////////////////
/*	helper functions of MyApp	*/

void updateLineNumMargin(MyApp* app) {
	//line number margin auto adjust
	const int margin = int(1 + log10(app->lineCount)) * LINENUM_MARGIN_BASE;
	app->SendEditor(SCI_SETMARGINWIDTHN, 0, margin);
}

void updateState(MyApp* app) {
	app->row = util::getCurLine(app);
	app->col = util::getCurCol(app);
	app->pos = app->SendEditor(SCI_GETCURRENTPOS);
	app->lineCount = app->SendEditor(EM_GETLINECOUNT);
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
					char* txt = this->getBuf(TEXT_BUF_SIZE);
					SendEditor(SCI_GETLINE, lineNum, reinterpret_cast<LPARAM>(txt));
					char* p = txt;
					while(*p==' ' || *p=='\t') {
						++p;	
					}
					if(*p != '\0' && *p!='\r' && *p!='\n' && p!=txt) {
						SendEditor(SCI_ADDTEXT, p-txt, reinterpret_cast<LPARAM>(txt));
					}
				} break;
			}
		} break;
	}	//switch
}

////////////////////////////////////////////////////
/*	Command Mode	*/
/*	State	*/
namespace State {

//helper functions and classes
bool readingCmdNum(MyApp* app, char ch, int index) {
	int& num = app->cmdNum[index];
	if(util::isDigit(ch)) {
		num *= 10;
		num += ch-'0';
		return true;
	}
	return false;	
}


class ScopeTempAllowEdit {
	MyApp* _app;
public:
	ScopeTempAllowEdit(MyApp* app) : _app(app) { _app->SendEditor(SCI_SETREADONLY, false);	}
	~ScopeTempAllowEdit() { _app->SendEditor(SCI_SETREADONLY, true); }
};


//state functions declaration
int Start(MyApp* app, char ch);	//num is the startup mode
int Norm(MyApp* app, char ch);
int Move(MyApp* app, char ch);
int Goto(MyApp* app, char ch);
int Del(MyApp* app, char ch);
int DelChar(MyApp* app, char ch);
	
//implementation of state functions

enum {
	SUCCESS,
	FAIL,
	CONTINUE,
};

int Start(MyApp* app, char ch) {
	int& num = app->cmdNum[0];
	const int& pos = app->pos;
	
	if(ch == '0' && num==0 ) {
		app->SendEditor(SCI_HOME);
		return SUCCESS;
	}

	if(readingCmdNum(app, ch, 0)) {
		return CONTINUE;	
	}

	if(num==0) {
		num = (ch == KEYCODE('G')) ? app->lineCount : num+1;
	}
	
	switch(ch) {
		/*	Home & End	*/
		case KEYCODE('$'):
			app->SendEditor(SCI_LINEEND);
			return SUCCESS;
			
		case KEYCODE('^'):
			app->SendEditor(SCI_VCHOME);
			return SUCCESS;
	
		/*	Goto	*/
		case KEYCODE('G'):
			return Goto(app, 'g');
		
		case KEYCODE('g'):
			return app->toState(Goto);
		
		/*	Move	*/
		case KEYCODE('b'):
		case KEYCODE('w'):
		case KEYCODE('h'):
		case KEYCODE('j'):
		case KEYCODE('k'):
		case KEYCODE('l'):
		{
			for(int i=0; i<num; ++i) {
				if(Move(app, ch) == FAIL) return FAIL;
			}
			return SUCCESS;
		}
		
		/*	Delete */
		case KEYCODE('x'): {
			for(int i=0; i<num; ++i) {
				if(DelChar(app, 'x') == FAIL) return FAIL;
			}
			return SUCCESS;
		}
		
		case KEYCODE('d'):
			return app->toState(Del);
			
		/*	Undo & Redo	*/
		case KEYCODE('u'): {
			ScopeTempAllowEdit stae(app);
			app->SendEditor(SCI_UNDO);
			return SUCCESS;
		}
		
		/*	Page Up/Down	*/
		case KEYCODE(CTRL + 'D'):
			app->SendEditor(SCI_PAGEDOWN);
			return SUCCESS;
			
		case KEYCODE(CTRL + 'U'):	
			app->SendEditor(SCI_PAGEUP);
			return SUCCESS;
	}
	
	return State::Norm(app, ch);
}

	
int Norm(MyApp* app, char ch) {
	int& num = app->cmdNum[0];
	
	switch(ch) {
		case KEYCODE('A'):
			app->SendEditor(SCI_LINEEND);
			app->toInsertMode();
			break;

		case KEYCODE('I'):
			app->SendEditor(SCI_HOME);
			app->toInsertMode();
			break;

		case KEYCODE('i'):
			app->toInsertMode();
			break;	
			
		case KEYCODE('u'):
			app->SendEditor(WM_UNDO);
			break;

		case KEYCODE('>'):
			app->SendEditor(SCI_TAB);
			break;
			
		case KEYCODE('<'):
			app->SendEditor(SCI_BACKTAB);
			break;
	}	//switch(ch)

	updateLineNumMargin(app);
	return SUCCESS;
}


int Move(MyApp* app, char ch) {
	int& num = app->cmdNum[0];
	
	const int& line = app->row;
	const int& col = app->col;
	const int& pos = app->pos;
	
	switch(ch) {
		case KEYCODE('b'):
			app->SendEditor(SCI_WORDLEFT);
			break;
			
		case KEYCODE('h'): {
			const int lineStartPos = app->SendEditor(SCI_POSITIONFROMLINE, line);
			if(pos <= lineStartPos) return FAIL;
			app->SendEditor(SCI_CHARLEFT);
		} break;
		
		case KEYCODE('j'): {
			if(line >= app->lineCount-1) return FAIL;
			app->SendEditor(SCI_LINEDOWN);
		} break;
			
		case KEYCODE('k'): 
			if(line == 0) return FAIL;
			app->SendEditor(SCI_LINEUP);
			break;
		
		case KEYCODE('l'): {
			const int lineEndPos = app->SendEditor(SCI_GETLINEENDPOSITION, line);
			if(pos >= lineEndPos) return FAIL;
			app->SendEditor(SCI_CHARRIGHT);
		} break;
		
		case KEYCODE('w'):
			app->SendEditor(SCI_WORDRIGHT);
			break;

	}	//switch(ch)

	return SUCCESS;
}


int Goto(MyApp* app, char ch) {
	int& num = app->cmdNum[0];

	switch(ch) {
		case KEYCODE('g'):
			if(app->SendEditor(SCI_GOTOLINE, num-1) == 0) return SUCCESS;
			
		default:
			return FAIL;
	}	//switch(ch)
	
}


int Del(MyApp* app, char ch) {
	ScopeTempAllowEdit stae(app);
	int& num = app->cmdNum[1];
	if(num==0) {
		switch(ch) {
			case KEYCODE('0'): 
			case KEYCODE('^'):
			{
				app->SendEditor(SCI_DELLINELEFT);		//todo: should be del to VCHOME
				return SUCCESS;
			}
			
			case KEYCODE('$'):
			{
				app->SendEditor(SCI_DELLINERIGHT);
				return SUCCESS;
			}			
		}	//switch(ch)

	}
	
	/*	read the delete cmd number	*/
	if(readingCmdNum(app, ch, 1)) {
		return CONTINUE;	
	}
	
	if(num==0) ++num;
	
	/*	delete operations	*/
	const int mnum = app->cmdNum[0] * app->cmdNum[1];
	for(int i=0; i<mnum; ++i) {
		switch(ch) {
			case KEYCODE('b'):
				app->SendEditor(SCI_DELWORDLEFT);
				break;
			
			case KEYCODE('d'):
				app->SendEditor(SCI_LINEDELETE);
				break;
				
			case KEYCODE('w'):
				app->SendEditor(SCI_DELWORDRIGHT);
				break;
		}	//switch(ch)
	}	//for
	
	app->SendEditor(SCI_SETREADONLY, true);
	return SUCCESS;
}


int DelChar(MyApp* app, char) {
	ScopeTempAllowEdit stae(app);
	if(Move(app, KEYCODE('l')) == FAIL) return FAIL;
	app->SendEditor(SCI_DELETEBACK);
	return SUCCESS;
}

}	//namespace State


#define DEFAULT_STATE State::Start

void MyApp::initState() {
	memset(cmdNum, 0, sizeof(cmdNum));
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
	static char tmpBuf[TEXT_BUF_SIZE];
	textbuf = tmpBuf;
}


MyApp::~MyApp() {
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
}




void MyApp::Notify(SCNotification *notification) {
	Base::Notify(notification);
	updateState(this);
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
	updateLineNumMargin(this);
}

void MyApp::InitialiseEditor() {
	Base::InitialiseEditor();
	SendEditor(SCI_SETMARGINWIDTHN, 0, LINENUM_MARGIN_BASE);	//shows line numbers
	//SendEditor(SCI_SETMARGINWIDTHN, 1, 30);					//shows symbols
	toDefaultMode();
}

char* MyApp::getBuf(int size) {
	if(size>TEXT_BUF_SIZE) return NULL;
	return textbuf;
}


void MyApp::trace(int i) {
  char msg[256]; sprintf(msg, "%i", i); trace(msg);
}


