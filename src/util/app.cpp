#include <util/app.h>
#include <DMApp.h>

namespace util {

int getCurLine(DMApp& app) {
	const int startPos = app.SendEditor(SCI_GETCURRENTPOS);
	return app.SendEditor(SCI_LINEFROMPOSITION, startPos);
}

int getCurCol(DMApp& app) {
	const int pos = app.SendEditor(SCI_GETCURRENTPOS);
	return app.SendEditor(SCI_GETCOLUMN, pos);
}

/*	used by auto indent	*/
int getEndLine(DMApp& app) {
	const int startPos = app.SendEditor(SCI_GETENDSTYLED);
	return app.SendEditor(SCI_LINEFROMPOSITION, startPos);	
}


}	//namespace util
