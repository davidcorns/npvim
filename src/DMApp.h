#ifndef __DMApp_h__
#define __DMApp_h__

#include <include/Scintilla.h>
#include <include/SciLexer.h>

#include <windows.h>
#include <richedit.h>

#include <constants.h>

struct DMApp {
	HINSTANCE hInstance;
	HWND currentDialog;
	HWND wMain;
	HWND wEditor;
	bool isDirty;
	char fullPath[MAX_PATH];
	

	DMApp();

	inline LRESULT SendEditor(UINT Msg, WPARAM wParam=0, LPARAM lParam=0) {
		return ::SendMessage(wEditor, Msg, wParam, lParam);
	}

	void GetRange(int start, int end, char *text);

	void SetTitle();
	void New();
	void Open();
	void OpenFile(const char *fileName);
	void Save();
	void SaveAs();
	void SaveFile(const char *fileName);
	int SaveIfUnsure();

	void Notify(SCNotification *notification);
	void Command(int id);
	void EnableAMenuItem(int id, bool enable);
	void CheckMenus();

	void SetAStyle(int style, COLORREF fore, COLORREF back=white, int size=-1, const char *face=0);
	void InitialiseEditor();
	

};


#endif	//__DMApp_h__
