#include <DMApp.h>
#include <stdio.h>

#include <resource.h>


DMApp::DMApp() {
	hInstance = 0;
	currentDialog = 0;
	wMain = 0;
	wEditor = 0;
	isDirty = false;
	fullPath[0] = '\0';
	

}

void DMApp::GetRange(int start, int end, char *text) {
	TEXTRANGE tr;
	tr.chrg.cpMin = start;
	tr.chrg.cpMax = end;
	tr.lpstrText = text;
	SendMessage(wEditor, EM_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));
}

void DMApp::SetTitle() {
	char title[MAX_PATH + 100];
	strcpy(title, appName);
	strcat(title, " - ");
	strcat(title, fullPath);
	::SetWindowText(wMain, title);
}

void DMApp::New() {
	SendEditor(SCI_CLEARALL);
	SendEditor(EM_EMPTYUNDOBUFFER);
	fullPath[0] = '\0';
	SetTitle();
	isDirty = false;
	SendEditor(SCI_SETSAVEPOINT);
}

void DMApp::OpenFile(const char *fileName) {
	New();
	SendEditor(SCI_CANCEL);
	SendEditor(SCI_SETUNDOCOLLECTION, 0);

	strcpy(fullPath, fileName);
	FILE *fp = fopen(fullPath, "rb");
	if (fp) {
		SetTitle();
		char data[blockSize];
		int lenFile = fread(data, 1, sizeof(data), fp);
		while (lenFile > 0) {
			SendEditor(SCI_ADDTEXT, lenFile,
					   reinterpret_cast<LPARAM>(static_cast<char *>(data)));
			lenFile = fread(data, 1, sizeof(data), fp);
		}
		fclose(fp);
	} else {
		char msg[MAX_PATH + 100];
		strcpy(msg, "Could not open file \"");
		strcat(msg, fullPath);
		strcat(msg, "\".");
		::MessageBox(wMain, msg, appName, MB_OK);
	}
	SendEditor(SCI_SETUNDOCOLLECTION, 1);
	::SetFocus(wEditor);
	SendEditor(EM_EMPTYUNDOBUFFER);
	SendEditor(SCI_SETSAVEPOINT);
	SendEditor(SCI_GOTOPOS, 0);
}

void DMApp::Open() {
	char openName[MAX_PATH] = "\0";
	OPENFILENAME ofn = {sizeof(OPENFILENAME)};
	ofn.hwndOwner = wMain;
	ofn.hInstance = hInstance;
	ofn.lpstrFile = openName;
	ofn.nMaxFile = sizeof(openName);
	char *filter = 
		"C++ (.cpp;.c;.cxx;.h;.hpp)\0*.cpp;*.c;*.cxx;*.h;*.hpp\0"
		"Web (.html;.htm;.asp;.shtml;.css;.xml)\0*.html;*.htm;*.asp;*.shtml;*.css;*.xml\0"
		"All Files (*.*)\0*.*\0\0";

	ofn.lpstrFilter = filter;
	ofn.lpstrCustomFilter = 0;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrTitle = "Open File";
	ofn.Flags = OFN_HIDEREADONLY;

	if (::GetOpenFileName(&ofn)) {
		OpenFile(openName);
	}
}

void DMApp::Save() {
	if(fullPath[0] == '\0')
    	SaveAs();
	else
		SaveFile(fullPath);
}

void DMApp::SaveAs() {
	char openName[MAX_PATH] = "\0";
	strcpy(openName, fullPath);
	OPENFILENAME ofn = {sizeof(ofn)};
	ofn.hwndOwner = wMain;
	ofn.hInstance = hInstance;
	ofn.lpstrFile = openName;
	ofn.nMaxFile = sizeof(openName);
	ofn.lpstrTitle = "Save File";
	ofn.Flags = OFN_HIDEREADONLY;

	if (::GetSaveFileName(&ofn)) {
		strcpy(fullPath, openName);
		SetTitle();
		SaveFile(fullPath);
		::InvalidateRect(wEditor, 0, 0);
	}
}

void DMApp::SaveFile(const char *fileName) {
	FILE *fp = fopen(fullPath, "wb");
	if (fp) {
		char data[blockSize + 1];
		int lengthDoc = SendEditor(SCI_GETLENGTH);
		for (int i = 0; i < lengthDoc; i += blockSize) {
			int grabSize = lengthDoc - i;
			if (grabSize > blockSize)
				grabSize = blockSize;
			GetRange(i, i + grabSize, data);
			fwrite(data, grabSize, 1, fp);
		}
		fclose(fp);
		SendEditor(SCI_SETSAVEPOINT);
	} else {
		char msg[MAX_PATH + 100];
		strcpy(msg, "Could not save file \"");
		strcat(msg, fullPath);
		strcat(msg, "\".");
		MessageBox(wMain, msg, appName, MB_OK);
	}
}

int DMApp::SaveIfUnsure() {
	if (isDirty) {
		char msg[MAX_PATH + 100];
		strcpy(msg, "Save changes to \"");
		strcat(msg, fullPath);
		strcat(msg, "\"?");
		int decision = MessageBox(wMain, msg, appName, MB_YESNOCANCEL);
		if (decision == IDYES) {
			Save();
		}
		return decision;
	}
	return IDYES;
}


void DMApp::Command(int id) {
	switch (id) {
	case IDM_FILE_NEW:
		if (SaveIfUnsure() != IDCANCEL) {
			New();
		}
		break;
	case IDM_FILE_OPEN:
		if (SaveIfUnsure() != IDCANCEL) {
			Open();
		}
		break;
	case IDM_FILE_SAVE:
		Save();
		break;
	case IDM_FILE_SAVEAS:
		SaveAs();
		break;
	case IDM_FILE_EXIT:
		if (SaveIfUnsure() != IDCANCEL) {
			::PostQuitMessage(0);
		}
		break;

	case IDM_EDIT_UNDO:
		SendEditor(WM_UNDO);
		break;
	case IDM_EDIT_REDO:
		SendEditor(SCI_REDO);
		break;
	case IDM_EDIT_CUT:
		SendEditor(WM_CUT);
		break;
	case IDM_EDIT_COPY:
		SendEditor(WM_COPY);
		break;
	case IDM_EDIT_PASTE:
		SendEditor(WM_PASTE);
		break;
	case IDM_EDIT_DELETE:
		SendEditor(WM_CLEAR);
		break;
	case IDM_EDIT_SELECTALL:
		SendEditor(SCI_SELECTALL);
		break;
	};
}

void DMApp::EnableAMenuItem(int id, bool enable) {
	if (enable)
		::EnableMenuItem(::GetMenu(wMain), id, MF_ENABLED | MF_BYCOMMAND);
	else
		::EnableMenuItem(::GetMenu(wMain), id, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND);
}

void DMApp::CheckMenus() {
	EnableAMenuItem(IDM_FILE_SAVE, isDirty);
	EnableAMenuItem(IDM_EDIT_UNDO, SendEditor(EM_CANUNDO));
	EnableAMenuItem(IDM_EDIT_REDO, SendEditor(SCI_CANREDO));
	EnableAMenuItem(IDM_EDIT_PASTE, SendEditor(EM_CANPASTE));
}

void DMApp::Notify(SCNotification *notification) {

	switch (notification->nmhdr.code) {
		case SCN_SAVEPOINTREACHED:
			isDirty = false;
			CheckMenus();
			break;
	
		case SCN_SAVEPOINTLEFT:
			isDirty = true;
			CheckMenus();
			break;
	}
}

void DMApp::SetAStyle(int style, COLORREF fore, COLORREF back, int size, const char *face) {
	SendEditor(SCI_STYLESETFORE, style, fore);
	SendEditor(SCI_STYLESETBACK, style, back);
	if (size >= 1)
		SendEditor(SCI_STYLESETSIZE, style, size);
	if (face) 
		SendEditor(SCI_STYLESETFONT, style, reinterpret_cast<LPARAM>(face));
}


void DMApp::InitialiseEditor() {
	SendEditor(SCI_SETLEXER, SCLEX_HTML);
	SendEditor(SCI_SETSTYLEBITS, 7);


	// Set up the global default style. These attributes are used wherever no explicit choices are made.
	SetAStyle(STYLE_DEFAULT, black, white, 12, "Courier New");
	SendEditor(SCI_STYLECLEARALL);	// Copies global style to all others
	
}


