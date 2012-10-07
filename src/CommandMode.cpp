#include <include/Scintilla.h>
#include <stack>

#include <MyApp.h>
#include <AppInfo.h>

#include <CommandMode.h>
#include <util/app.h>
#include <util/math.h>

#include <util/CycleStack.h>
#include <cstdio>	//for trace

/////////////////////////////////////////////////////////////////////////////////////////////////
/*	CommandMode::StateView	*/

namespace State {
	
//forward declaration
class StateView;

//enum
enum StateReturn{
	SUCCESS,
	FAIL,
	CONTINUE,
};

//typedef
typedef StateReturn(*StateFunc)(StateView&,char);
typedef StateReturn(*StateWrapper)(StateFunc&,StateView&,char);
typedef unsigned int UnsignNum;



////////////////////////////////////////////////////
/*	State StateFunc declaration	*/
#define DECLARE_STATE_FUNC(func)	StateReturn func(StateView&, char);

//GroupState
DECLARE_STATE_FUNC(Move);
DECLARE_STATE_FUNC(Edit);
DECLARE_STATE_FUNC(Insert);
DECLARE_STATE_FUNC(Undo);

//Command Mode State that can be toState
DECLARE_STATE_FUNC(Start);
DECLARE_STATE_FUNC(Goto);
DECLARE_STATE_FUNC(Find);
DECLARE_STATE_FUNC(FindBack);
DECLARE_STATE_FUNC(Till);
DECLARE_STATE_FUNC(TillBack);

////////////////////////////////////////////////////
/*	Sel StateFunc declaration	*/
#define DECLARE_STATE_WRAPPER(func)	StateReturn func(StateFunc&,StateView&,char);

DECLARE_STATE_WRAPPER(NotWrapper);
DECLARE_STATE_WRAPPER(DelWrapper);


////////////////////////////////////////////////////
/*	StateView declaration & defination	*/
class StateView {
	StateFunc curState;		//the function pointer which the function is to perform actoin base on the char input
	StateWrapper curWrapper;	//the function pointer which the function is for muliplate the selection
	MyApp& app;
	util::CycleStack<int> posStack;
	
public:
	struct NumCache {
		UnsignNum 	tmp,
					edit;
	} num;
	
	const AppInfo& info;

public:
	StateView(MyApp& a);
	void init();
	
	void initNum() {
		num.tmp = 0;
		num.edit = 1;
	}
	
	inline UnsignNum getNum() {
		return num.edit * num.tmp;
	}
	
	inline bool toInsertMode() {
		app.toInsertMode();
		return true;
	}

	inline LRESULT execute(UINT Msg, WPARAM wParam=0, LPARAM lParam=0) {
		return ::SendMessage(app.wEditor, Msg, wParam, lParam);
	}

	inline StateReturn toState(State::StateFunc s) { 
		this->curState = s; 
		return CONTINUE;
	}
	
	inline StateReturn toWrapper(State::StateWrapper p) {
		this->curWrapper = p;
		return CONTINUE;
	}
	
	//TODO: will be removed
	inline void trace(char* msg) {::MessageBox(app.wMain, msg, msg, MB_OK);	}
	void trace(int i) {	
		char msg[256]; sprintf(msg, "%i", i); trace(msg);
	}
	
};	//class CommandMode::StateView


StateView::StateView(MyApp& a): 
	app(a),
	info(a.getInfo())
{
	init();
}

void StateView::init() {
	initNum();
	toState(Start);
	toWrapper(NotWrapper);
}

////////////////////////////////////////////////////
/*	helper functions and classes	*/

class ScopeTempAllowEdit {
	StateView& sv;
public:
	ScopeTempAllowEdit(StateView& view) : sv(view) { sv.execute(SCI_SETREADONLY, false);	}
	~ScopeTempAllowEdit() { sv.execute(SCI_SETREADONLY, true); }
};


////////////////////////////////////////////////////
/*	StateFunc defination	*/

#define KEYCODE(x)	x
#define LOOP(num)	for(int _i=0;_i<num;++_i)

StateReturn Start(StateView& view,char ch) {
	//reading number
	UnsignNum& num = view.num.tmp;
	
	if(ch == '0' && num==0 ) {
		view.execute(SCI_HOME);
		return SUCCESS;
	}
	
	if(util::isDigit(ch)) {
		num *= 10;
		num += ch-'0';
		return CONTINUE;
	}
	if(num == 0) ++num;
	
	StateReturn st;
	st = Insert(view, ch);				if(st != FAIL) return st;
	st = Move(view, ch);				if(st != FAIL) return st;
	st = Edit(view, ch);				if(st != FAIL) return st;
	st = Undo(view, ch);				if(st != FAIL) return st;
	
	return FAIL;
}

StateReturn Move(StateView& view,char ch) {
	const int lineStartPos = view.execute(SCI_POSITIONFROMLINE, view.info.row);
	const int num = view.getNum();
	
	switch(ch) {
		/*	Home & End	*/
		case KEYCODE('0'):
			view.execute(SCI_HOME);
			break;
		
		case KEYCODE('$'):
			view.execute(SCI_LINEEND);
			if(view.info.pos > lineStartPos) view.execute(SCI_CHARLEFT);
			break;
			
		case KEYCODE('^'):
			view.execute(SCI_VCHOME);
			break;

		/*	Word Jump	*/
		case KEYCODE('b'):
			LOOP(num) {
				view.execute(SCI_WORDLEFT);
			}
			break;
			
		case KEYCODE('e'):
			view.execute(SCI_CLEARSELECTIONS);
			break;
			
		case KEYCODE('w'):
			LOOP(num) {
				view.execute(SCI_WORDRIGHT);
			}
			break;
			
		/*	Arrow Move	*/
		case KEYCODE('h'):
			LOOP(num) {
				if(view.info.pos <= lineStartPos) return FAIL;
				view.execute(SCI_CHARLEFT);
			}
			break;
		
		case KEYCODE('j'): {
			LOOP(num) {
				if(view.info.row >= view.info.lineCount-1) return FAIL;
				view.execute(SCI_LINEDOWN);
			}
		} break;
			
		case KEYCODE('k'): 
			LOOP(num) {
				if(view.info.row == 0) return FAIL;
				view.execute(SCI_LINEUP);
			}
			break;
		
		case KEYCODE('l'): {
			//the vi cursor cannot move over the line end, thus -1
			const int lineEndPos = view.execute(SCI_GETLINEENDPOSITION, view.info.row) - 1;	
			LOOP(num) {
				if(view.info.pos >= lineEndPos) return FAIL;
				view.execute(SCI_CHARRIGHT);
			}
		} break;
		
		/*	Goto	*/
		case KEYCODE('G'):
			return Goto(view, ch);
			
		case KEYCODE('g'):
			return view.toState(Goto);

		/*	Find & Till	*/
		case KEYCODE('F'):
			return view.toState(FindBack);

		case KEYCODE('f'): 
			return view.toState(Find);

		case KEYCODE('T'):
			return view.toState(TillBack);

		case KEYCODE('t'):
			return view.toState(Till);

		default:
			return FAIL;
	}	//switch(ch)
	
	return SUCCESS;
}


StateReturn Goto(StateView& view, char ch) {
	switch(ch) {
		case KEYCODE('G'):
		case KEYCODE('g'):
			if(view.execute(SCI_GOTOLINE, view.getNum()-1) == 0)
				return SUCCESS;

		default:
			return FAIL;
	}	//switch(ch)
}

//@param MaxMsg		the SCI Command message to get the Sci_TextToFind.chrg.cpMax
//@param direction	the direction of finding, +1 means to right and -1 means to left
//@param delta		the delta to the result position of finding
StateReturn _Find(StateView& view, char ch, UINT MaxMsg, short direction, short delta) {
	const char str[] = {ch, '\0'};

	Sci_TextToFind ttf;
	ttf.chrg.cpMax = view.execute(MaxMsg, view.info.row);
	ttf.lpstrText = const_cast<char*>(str);
	
	const UnsignNum num = view.getNum();
	int pos = view.info.pos;
	for(int i=0; i<num; ++i) {
		ttf.chrg.cpMin = pos + direction;
		pos = view.execute(SCI_FINDTEXT, SCFIND_MATCHCASE, (LPARAM)&ttf);
		if( pos == -1 ) return FAIL;
	}

	view.execute(SCI_GOTOPOS, pos + delta);
	return SUCCESS;
} 


StateReturn Find(StateView& view, char ch) {
	return _Find(view, ch, SCI_GETLINEENDPOSITION, 1, 0);
}

StateReturn FindBack(StateView& view, char ch) {
	return _Find(view, ch, SCI_POSITIONFROMLINE, -1, 0);
}


StateReturn Till(StateView& view, char ch) {
	return _Find(view, ch, SCI_GETLINEENDPOSITION, 1, -1);
}


StateReturn TillBack(StateView& view, char ch) {
	return _Find(view, ch, SCI_POSITIONFROMLINE, -1, +1);
}

StateReturn Insert(StateView& view, char ch) {
	int row = view.info.row;
	
	switch(ch) {	
		case KEYCODE('A'):
			view.execute(SCI_LINEEND);
			break;

		case KEYCODE('I'):
			view.execute(SCI_HOME);
			break;

		case KEYCODE('i'):
			break;
			
			/*	New Line	*/
		case KEYCODE('O'):
			if(--row < 0) row = 0;
			view.execute(SCI_GOTOLINE, row);
		case KEYCODE('o'): {
			ScopeTempAllowEdit stae(view);
			view.execute(SCI_LINEEND);
			view.execute(SCI_NEWLINE);
			break;
		}
			
		default:
			return FAIL;
	}	//witch(ch)
	
	return view.toInsertMode() ? SUCCESS : FAIL;
}

StateReturn Edit(StateView& view, char ch) {
	switch(ch) {
		case KEYCODE('d'):
			view.toWrapper(DelWrapper);
			break;
			
		default:
			return FAIL;
	}
	
	view.num.edit = view.num.tmp;
	view.num.tmp = 0;
	return CONTINUE;
}


StateReturn Undo(StateView& view, char ch) {
	if(ch != KEYCODE('u')) return FAIL;
	ScopeTempAllowEdit stae(view);
	for(int i=0; i<view.getNum(); ++i) {
		view.execute(SCI_UNDO);
	}
	return SUCCESS;
}

////////////////////////////////////////////////////
/*	State Wrapper defination	*/
StateReturn NotWrapper(StateFunc& state, StateView& view, char ch) {
	return state(view, ch);
}

StateReturn DelWrapper(StateFunc& state, StateView& view, char ch) {
	
	return state(view, ch);
}

}	//namespace State

/////////////////////////////////////////////////////////////////////////////////////////////////
/*	Command Mode implmentation	*/

CommandMode::CommandMode(MyApp& app): Base(app) {
	sv = new State::StateView(app);
}


CommandMode::~CommandMode() {
	if(sv) delete sv;
}


void CommandMode::Notify(SCNotification* notification) {
	if(notification->nmhdr.code != SCN_CHARADDED) return;
	
	//push the pos to stack
	const int& pos = app.getInfo().pos;
	if(pos != sv->posStack.top()) sv->posStack.push(pos);
	
	//execute the state func
	State::StateReturn res = sv->curWrapper(sv->curState, *sv, notification->ch);
	switch(res) {
		case State::SUCCESS:
		case State::FAIL:
			sv->init();
	}
}

void CommandMode::init() {
	app.SendEditor(SCI_SETREADONLY, true);	
}


