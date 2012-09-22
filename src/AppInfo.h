#ifndef __APP_INFO_H__
#define __APP_INFO_H__

/*	D-Pointer class of MyApp	*/
struct AppInfo {
	int row;
	int col;
	int pos;
	int lineCount;
	int lineLen;
	int cmdNum[NUM_BUF_SIZE];
};


#endif	//__APP_INFO_H__
