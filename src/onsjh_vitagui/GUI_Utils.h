#ifndef __GUI_UTILS_H__
#define __GUI_UTILS_H__

#include <vita2d.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vitasdk.h>

using namespace std;

typedef struct point {
	int x;
	int y;
} point;

typedef struct rectangle {
	int left;
	int top;
	int right;
	int bottom;
} rectangle;


typedef struct configure {
	int ver;
	char *list_mode;
	int use_dpad;
	int use_btouch;
	int icon_row;
	int icon_col;
	int list_row;
} configure;



class RomInfo {
private:
	char* temp;
public:
	string path;
	string name;
	string last_date;
	string icon_path;
	vita2d_texture *icon;
	point pos;
	rectangle touch_area;
	uint64_t size;
	int w;
	int h;
	RomInfo() {
		touch_area = { 0,0,0,0 };
	}
	RomInfo(string path_) {
		RomInfo();
		path = path_;
		icon_path = path_ + "/icon.png";
		SceUID fd = sceIoOpen(char_icon_path(), SCE_O_RDONLY, 0777);
		if (fd > 0) 
		{
			icon = vita2d_load_PNG_file(char_icon_path());
			sceIoClose(fd);
		}
		else
		{
			icon = vita2d_load_PNG_file("app0:/sce_sys/icon1.png");
		}
			
		FILE *fp = fopen((path_ + "/caption.txt").c_str(), "r");
		if (fp)
		{
			char* chs = new char[256];
			fgets(chs, 512, fp);
			name = chs;
			free(chs);
			fclose(fp);
		}
		else
		{
			name = path_.substr(path_.find_last_of("/\\") + 1);
		}
			
		w = sceGxmTextureGetWidth(&icon->gxm_tex);
		h = sceGxmTextureGetHeight(&icon->gxm_tex);
	}
	static char *to_char(string str) {
		char* temp1 = new char[str.length() + 1];
		strcpy(temp1, str.c_str());
		return temp1;
	}
	static char *to_char(int num) {
		char* temp1 = new char[9];
		sprintf(temp1, "%d", num);
		return temp1;
	}
	char *char_path() {
		temp = new char[path.length() + 1];
		strcpy(temp, path.c_str());
		return temp;
	}
	char *char_name() {
		temp = new char[name.length() + 1];
		strcpy(temp, name.c_str());
		return temp;
	}
	char *char_last_date() {
		temp = new char[last_date.length() + 1];
		strcpy(temp, last_date.c_str());
		return temp;
	}
	char *char_icon_path() {
		temp = new char[icon_path.length() + 1];
		strcpy(temp, icon_path.c_str());
		return temp;
	}
};

extern vector<RomInfo> rom_list;
extern configure config;
extern vita2d_font* font;

void load_config();
void save_config();

void init_input();
void lock_psbutton();
void unlock_psbutton();
int read_buttons();
int read_touchscreen(point *p);

int load_rom_list();
int parseOption(string &cmdstr, int(&cmd)[10], char *cmd_str[10], int flag);
void sittings_file(string path, string &str, char mode, int nowrite = 0);

void convertUtcToLocalTime(SceDateTime *time_local, SceDateTime *time_utc);
void getDateString(char string[24], int date_format, SceDateTime *time);
void getTimeString(char string[16], int time_format, SceDateTime *time);
#endif // __GUI_UTILS_H__
