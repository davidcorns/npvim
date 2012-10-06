#include <include/Scintilla.h>

#include <MyApp.h>
#include <AppInfo.h>

#include <InsertMode.h>
#include <util/App.h>

InsertMode::InsertMode(MyApp& app) : Base(app) {

}


InsertMode::~InsertMode() {

}

void InsertMode::Notify(SCNotification *notification) {
	switch (notification->nmhdr.code) {
		case SCN_CHARADDED: {
			const char& ch = notification->ch;	
				
			switch(ch) {
				case '\n': {
					//auto indent
					const int lineNum = util::getEndLine(app);
					char* txt = new char[app.info->lineLen];
					app.SendEditor(SCI_GETLINE, lineNum, reinterpret_cast<LPARAM>(txt));
					char* p = txt;
					while(*p==' ' || *p=='\t') {
						++p;	
					}
					if(*p != '\0' && *p!='\r' && *p!='\n' && p!=txt) {
						app.SendEditor(SCI_ADDTEXT, p-txt, reinterpret_cast<LPARAM>(txt));
					}
					if(txt) delete txt;
				} break;
			}
		} break;
	}	//switch
	
}

void InsertMode::init() {
	app.SendEditor(SCI_SETREADONLY, false);
}
