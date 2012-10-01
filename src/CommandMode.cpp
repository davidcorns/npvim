#include <include/Scintilla.h>

#include "MyApp.h"
#include "AppInfo.h"

#include "CommandMode.h"
#include "StateEnum.h"


#define DEFAULT_STATE State::Start

CommandMode::CommandMode(MyApp& app): Base(app) {
}


CommandMode::~CommandMode() {

}


void CommandMode::Notify(SCNotification* notification) {
	switch(notification->nmhdr.code) {
		case SCN_CHARADDED:
			const int res = state(app, notification->ch);
			switch(res) {
				case State::SUCCESS:
				case State::FAIL:
					this->initState();
					break;
			}
			break;
	}
}

void CommandMode::init() {
	app.SendEditor(SCI_SETREADONLY, true);	
	this->initState();
}


void CommandMode::initState() {
	memset(app.info->cmdNum, 0, sizeof(app.info->cmdNum));
	toState(DEFAULT_STATE);
}

int CommandMode::toState(State::Func s) {
	state = s;
	return State::CONTINUE;
}
