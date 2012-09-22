#include <cstdio>
#include <math.h>
#include <resource.h>

#include <MyApp.h>
#include <util.h>
#include <KeyMap.h>

#define KEYCODE(x)	x
#define LINENUM_MARGIN_BASE 12




/////////////////////////////////////////////////////////////////////////////////////////////////
/*	D-Pointer class of MyApp	*/
struct MyApp::Info {
	int row;
	int col;
	int pos;
	int lineCount;
	int lineLen;
	int cmdNum[NUM_BUF_SIZE];
};

static const char EMPTY_STRING[] = "";


/*	helper functions of MyApp	*/

void updateLineNumMargin(MyApp* app) {
	//line number margin auto adjust
	const int margin = int(1 + log10(app->info->lineCount)) * LINENUM_MARGIN_BASE;
	app->SendEditor(SCI_SETMARGINWIDTHN, 0, margin);
}

void updateInfo(MyApp* app) {
	MyApp::Info* info = app->info;
	info->row = util::getCurLine(app);
	info->col = util::getCurCol(app);
	info->pos = app->SendEditor(SCI_GETCURRENTPOS);
	info->lineCount = app->SendEditor(EM_GETLINECOUNT);
	info->lineLen = app->SendEditor(SCI_LINELENGTH);
}


inline int getLineEndPosition(MyApp* app, int line) {
	return app->SendEditor(SCI_GETLINEENDPOSITION, line);	
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
/*	State	*/
namespace State {

//helper functions and classes
bool readingCmdNum(MyApp* app, char ch, int index) {
	int& num = app->info->cmdNum[index];
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
int Start(MyApp* app, char ch);
int Insert(MyApp* app, char ch);
int Move(MyApp* app, char ch);
int Goto(MyApp* app, char ch);
int Del(MyApp* app, char ch);
int DelChar(MyApp* app, char ch);
int Find(MyApp* app, char ch);
int FindBack(MyApp* app, char ch);
int Till(MyApp* app, char ch);
int TillBack(MyApp* app, char ch);
int DelFind(MyApp* app, char ch);
int DelFindBack(MyApp* app, char ch);
int DelTill(MyApp* app, char ch);
int DelTillBack(MyApp* app, char ch);
int Replace(MyApp* app, char ch);
	
//implementation of state functions

enum {
	SUCCESS,
	FAIL,
	CONTINUE,
};

int Start(MyApp* app, char ch) {
	int& num = app->info->cmdNum[0];
	const int& pos = app->info->pos;
	
	if(ch == '0' && num==0 ) {
		app->SendEditor(SCI_HOME);
		return SUCCESS;
	}

	if(readingCmdNum(app, ch, 0)) {
		return CONTINUE;	
	}

	if(num==0) {
		num = (ch == KEYCODE('G')) ? app->info->lineCount : num+1;
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
		case KEYCODE('e'):
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
		case KEYCODE('x'): 
		{
			for(int i=0; i<num; ++i) {
				if(DelChar(app, 'x') == FAIL) return FAIL;
			}
			return SUCCESS;
		}
		
		case KEYCODE('d'):
			return app->toState(Del);
			
		/*	Undo & Redo	*/
		case KEYCODE('u'): 
		{
			ScopeTempAllowEdit stae(app);
			for(int i=0; i<num; ++i) {
				app->SendEditor(SCI_UNDO);
			}
			return SUCCESS;
		}
		
		/*	Find & Till	*/
		case KEYCODE('F'):
			return app->toState(FindBack);
		case KEYCODE('f'): 
			return app->toState(Find);
		case KEYCODE('T'):
			return app->toState(TillBack);
		case KEYCODE('t'):
			return app->toState(Till);
		
		/*	Replace	*/
		case KEYCODE('r'):
			app->SendEditor(SCI_SETCURRENTPOS, app->info->pos+1);
			return app->toState(Replace);
		
		default:
			return Insert(app, ch);
	}
}


int Insert(MyApp* app, char ch) {
	int& num = app->info->cmdNum[0];
	int& row = app->info->row;

	switch(ch) {
		/*	to insert mode	*/			
		case KEYCODE('A'):
			app->SendEditor(SCI_LINEEND);
			app->toInsertMode();
			return SUCCESS;

		case KEYCODE('I'):
			app->SendEditor(SCI_HOME);
			app->toInsertMode();
			return SUCCESS;

		case KEYCODE('i'):
			app->toInsertMode();
			return SUCCESS;
			
		/*	New Line	*/
		case KEYCODE('O'):
			if(--row < 0) row = 0;
			app->SendEditor(SCI_GOTOLINE, row);
		case KEYCODE('o'):
			app->toInsertMode();
			app->SendEditor(SCI_LINEEND);
			app->SendEditor(SCI_NEWLINE);
			return SUCCESS;		
	}	//witch(ch)
}

	


int Move(MyApp* app, char ch) {
	int& num = app->info->cmdNum[0];
	
	const int& line = app->info->row;
	const int& col = app->info->col;
	const int& pos = app->info->pos;
	
	switch(ch) {
		case KEYCODE('b'):
			app->SendEditor(SCI_WORDLEFT);
			break;
			
		case KEYCODE('e'):
			//app->SendEditor(SCI_WORDPARTRIGHT);
			break;
			
		case KEYCODE('h'): {
			const int lineStartPos = app->SendEditor(SCI_POSITIONFROMLINE, line);
			if(pos <= lineStartPos) return FAIL;
			app->SendEditor(SCI_CHARLEFT);
		} break;
		
		case KEYCODE('j'): {
			if(line >= app->info->lineCount-1) return FAIL;
			app->SendEditor(SCI_LINEDOWN);
		} break;
			
		case KEYCODE('k'): 
			if(line == 0) return FAIL;
			app->SendEditor(SCI_LINEUP);
			break;
		
		case KEYCODE('l'): {
			const int lineEndPos = getLineEndPosition(app, line);
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
	int& num = app->info->cmdNum[0];

	switch(ch) {
		case KEYCODE('g'):
			if(app->SendEditor(SCI_GOTOLINE, num-1) == 0) return SUCCESS;
			
		default:
			return FAIL;
	}	//switch(ch)
	
}


int Del(MyApp* app, char ch) {
	ScopeTempAllowEdit stae(app);
	int& num = app->info->cmdNum[1];
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
	
	/*	delete for Find & Till	*/
	switch(ch) {
		case KEYCODE('F'):
			return app->toState(DelFindBack);
		case KEYCODE('f'): 
			return app->toState(DelFind);
		case KEYCODE('T'):
			return app->toState(DelTillBack);
		case KEYCODE('t'):
			return app->toState(DelTill);
	}	//switch(ch) 
	
	
	/*	delete operations	*/
	const int mnum = app->info->cmdNum[0] * app->info->cmdNum[1];
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
	
	return SUCCESS;
}


int DelChar(MyApp* app, char) {
	ScopeTempAllowEdit stae(app);
	if(Move(app, KEYCODE('l')) == FAIL) return FAIL;
	app->SendEditor(SCI_DELETEBACK);
	return SUCCESS;
}

int _Find(MyApp* app, char ch, UINT Msg, int num, short delta) {
	const char str[] = {ch, '\0'};

	Sci_TextToFind ttf;
	ttf.chrg.cpMax = getLineEndPosition(app, app->info->row);
	ttf.lpstrText = const_cast<char*>(str);
	
	int pos = app->info->pos;
	for(int i=0; i<num; ++i) {
		ttf.chrg.cpMin = pos + 1;
		pos = app->SendEditor(SCI_FINDTEXT, SCFIND_MATCHCASE, (LPARAM)&ttf);
		if( pos == -1 ) return FAIL;
	}

	app->SendEditor(Msg, pos + delta);
	return SUCCESS;
} 

int _FindBack(MyApp* app, char ch, UINT Msg, int num, int delta) {
	const char str[] = {ch, '\0'};

	Sci_TextToFind ttf;
	ttf.chrg.cpMax = app->SendEditor(SCI_POSITIONFROMLINE, app->info->row);
	ttf.lpstrText = const_cast<char*>(str);
	
	int pos = app->info->pos;
	for(int i=0; i<num; ++i) {
		ttf.chrg.cpMin = pos - 1;
		pos = app->SendEditor(SCI_FINDTEXT, SCFIND_MATCHCASE, (LPARAM)&ttf);
		if( pos == -1 ) return FAIL;
	}

	app->SendEditor(Msg, pos + delta);
	return SUCCESS;	
}


int Find(MyApp* app, char ch) {
	return _Find(app, ch, SCI_GOTOPOS, app->info->cmdNum[0], 0);
}

int FindBack(MyApp* app, char ch) {
	return _FindBack(app, ch, SCI_GOTOPOS, app->info->cmdNum[0], 0);
}


int Till(MyApp* app, char ch) {
	return _Find(app, ch, SCI_GOTOPOS, app->info->cmdNum[0], -1);
}


int TillBack(MyApp* app, char ch) {
	return _FindBack(app, ch, SCI_GOTOPOS, app->info->cmdNum[0], +1);
}


int DelFind(MyApp* app, char ch) {
	int res;
	const int num = app->info->cmdNum[0] * app->info->cmdNum[1];
	
	ScopeTempAllowEdit stae(app);
	res = _Find(app, ch, SCI_SETCURRENTPOS, num, 1);			if(res != SUCCESS) return res;
	res = app->SendEditor(SCI_CUT);								if(res <= 0) return FAIL;
	return SUCCESS;
}

int DelFindBack(MyApp* app, char ch) {
	int res;
	const int num = app->info->cmdNum[0] * app->info->cmdNum[1];
	
	ScopeTempAllowEdit stae(app);
	res = _FindBack(app, ch, SCI_SETCURRENTPOS, num, 0);		if(res != SUCCESS) return res;
	res = app->SendEditor(SCI_CUT);								if(res <= 0) return FAIL;
	return SUCCESS;
}

int DelTill(MyApp* app, char ch) {
	int res;
	const int num = app->info->cmdNum[0] * app->info->cmdNum[1];
	
	ScopeTempAllowEdit stae(app);
	res = _Find(app, ch, SCI_SETCURRENTPOS, num, 0);			if(res != SUCCESS) return res;
	res = app->SendEditor(SCI_CUT);								if(res <= 0) return FAIL;
	return SUCCESS;
}

int DelTillBack(MyApp* app, char ch) {
	int res;
	const int num = app->info->cmdNum[0] * app->info->cmdNum[1];
	
	ScopeTempAllowEdit stae(app);
	res = _FindBack(app, ch, SCI_SETCURRENTPOS, num, +1);		if(res != SUCCESS) return res;
	res = app->SendEditor(SCI_CUT);								if(res <= 0) return FAIL;
	return SUCCESS;
}

int Replace(MyApp* app, char ch) {
	ScopeTempAllowEdit stae(app);
	const char str[] = { ch, '\0' };
	app->SendEditor(SCI_REPLACESEL, NULL, (LPARAM)str);
	return SUCCESS;
}

}	//namespace State


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
	info = new Info();
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


