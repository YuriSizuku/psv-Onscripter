/* -*- C++ -*-
 *
 *  onscripter_main.cpp -- main function of ONScripter
 *
 *  Copyright (c) 2001-2017 Ogapee. All rights reserved.
 *            (C) 2014-2017 jh10001 <jh10001@live.cn>
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h> 
#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>
#include <string>
#include <vector>
#include <iostream>

#include "vitaPackage.h"
#include "filesystem.h"

#include "GUI_common.h"
#include "GUI_Utils.h"
#include "version.h"
#include "iniparser.h"

using namespace std;
// must use app0:xxx.bin
#define ONSJH_PATH "app0:onsjh.bin"

extern unsigned char _binary_res_logo_png_start;
extern unsigned char _binary_res_logo_png_size;

int _newlib_heap_size_user = 192 * 1024 * 1024;

extern int ons_main(int argc, char *argv[]);
//-----------------------------------
int select_row = 0;
int select_col = 0;
int select_appinfo_button = 0;
int select_slot = 0;
int select_config = 0;
vita2d_font* font;
const char* help_msg= "\
使用帮助(游戏内)\n\n\
○　　　　确认/继续\n\
╳　　　　按住快进\n\
□　　　　自动模式\n\
△　　　　菜单/关闭回想模式\n\
Ｌ　　　　快进当前页\n\
Ｒ　　　　开启/停止快进\n\
←→　　　回想模式选择\n\
↑↓　　　选项/按钮选择\n\
左摇杆　　等同方向键\n";

const char* about_msg ="\
　　　关于ONS for PSV\n\n\
ONScripter 　　　<Ogapee>\n\
ONScripter-jh　　<jh10001>\n\
vita-savemgr 　　<d3m3vilurr>\n\
ONS-jh-PSV 　　 <wetor>\n\n\
　　　wetor(@依旧W如此)\n\
　　<2929339419@qq.com>\n\
　　　 http://maho.wang\n\
　　　 请不要吐槽界面...\n\n\
maintained by Yurisiziku, \n\
https://github.com/YuriSizuku/psv-Onscripter\n";

int game_start_select = -1;
string startup_cmd;
int cmd_default[] = { 0,1,0,1,0,0,0,0,0,0 };
int cmd[10] = {0};
int cmd_num = 0;
char *cmd_str[10];
string sittings[] = {
		"强制全屏幕",
		"缓存字体",
		"文字阴影",
		"显示文字框",
		"日文游戏",
		"",
		"",
		"",
		"恢复默认设置",
		"返回"
};
DrawListMode mainscreen_list_mode;

void draw_icon(int curr, int row, int col) {
	if (config.use_dpad && row == select_row && col == select_col) {
		vita2d_draw_rectangle(ICON_LEFT(col) - ITEM_BOX_MARGIN,
			ICON_TOP(row) - ITEM_BOX_MARGIN,
			ICON_WIDTH + ITEM_BOX_MARGIN * 2,
			ICON_HEIGHT + ITEM_BOX_MARGIN * 2,
			WHITE);
	}
	rom_list[curr].touch_area.left = ICON_LEFT(col);
	rom_list[curr].touch_area.top = ICON_TOP(row);
	rom_list[curr].touch_area.right = rom_list[curr].touch_area.left + ICON_WIDTH;
	rom_list[curr].touch_area.bottom = rom_list[curr].touch_area.top + ICON_HEIGHT;

	if (!rom_list[curr].icon) {
		return;
	}
	float w = rom_list[curr].w;
	float h = rom_list[curr].h;
	float z0 = ICON_WIDTH / w;
	float z1 = ICON_HEIGHT / h;
	float zoom = z0 < z1 ? z0 : z1;
	vita2d_draw_texture_scale_rotate_hotspot(rom_list[curr].icon,
		ICON_LEFT(col) + (ICON_WIDTH / 2),
		ICON_TOP(row) + (ICON_HEIGHT / 2),
		zoom, zoom,
		0,
		w / 2,
		h / 2
	);
}

void draw_icons(int curr) {
	// __________tm_bat
	// |__|__|__|__|__|
	// |__|__|__|__|__|
	// |__|__|__|__|__|
	// |__|__|__|__|__|
	// ------helps-----
	vita2d_draw_rectangle(ITEMS_PANEL_LEFT, ITEMS_PANEL_TOP,
		ITEMS_PANEL_WIDTH, ITEMS_PANEL_HEIGHT, BLACK);

	for (int i = 0; i + curr < rom_list.size() && i < (ICONS_COL * ICONS_ROW); i++) {
		draw_icon(i + curr, i / ICONS_COL, i % ICONS_COL);
	}
}

void draw_list_row(int curr, int row) {
	if (config.use_dpad && row == select_row) {
		vita2d_draw_rectangle(LIST_LEFT - ITEM_BOX_MARGIN,
			LIST_TOP(row) - ITEM_BOX_MARGIN,
			LIST_WIDTH + ITEM_BOX_MARGIN * 2,
			LIST_HEIGHT + ITEM_BOX_MARGIN * 2,
			WHITE);
		vita2d_draw_rectangle(LIST_LEFT,
			LIST_TOP(row),
			LIST_WIDTH,
			LIST_HEIGHT,
			BLACK);
	}

	rom_list[curr].touch_area.left = LIST_LEFT;
	rom_list[curr].touch_area.top = LIST_TOP(row);
	rom_list[curr].touch_area.right = rom_list[curr].touch_area.left + LIST_WIDTH;
	rom_list[curr].touch_area.bottom = rom_list[curr].touch_area.top + LIST_HEIGHT;

	if (rom_list[curr].icon) {
		float w = rom_list[curr].w;
		float h = rom_list[curr].h;
		float z0 = (float)LIST_HEIGHT / w;
		float z1 = (float)LIST_HEIGHT / h;
		float zoom = z0 < z1 ? z0 : z1;
		vita2d_draw_texture_scale_rotate_hotspot(rom_list[curr].icon,
			LIST_LEFT + (LIST_HEIGHT / 2),
			LIST_TOP(row) + (LIST_HEIGHT / 2),
			zoom, zoom,
			0,
			w / 2,
			h / 2
		);
	}

	int text_height = FONT_SIZE;// vita2d_font_text_height(font, FONT_SIZE, text);
	int text_top_margin = (LIST_HEIGHT - text_height) / 2;

	vita2d_font_draw_text(font,
		LIST_TEXT_LEFT,// + text_left_margin,
		LIST_TOP(row) + text_top_margin + text_height,
		WHITE, FONT_SIZE, rom_list[curr].char_name());
	vita2d_draw_rectangle(LIST_TEXT_LEFT + 410,
		LIST_TOP(row),
		LIST_WIDTH - (LIST_TEXT_LEFT + 410),
		LIST_HEIGHT,
		BLACK);
	vita2d_font_draw_text(font,
		LIST_TEXT_LEFT + 425,// + text_left_margin,
		LIST_TOP(row) + text_top_margin + text_height,
		WHITE, FONT_SIZE, rom_list[curr].char_path());
}

void draw_list(int curr) {
	// __________tm_bat
	// |__|___________|
	// |__|___________|
	// |__|___________|
	// |__|___________|
	// ------helps-----
	vita2d_draw_rectangle(ITEMS_PANEL_LEFT, ITEMS_PANEL_TOP,
		ITEMS_PANEL_WIDTH, ITEMS_PANEL_HEIGHT, BLACK);

	for (int i = 0; i + curr < rom_list.size() && i < LIST_ROW; i++) {
		draw_list_row(i + curr, i);
	}
}

void draw_title() {
	
	// |-----title----|
	// ----------------
	// ......
	vita2d_draw_rectangle(0, 0,
		ITEMS_PANEL_WIDTH, HEADER_HEIGHT, BLACK_1);
	char *ver_str = new char[256];
	sprintf(ver_str, "Vita ONScripter-Jh (yuri) %s (GUI %d, Jh %s, %d.%02d)\n", 
		ONS_JH_VITA_VERSION, GUI_VERSION, ONS_JH_VERSION, 
		NSC_VERSION / 100, NSC_VERSION % 100);
	
	char time_str[16];
	SceDateTime time;
	sceRtcGetCurrentClock(&time, 0);

	getTimeString(time_str, 24, &time);
	vita2d_font_draw_text(font, 5, FONT_SIZE - 1, WHITE, FONT_SIZE, ver_str);
	vita2d_font_draw_text(font, ITEMS_PANEL_WIDTH - 80, FONT_SIZE - 1 , WHITE, FONT_SIZE, time_str);
	free(ver_str);
}

void draw_help() {
	// ......
	// ________________
	// |-----helps----|
	vita2d_draw_rectangle(0, FOOTER_TOP, ITEMS_PANEL_WIDTH, FOOTER_HEIGHT, BLACK_1);
	if(rom_list.size() == 0)
		vita2d_font_draw_text(font, 5, FOOTER_TOP + FONT_SIZE - 1, WHITE, FONT_SIZE, "无游戏！请把游戏放至ux0(uma0/ur0)下的onsemu/中");
	else
		vita2d_font_draw_text(font, 5, FOOTER_TOP + FONT_SIZE - 1, WHITE, FONT_SIZE , "L 打开设置菜单   R 查看帮助   Select 关于");
	vita2d_font_draw_text(font, ITEMS_PANEL_WIDTH - 120, FOOTER_TOP + FONT_SIZE - 1, WHITE, FONT_SIZE -2, GUI_VERSION_DATE);
}


void draw_button(int left, int top, int width, int height, string text, int zoom, int pressed) {
	// TODO render more looking button
	int text_color;
	if (pressed) {
		vita2d_draw_rectangle(left, top, width, height, BLACK);
		vita2d_draw_rectangle(left + 4, top + 4, width - 5, height - 5, LIGHT_GRAY);
		text_color = WHITE;
	}
	else {
		vita2d_draw_rectangle(left, top, width, height, BLACK);
		vita2d_draw_rectangle(left + 1, top + 1, width - 5, height - 5, WHITE);
		text_color = BLACK;
	}
	int text_width = vita2d_font_text_width(font, zoom, RomInfo::to_char(text));
	int text_height = vita2d_font_text_height(font, zoom, RomInfo::to_char(text));
	int text_left_margin = (width - text_width) / 2;
	int text_top_margin = (height - text_height) / 2;

	vita2d_font_draw_text(font,
		left + text_left_margin,
		top + text_top_margin + text_height-5,
		text_color, zoom, RomInfo::to_char(text));
}


struct config_item {
	const char *name;
	const char *value;
};

void draw_config() {
	struct config_item items[] = {
		{"显示模式",		 strcmp(config.list_mode,"icon") ? "列表" : "图标"},
		{"仅使用按键",		 config.use_dpad ? "启用" : "关闭"},
		{"图标模式行数",     RomInfo::to_char(config.icon_row)},
		{"图标模式列数",     RomInfo::to_char(config.icon_col)},
		{"列表模式行数",     RomInfo::to_char(config.list_row)},
		{"游戏触摸控制",	 config.use_btouch == 0 ? "关闭" :(config.use_btouch == 1 ? "仅前触屏" : "前后触屏") }
	};

	// FIXME: ugly UI
	vita2d_draw_rectangle(ITEMS_PANEL_LEFT, ITEMS_PANEL_TOP,ITEMS_PANEL_WIDTH, ITEMS_PANEL_HEIGHT, BLACK_HALF_ALPHA);

	for (int i = 0; i < CONFIG_NUM; i++) {
		int color = i == select_config ? GREEN : WHITE;
		if (!strcmp(config.list_mode, "list")) {
			if (i == 2 || i == 3)
				color = LIGHT_SLATE_GRAY;
		}
		else if (i == 4)
			color = LIGHT_SLATE_GRAY;
			
		vita2d_font_draw_text(font, ITEMS_PANEL_LEFT + 10, ITEMS_PANEL_TOP + i * 30 + 30, color, FONT_SIZE, (char*)items[i].name);
		vita2d_font_draw_text(font, ITEMS_PANEL_LEFT + 200, ITEMS_PANEL_TOP + i * 30 + 30, color, FONT_SIZE, (char*)items[i].value);
	}
}

void draw_appinfo_icon(int curr) {

	float w = rom_list[curr].w;
	float h = rom_list[curr].h;
	float z0 = APPINFO_ICON_WIDTH / w;
	float z1 = APPINFO_ICON_HEIGHT / h;
	float zoom = z0 < z1 ? z0 : z1;

	vita2d_draw_texture_scale_rotate_hotspot(rom_list[curr].icon,
		APPINFO_ICON_LEFT + (APPINFO_ICON_WIDTH / 2),
		APPINFO_ICON_TOP + (APPINFO_ICON_HEIGHT / 2),
		zoom, zoom,
		0,
		w / 2,
		h / 2
	);
}

void draw_appinfo(ScreenState state, int choose) {
	// __________tm_bat
	// |name |_|__|__|
	// |...  |_|__|__|
	// |...  |_|__|__|
	// |_____|_|__|__|
	// ------helps-----

	// .--------------------------------------
	// | icon  | backup   |
	// |       | restore  |
	// |       | format   |
	// |       | ...      |
	// |------------------|
	// | title id         |
	// | title            |
	// | cart /dl         |
	// | save position    |
	// | ...              |
	// '---------------------------------------
	vita2d_draw_rectangle(APPINFO_PANEL_LEFT, APPINFO_PANEL_TOP,
		APPINFO_PANEL_WIDTH, APPINFO_PANEL_HEIGHT, WHITE);

	draw_appinfo_icon(choose);
	draw_button(APPINFO_BUTTON_LEFT, APPINFO_BUTTON_TOP(0),
		APPINFO_BUTTON_WIDTH, APPINFO_BUTTON_HEIGHT,
		RomInfo::to_char("启动"), FONT_SIZE,
		(state == START_MODE));

	draw_button(APPINFO_BUTTON_LEFT, APPINFO_BUTTON_TOP(1),
		APPINFO_BUTTON_WIDTH, APPINFO_BUTTON_HEIGHT,
		RomInfo::to_char("设置"), FONT_SIZE,
		(state == SETTING_MODE));
	draw_button(APPINFO_BUTTON_LEFT, APPINFO_BUTTON_TOP(2),
		APPINFO_BUTTON_WIDTH, APPINFO_BUTTON_HEIGHT,
		RomInfo::to_char("制作气泡"), FONT_SIZE,
		(state == SHORTCUT_MODE));

	draw_button(APPINFO_BUTTON_LEFT, APPINFO_BUTTON_TOP(3),
		APPINFO_BUTTON_WIDTH, APPINFO_BUTTON_HEIGHT,
		RomInfo::to_char("暂未开放"), FONT_SIZE,
		(state == DELETE_MODE));

	if (config.use_dpad && state == PRINT_APPINFO) {
		vita2d_draw_rectangle(APPINFO_BUTTON_LEFT,
			APPINFO_BUTTON_TOP(select_appinfo_button),
			APPINFO_BUTTON_WIDTH, APPINFO_BUTTON_HEIGHT,
			LIGHT_GRAY);
	}

	vita2d_draw_rectangle(APPINFO_DESC_LEFT, APPINFO_DESC_TOP,
		APPINFO_DESC_WIDTH, APPINFO_DESC_HEIGHT,
		LIGHT_SLATE_GRAY);

	static int old_choose = -1;
	static char tmp_str[512];

	if (choose != old_choose) {
		old_choose = choose;
		uint64_t size = 0;
		uint32_t file_num = 0, floder_num = 0;
		char size_str[16];
		getPathInfo(rom_list[choose].char_path(), &size, &floder_num, &file_num);
		getSizeString(size_str, size);
		sprintf(tmp_str, "名称：%s\n位置：%s\n大小：%s\n包含：%d个文件，%d个文件夹", rom_list[choose].char_name(), rom_list[choose].char_path(), size_str, file_num, floder_num);
	}

	vita2d_font_draw_text(font,
		APPINFO_DESC_LEFT + APPINFO_DESC_PADDING,
		APPINFO_DESC_TOP + APPINFO_DESC_PADDING + 30,
		BLACK, FONT_SIZE, tmp_str);
}

void draw_slots(int index_, int slot) {
	// __________tm_bat
	// |name |=======|
	// |...  |=======|
	// |...  |..     |
	// |_____|_______|
	// ------helps-----

	// .--------------------------------------
	// | icon  | backup   | slot0
	// |       | restore  | slot1
	// |       | format   |
	// |       | ...      |  ..
	// |------------------|
	// | title id         |
	// | title            |
	// | cart /dl         |
	// | save position    |
	// | ...              | slot9
	// '---------------------------------------
	vita2d_draw_rectangle(SLOT_PANEL_LEFT, SLOT_PANEL_TOP,
		SLOT_PANEL_WIDTH, SLOT_PANEL_HEIGHT, WHITE);

	for (int i = 0; i < SLOT_BUTTON; i++) {
		string tmp = sittings[i];
		if (i < SITTINGS_NUM) {
			while (tmp.length() < 30)
				tmp += " ";
			tmp += "[";
			tmp += cmd[i] ? "开启" : "关闭";
			tmp += "]";
			if (cmd[i])
				tmp += "●";
			else
				tmp += "○";
		}
		draw_button(SLOT_BUTTON_LEFT, SLOT_BUTTON_TOP(i),
			SLOT_BUTTON_WIDTH, SLOT_BUTTON_HEIGHT,
			tmp, FONT_SIZE,
			(slot == i));

		if (slot < 0 && config.use_dpad && select_slot == i) {
			vita2d_draw_rectangle(SLOT_BUTTON_LEFT, SLOT_BUTTON_TOP(i),
				SLOT_BUTTON_WIDTH, SLOT_BUTTON_HEIGHT,
				LIGHT_GRAY);
		}
	}
	/*if (sittings) {
		free(sittings);
	}*/
}

void draw_message(char *msg, int choose,int fontsize) {


	int text_width = vita2d_font_text_width(font, fontsize, msg);
	int text_height = vita2d_font_text_height(font, fontsize, msg);

	int padding = 50;
	int width = text_width + (padding * 2);
	int height = text_height + (padding * 2);

	int left = (SCREEN_WIDTH - width) / 2;
	int top = (SCREEN_HEIGHT - height) / 2;

	vita2d_draw_rectangle(left, top, width, height, LIGHT_GRAY);

	vita2d_font_draw_text(font, left + padding, top + padding, BLACK, fontsize, msg);
	vita2d_font_draw_text(font,
		left + ((width - confirm_msg_width) / 2),
		top + height - 25, BLACK, fontsize, confirm_msg);
	
}

void draw_alert(char *msg, int fontsize) {


	int text_width = vita2d_font_text_width(font, fontsize, msg);
	int text_height = vita2d_font_text_height(font, fontsize, msg);
	int padding = 50;
	int width = text_width + (padding * 2);
	int height = text_height + (padding * 2);

	int left = (SCREEN_WIDTH - width) / 2;
	int top = (SCREEN_HEIGHT - height) / 2;

	vita2d_draw_rectangle(left, top, width, height, LIGHT_GRAY);

	vita2d_font_draw_text(font, left + padding, top + padding, BLACK, fontsize, msg);
	vita2d_font_draw_text(font,
		left + ((width - close_msg_width) / 2),
		top + height - 25, BLACK, fontsize, close_msg);

}

void draw_screen(ScreenState state, int curr, int choose, int slot) {

	vita2d_start_drawing();
	vita2d_clear_screen();

	if (state >= MAIN_SCREEN) {
		//draw header
		draw_title();
		//draw footer
		draw_help();

		switch (mainscreen_list_mode) {
		case USE_ICON:
			draw_icons(curr);
			break;
		case USE_LIST:
			draw_list(curr);
			break;
		}
	}

	if (state == CONFIG_SCREEN) {
		draw_config();
	}
	if (state == HELP_MSG) {
		draw_alert((char*)help_msg, FONT_SIZE);
	}
	if (state == ABOUT_MSG) {
		draw_alert((char*)about_msg, FONT_SIZE);
	}
	if (state >= PRINT_APPINFO) {
		draw_appinfo(state, choose);
	}

	switch (state) {
	case START_MODE:
		break;
	case SETTING_MODE:
		draw_slots(choose, -1);
		break;
	case DELETE_MODE:
		draw_message((char*)"[暂未开放的功能]", choose, FONT_SIZE);
		break;
	case SHORTCUT_MODE:
		draw_message((char*)"是否要生成快捷启动气泡？", choose, FONT_SIZE);
		break;
	case SHORTCUT_WAIT:
		draw_alert((char*)"正在生成气泡中...请勿操作...", FONT_SIZE);
		break;
	case SHORTCUT_DONE_MODE:
		draw_alert((char*)"快捷启动气泡生成完毕！", FONT_SIZE);
		break;
	case SHORTCUT_FAIL_MODE:
		draw_alert((char*)"快捷启动气泡生成失败...", FONT_SIZE);
		break;
	default:
		break;
	}

	vita2d_end_drawing();
	vita2d_wait_rendering_done();
	vita2d_swap_buffers();

}

#define IN_RANGE(start, end, value) (start < value && value < end)
#define IS_TOUCHED(rect, pt) \
    (IN_RANGE(rect.left, rect.right, pt.x) && IN_RANGE(rect.top, rect.bottom, pt.y))

ScreenState on_mainscreen_event_with_touch(int steps, int &step, int &curr, int &touched) {
	// FIXME: cleanup
	int move_row = 0;
	int move_col = 0;
	switch (mainscreen_list_mode) {
	case USE_LIST:
		move_row = LIST_ROW;
		move_col = 1;
		break;
	case USE_ICON:
	default:
		move_row = 1;
		move_col = ICONS_COL;
		break;
	}
	int btn = read_buttons();

	if (btn & SCE_CTRL_HOLD) {
		return UNKNOWN;
	}
	if (btn & SCE_CTRL_R2) {
		return HELP_MSG;
	}
	if (btn & SCE_CTRL_L2) {
		return CONFIG_SCREEN;
	}
	if (btn & SCE_CTRL_SELECT) {
		return ABOUT_MSG;
	}
	if (btn & SCE_CTRL_UP) {
		if (step <= 0) {
			return UNKNOWN;
		}
		step -= move_row;
		curr -= move_row * move_col;
		return MAIN_SCREEN;
	}
	if (btn & SCE_CTRL_DOWN) {
		if (step >= steps) {
			return UNKNOWN;
		}
		step += move_row;
		curr += move_row * move_col;
		return MAIN_SCREEN;
	}

	point p;
	if (!read_touchscreen(&p)) {
		return UNKNOWN;
	}

	for (int i = curr; i < rom_list.size() && i < (ICONS_COL * ICONS_ROW); i++) {
		if (IS_TOUCHED(rom_list[i].touch_area, p)) {
			touched = i;
			return PRINT_APPINFO;
		}
	}
	return UNKNOWN;
}

int selectable_count(int curr, int row, int col) {
	int selectable_count = 0;
	while (curr < rom_list.size() && selectable_count < (row * col)) {
		selectable_count += 1;
		curr++;
	}
	return selectable_count;
}

#define IS_OVERFLOW() ( \
        select_row * max_col + select_col >= \
        selectable_count(curr, max_row, max_col) \
    )

ScreenState on_mainscreen_event_with_dpad(int steps, int &step, int &curr, int &touched) {
	int moves;
	int max_row;
	int max_col;
	switch (mainscreen_list_mode) {
	case USE_LIST:
		moves = 1;
		max_row = LIST_ROW;
		max_col = 1;
		break;
	case USE_ICON:
	default:
		moves = ICONS_COL;
		max_row = ICONS_ROW;
		max_col = ICONS_COL;
		break;
	}

	int btn = read_buttons();

	if (btn & SCE_CTRL_UP) {
		if (select_row == 0) {
			if (step == 0) {
				return UNKNOWN;
			}
			step -= 1;
			curr -= moves;
		}
		else {
			select_row -= 1;
		}
		return MAIN_SCREEN;
	}
	if (btn & SCE_CTRL_DOWN) {
		if (select_row + 1 == max_row) {
			if (step == steps) {
				return UNKNOWN;
			}
			step += 1;
			curr += moves;
		}
		else {
			select_row += 1;
		}
		if (IS_OVERFLOW()) {
			select_row -= 1;
		}
		return MAIN_SCREEN;
	}
	if (btn & SCE_CTRL_LEFT) {
		select_col -= 1;
		if (select_col < 0) {
			select_col = 0;
		}
		return MAIN_SCREEN;
	}
	if (btn & SCE_CTRL_RIGHT) {
		select_col += 1;
		if (select_col >= max_col) {
			select_col = max_col - 1;
		}
		if (IS_OVERFLOW()) {
			select_col -= 1;
		}
		return MAIN_SCREEN;
	}
	if (!(btn & SCE_CTRL_HOLD) && btn & SCE_CTRL_R2) {
		return HELP_MSG;
	}
	if (!(btn & SCE_CTRL_HOLD) && btn & SCE_CTRL_L2) {
		return CONFIG_SCREEN;
	}
	if (!(btn & SCE_CTRL_HOLD) && btn & SCE_CTRL_SELECT) {
		return ABOUT_MSG;
	}

	if (!(btn & SCE_CTRL_HOLD) && btn & SCE_CTRL_ENTER) {
		int tmp_curr = curr;
		tmp_curr += (rom_list.size() - 1 < (select_row * max_col) + select_col) ? 
			rom_list.size() - 1 : (select_row * max_col) + select_col;
		//for (; curr<rom_list.size() && curr < (select_row * max_col) + select_col; curr++);
		touched = tmp_curr;
		select_appinfo_button = 0;
		return PRINT_APPINFO;
	}

	return UNKNOWN;
}
#undef IS_OVERFLOW

ScreenState on_mainscreen_event(int steps, int &step, int &curr, int &touched) {
	if (config.use_dpad) {
		return on_mainscreen_event_with_dpad(steps, step, curr, touched);
	}
	return on_mainscreen_event_with_touch(steps, step, curr, touched);
}

ScreenState on_config_event() {
	static int need_refresh = 0;
	static int need_save = 0;
	int btn = read_buttons();

	if (btn & SCE_CTRL_HOLD) {
		return UNKNOWN;
	}

	if (btn & SCE_CTRL_CANCEL || btn & SCE_CTRL_L2) {
		if (need_save) {
			save_config();
		}
		if (need_refresh) {
			need_refresh = 0;
			return RELOAD_MAINSCREEN;
		}
		return MAIN_SCREEN;
	}

	if (btn & SCE_CTRL_UP) {
		select_config -= 1;
		if (strncmp(config.list_mode, "icon", 4) == 0) {
			for (; select_config == 4; select_config--);
		}
		else {
			for (; select_config == 2|| select_config == 3; select_config--);
		}
		if (select_config < 0) {
			select_config = 0;
		}
		return UNKNOWN;
	}

	if (btn & SCE_CTRL_DOWN) {
		select_config += 1;
		if (strncmp(config.list_mode, "icon", 4) == 0) {
			for (; select_config == 4; select_config++);
		}
		else {
			for (; select_config == 2 || select_config == 3; select_config++);
		}
		if (select_config > CONFIG_NUM - 1) {
			select_config = CONFIG_NUM - 1;
		}
		return UNKNOWN;
	}

	if (btn & SCE_CTRL_ENTER) {
		switch (select_config) {
		case 0:
			if (strncmp(config.list_mode, "icon", 4) == 0) {
				strncpy(config.list_mode, "list", 4);
				mainscreen_list_mode = USE_LIST;
			}
			else {
				strncpy(config.list_mode, "icon", 4);
				mainscreen_list_mode = USE_ICON;
			}
			break;
		case 1:
			config.use_dpad = !config.use_dpad;
			break;
		case 5:
			config.use_btouch++;
			if (config.use_btouch > 2)
				config.use_btouch = 0;
			break;
		default:
			break;
		}
		need_refresh = 1;
		need_save = 1;

	}
	if (btn & SCE_CTRL_LEFT) {
		switch (select_config) {
		case 0:
			if (strncmp(config.list_mode, "icon", 4) == 0) {
				strncpy(config.list_mode, "list", 4);
				mainscreen_list_mode = USE_LIST;
			}
			else {
				strncpy(config.list_mode, "icon", 4);
				mainscreen_list_mode = USE_ICON;
			}
			break;
		case 1:
			config.use_dpad = !config.use_dpad;
			break;
		case 2:
			config.icon_row--;
			if (config.icon_row < ICON_ROW_MIN)
				config.icon_row = ICON_ROW_MIN;
			ICONS_ROW = config.icon_row;
			break;
		case 3:
			config.icon_col--;
			if (config.icon_col < ICON_COL_MIN)
				config.icon_col = ICON_COL_MIN;
			ICONS_COL = config.icon_col;
			break;
		case 4:
			config.list_row--;
			if (config.list_row < LIST_ROW_MIN)
				config.list_row = LIST_ROW_MIN;
			LIST_ROW = config.list_row;
			break;
		default:
			break;
		}
		need_refresh = 1;
		need_save = 1;
	}
	if (btn & SCE_CTRL_RIGHT) {
		switch (select_config) {
		case 0:
			if (strncmp(config.list_mode, "icon", 4) == 0) {
				strncpy(config.list_mode, "list", 4);
				mainscreen_list_mode = USE_LIST;
			}
			else {
				strncpy(config.list_mode, "icon", 4);
				mainscreen_list_mode = USE_ICON;
			}
			break;
		case 1:
			config.use_dpad = !config.use_dpad;
			break;
		case 2:
			config.icon_row++;
			if (config.icon_row > ICON_ROW_MAX)
				config.icon_row = ICON_ROW_MAX;
			ICONS_ROW = config.icon_row;
			break;
		case 3:
			config.icon_col++;
			if (config.icon_col > ICON_COL_MAX)
				config.icon_col = ICON_COL_MAX;
			ICONS_COL = config.icon_col;
			break;
		case 4:
			config.list_row++;
			if (config.list_row > LIST_ROW_MAX)
				config.list_row = LIST_ROW_MAX;
			LIST_ROW = config.list_row;
			break;
		default:
			break;
		}
		need_refresh = 1;
		need_save = 1;
	}
	return UNKNOWN;
}

#define APPINFO_BUTTON_AREA(n) \
    { \
        .left = APPINFO_BUTTON_LEFT, \
        .top = APPINFO_BUTTON_TOP(n), \
        .right = APPINFO_BUTTON_LEFT + APPINFO_BUTTON_WIDTH, \
        .bottom = APPINFO_BUTTON_TOP(n) + APPINFO_BUTTON_HEIGHT, \
    }

ScreenState on_appinfo_button_event(point p) {
	static rectangle backup_button_area = APPINFO_BUTTON_AREA(0);
	static rectangle restore_button_area = APPINFO_BUTTON_AREA(1);
	static rectangle delete_button_area = APPINFO_BUTTON_AREA(2);
	static rectangle reset_button_area = APPINFO_BUTTON_AREA(3);
	if (IS_TOUCHED(backup_button_area, p)) {
		return START_MODE;
	}

	if (IS_TOUCHED(restore_button_area, p)) {
		return SETTING_MODE;
	}

	if (IS_TOUCHED(delete_button_area, p)) {
		return SHORTCUT_MODE;
	}

	if (IS_TOUCHED(reset_button_area, p)) {
		return DELETE_MODE;
	}

	return UNKNOWN;
}

#undef APPINFO_BUTTON_AREA

ScreenState on_appinfo_event_with_touch() {
	static rectangle appinfo_area = {
		APPINFO_PANEL_LEFT,
		APPINFO_PANEL_TOP,
		APPINFO_PANEL_LEFT + APPINFO_PANEL_WIDTH,
		APPINFO_PANEL_TOP + APPINFO_PANEL_HEIGHT,
	};

	int btn = read_buttons();

	if (!(btn & SCE_CTRL_HOLD) && btn & SCE_CTRL_CANCEL) {
		return MAIN_SCREEN;
	}
	// TODO

	point p;
	if (!read_touchscreen(&p)) {
		return UNKNOWN;
	}

	if (!IS_TOUCHED(appinfo_area, p)) {
		return MAIN_SCREEN;
	}

	return on_appinfo_button_event(p);
}

ScreenState on_appinfo_event_with_dpad() {
	int btn = read_buttons();
	if (btn & SCE_CTRL_HOLD) {
		return UNKNOWN;
	}

	if (btn & SCE_CTRL_UP) {
		select_appinfo_button -= 1;
		if (select_appinfo_button < 0) {
			select_appinfo_button = 0;
		}
		return PRINT_APPINFO;
	}
	if (btn & SCE_CTRL_DOWN) {
		select_appinfo_button += 1;
		if (select_appinfo_button >= APPINFO_BUTTON) {
			select_appinfo_button = APPINFO_BUTTON - 1;
		}
		return PRINT_APPINFO;
	}
	if (btn & SCE_CTRL_CANCEL) {
		return MAIN_SCREEN;
	}
	if (btn & SCE_CTRL_ENTER) {
		select_slot = 0;
		switch (select_appinfo_button) {
		case 0:
			return START_MODE;
		case 1:
			return SETTING_MODE;
		case 2:
			return SHORTCUT_MODE;
		case 3:
			return DELETE_MODE;
		}
	}
	return UNKNOWN;
}

ScreenState on_appinfo_event() {
	if (config.use_dpad) {
		return on_appinfo_event_with_dpad();
	}
	return on_appinfo_event_with_touch();
}

ScreenState on_slot_event_with_touch(int &slot) {
	slot = -1;
	int btn = read_buttons();

	if (!(btn & SCE_CTRL_HOLD) && btn & SCE_CTRL_CANCEL) {
		return PRINT_APPINFO;
	}

	point p;
	if (!read_touchscreen(&p)) {
		return UNKNOWN;
	}

	ScreenState move_appinfo_action = on_appinfo_button_event(p);
	if (move_appinfo_action != UNKNOWN) {
		return move_appinfo_action;
	}

	for (int i = 0; i < SLOT_BUTTON; i++) {
		rectangle slot_area = {
			 SLOT_BUTTON_LEFT,
			 SLOT_BUTTON_TOP(i),
			 SLOT_BUTTON_LEFT + SLOT_BUTTON_WIDTH,
			 SLOT_BUTTON_TOP(i) + SLOT_BUTTON_HEIGHT,
		};
		if (IS_TOUCHED(slot_area, p)) {
			slot = i;
			if (slot == SITTINGS_DEFAULT) {
				for (int j = 0; j < SITTINGS_NUM; j++) cmd[j] = cmd_default[j];
			}
			else if (slot == SITTINGS_RETURN) {
				return PRINT_APPINFO;
			}
			else if (slot < SITTINGS_NUM) {
				cmd[slot] = !cmd[slot];
			}
			return UNKNOWN;
		}
	}
	return UNKNOWN;
}

ScreenState on_slot_event_with_dpad(int &slot) {
	slot = -1;
	int btn = read_buttons();

	if (btn & SCE_CTRL_UP) {
		select_slot -= 1;
		if (select_slot < 0) {
			select_slot = 0;
		}
		return UNKNOWN;
	}
	if (btn & SCE_CTRL_DOWN) {
		select_slot += 1;
		if (select_slot >= SLOT_BUTTON) {
			select_slot = SLOT_BUTTON - 1;
		}
		return UNKNOWN;
	}
	if (!(btn & SCE_CTRL_HOLD) && btn & SCE_CTRL_CANCEL) {
		return PRINT_APPINFO;
	}
	if (!(btn & SCE_CTRL_HOLD) && btn & SCE_CTRL_ENTER) {
		slot = select_slot;
		if (slot == SITTINGS_DEFAULT) {
			for (int j = 0; j < SITTINGS_NUM; j++) cmd[j] = cmd_default[j];
		}
		else if (slot == SITTINGS_RETURN) {
			return PRINT_APPINFO;
		}
		else if (slot < SITTINGS_NUM) {
			cmd[slot] = !cmd[slot];
		}
		return UNKNOWN;
	}
	return UNKNOWN;
}

ScreenState on_slot_event(int &slot) {
	if (config.use_dpad) {
		return on_slot_event_with_dpad(slot);
	}
	return on_slot_event_with_touch(slot);
}

ScreenState on_message_event(int curr, int(*progress_func)(int), ScreenState state_done, ScreenState state_fail, ScreenState state_cancel,int norun_fun = 0) {
	while (1) {

		int btn = read_buttons();
		if (btn & SCE_CTRL_HOLD) {
			continue;
		}
		if (btn & SCE_CTRL_ENTER) {
			if(norun_fun)
				return state_done;
			if (progress_func(curr)) {
				return state_done;
			}
			else {
				return state_fail;
			}
		}
		if (btn & SCE_CTRL_CANCEL) {
			return state_cancel;
		}
	}
	return state_cancel;
}

ScreenState on_alert_event(ScreenState state) {
	while (1) {
		int btn = read_buttons();
		if (btn & SCE_CTRL_HOLD) {
			continue;
		}
		if (btn & SCE_CTRL_ENTER) {
			break;
		}
	}
	return state;
}

int  game_delete(int choose) {

	return 1;
}

void  game_start(int choose) {
	game_start_select = choose;
}

char* get_title_id() {
	int ret;
	char *title_id;
	title_id = new char[32];
	int num = -1;
	do {
		num++;
		sprintf(title_id, "ux0:app/ONSVG0%03d/\0", num);
		ret = checkFolderExist(title_id);
		printf("%d %s\n",ret, title_id);
	} while (ret);
	sprintf(title_id, "ONSVG0%03d\0", num);
	return title_id;
}

int game_shortcut(int choose) {
	string path = PACKAGE_TEMP;
	int ret;
	if (checkFolderExist(path.c_str())) {
		ret = removePath(path);
		if (ret < 0) {
			printf("removePath() = 0x%08X\n", ret);
		}
	}

//temp:EBOOT
	sceIoMkdir(path.c_str(), 0777);
	copyFile("app0:eboot.bin", (path + "/eboot.bin").c_str());//EBOOT
	copyFile("app0:onsjh.bin", (path + "onsjh.bin").c_str());//ons
	char *src_path = new char[64];
	char *dst_path = new char[64];
	char *tmp_str = new char[512];
//sittings.txt
	sprintf(src_path, "%s/%s", rom_list[choose].char_path(), SITTINGS_FILE);
	sprintf(dst_path, "%s/%s", path.c_str(), SITTINGS_FILE);
	copyFile(src_path, dst_path);
//startup.ini
	sprintf(dst_path, "%s/%s", path.c_str(), "startup.ini");
	FILE * tmp = fopen(dst_path, "w");
	sprintf(tmp_str,
		"[AUTO_START]\n"
		"use_btouch = %d\n"
		"rom_path = %s",
		config.use_btouch,
		rom_list[choose].char_path()
	);
	fprintf(tmp, tmp_str);
	fclose(tmp);

	free(src_path);
	free(dst_path);
	free(tmp_str);
//sce_sys
	sceIoMkdir((path + "/sce_sys/").c_str(), 0777);
	//copyFile(rom_list[choose].char_icon_path(), (path + "/sce_sys/icon0.png").c_str());
	copyFile("app0:sce_sys/icon0.png", (path + "/sce_sys/icon0.png").c_str());
	copyFile("app0:sce_sys/param.sfo", (path + "/sce_sys/param.sfo").c_str());
	copyPath("app0:sce_sys/livearea", (path + "/sce_sys/livearea").c_str());
//TITLE ID
	SceDateTime time, time_local;
	sceRtcGetCurrentClock(&time, 0);
	convertUtcToLocalTime(&time_local,&time);
	char *title_id_tmp;
	title_id_tmp = new char[10];
	sprintf(title_id_tmp, "G%02d%02d%02d%02d", time_local.day, time_local.hour, time_local.minute, time_local.second);

//TITLE
	string title = getFileName(rom_list[choose].path);
//Install
	VitaPackage pkg(path);
	pkg.SetSFOString(title, title_id_tmp);
	ret = pkg.Install();
	if(!ret)
		printf("Shortcut fail\n", ret);
	return ret;
}

int mainloop() {
	int rows;
	int steps;
	switch (mainscreen_list_mode) {
	case USE_LIST:
		rows = rom_list.size();
		steps = rows - LIST_ROW;
		break;
	case USE_ICON:
	default:
		rows = (rom_list.size() / ICONS_COL) + ((rom_list.size() % ICONS_COL) ? 1 : 0);
		steps = rows - ICONS_ROW;
		break;
	}
	//printf("total: %d row: %d steps: %d\n", rom_list.size(), rows, steps);

	if (steps < 0) {
		steps = 0;
	}

	int curr = 0;
	int choose = 0;
	int step = 0;
	select_row = 0;
	select_col = 0;

	int need_load = 0;
	int need_save = 0;
	ScreenState state = MAIN_SCREEN;
	int slot = -1;
	while (1) {
		draw_screen(state, curr, choose,slot);
		ScreenState new_state = UNKNOWN;
		while (1) {
			switch (state) {
			case MAIN_SCREEN:
				if (!need_load) need_load = 1;
				new_state = on_mainscreen_event(steps, step, curr, choose);
				//printf("%d \n", curr);
				break;
			case CONFIG_SCREEN:
				new_state = on_config_event();
				break;
			case HELP_MSG:
				new_state = on_alert_event(MAIN_SCREEN);
				break;
			case ABOUT_MSG:
				new_state = on_alert_event(MAIN_SCREEN);
				break;
			case PRINT_APPINFO:
				if (need_save) {
					parseOption(startup_cmd, cmd, cmd_str, 1);
					sittings_file(rom_list[choose].path, startup_cmd, 'w');
					need_save = 0;
				}else if (need_load) {
					sittings_file(rom_list[choose].path, startup_cmd, 'r');
					parseOption(startup_cmd, cmd, NULL, 0);
					need_load = 0;
				}
				new_state = on_appinfo_event();
				break;
			case START_MODE:
				game_start(choose);
				new_state = PRINT_APPINFO;
				return -1;
				break;
			case SETTING_MODE:
				if (!need_save) need_save = 1;
				new_state = on_slot_event(slot);
				break;
			case DELETE_MODE:				
				new_state = on_message_event(choose, game_delete, PRINT_APPINFO, PRINT_APPINFO, PRINT_APPINFO);
				break;
			case SHORTCUT_MODE:
				new_state = on_message_event(choose, game_shortcut, SHORTCUT_WAIT, PRINT_APPINFO, PRINT_APPINFO,1);
				break;
			case SHORTCUT_WAIT:
				if(game_shortcut(choose))
					new_state = SHORTCUT_DONE_MODE;
				else
					new_state = SHORTCUT_FAIL_MODE;
				break;
			case SHORTCUT_DONE_MODE:
				new_state = on_alert_event(PRINT_APPINFO);
				break;
			case SHORTCUT_FAIL_MODE:
				new_state = on_alert_event(PRINT_APPINFO);
				break;
			case RELOAD_MAINSCREEN:
				return 1;
			default:
				break;
			}
			if (new_state == UNKNOWN) {
				break;
			}
			state = new_state;
			
			break;
		}
	}
}

int unload_ui_start(string &rom_path) {
	//return 0;

	dictionary *ini;
	ini = iniparser_load("app0:startup.ini");
	if (ini == NULL) {
		return 0;
	}
	config.use_btouch = iniparser_getint(ini, "AUTO_START:use_btouch", 1);
	rom_path = iniparser_getstring(ini, "AUTO_START:rom_path", "app0:");
	iniparser_freedict(ini);
	sittings_file("app0:", startup_cmd, 'r', 1);
	if (startup_cmd != "") {
		parseOption(startup_cmd, cmd, NULL, 0);
		return 1;
	}
	return 0;
}


int main() 
{
	printf("ONScripter-Jh for Vita version %s (Jh %s, %d.%02d)\n", 
		ONS_JH_VITA_VERSION, ONS_JH_VERSION, ONS_VERSION, 
		NSC_VERSION / 100, NSC_VERSION % 100);

	sceIoMkdir("ux0:data/onsemu", 0777);
	vita2d_init();
	vita2d_set_clear_color(BLACK);
	
	//draw LOGO began
	/*vita2d_texture *image = vita2d_load_PNG_buffer(&_binary_res_logo_png_start);
	vita2d_start_drawing();
	vita2d_draw_texture(image, 0, 0);
	vita2d_end_drawing();
	vita2d_swap_buffers();

	sceKernelDelayThread(1000 * 3000);
	vita2d_free_texture(image);*/
	//draw LOGO end
	
	string rom_path;
	int unload_ui = unload_ui_start(rom_path);

	if (!unload_ui) 
	{
		font = vita2d_load_font_file("app0:default.ttf");
		load_config();

		if (strcmp(config.list_mode, "icon") == 0) {
			mainscreen_list_mode = USE_ICON;
		}
		else if (strcmp(config.list_mode, "list") == 0) {
			mainscreen_list_mode = USE_LIST;
		}
		init_input();

		confirm_msg = new char[256];
		sprintf(confirm_msg, "%s 取消    %s 确定", ICON_CANCEL, ICON_ENTER);
		confirm_msg_width = vita2d_font_text_width(font, FONT_SIZE, confirm_msg);
		close_msg = new char[256];
		sprintf(close_msg, "%s 关闭", ICON_ENTER);
		close_msg_width = vita2d_font_text_width(font, FONT_SIZE, close_msg);

		//load_config();
		load_rom_list();//\BC\D3\D4\D8ͼ\B1\EA
		while (mainloop() >= 0);

		vita2d_start_drawing();
		vita2d_clear_screen();
		vita2d_end_drawing();

		vita2d_free_font(font);
	}
	vita2d_fini();

	if (game_start_select >= 0 || unload_ui) 
	{
		cmd_num = parseOption(startup_cmd, cmd, cmd_str, 1);
		if (!unload_ui) {
			rom_path = rom_list[game_start_select].path;
			sittings_file(rom_path, startup_cmd, 'w');
		}
		cmd_str[cmd_num++] = (char*)"--root";
		cmd_str[cmd_num++] = RomInfo::to_char(rom_path);
		cmd_str[cmd_num++] = (char*)"--touch-mode";
		cmd_str[cmd_num++] = (char*)(config.use_btouch == 0 ? 
			"use_not_touch" : (config.use_btouch == 1 
				? "use_front_only_touch" : "use_front_back_touch"));
		cmd_str[cmd_num] = NULL;
		for(int i=0; i<cmd_num; i++)
		{
			printf("cmd_str[%d] %s\n", i, cmd_str[i]);
		}

		printf("prepare sceAppMgrLoadExec %s\n", ONSJH_PATH);
		int res = sceAppMgrLoadExec(ONSJH_PATH, cmd_str, NULL);
		printf("sceAppMgrLoadExec %s res=0x%08x\n", ONSJH_PATH, res);

		// int status;
		// // SceUID res = sceKernelLoadStartModule("app0:module/onsjh.suprx", 0, NULL, 0, NULL , &status);
		// SceUID modid = sceKernelLoadModule("app0:module/onsjh2.suprx", 0, NULL);
		// printf("sceKernelLoadModule onsjh.suprx modid=0x%08x", modid);
		// ons_main(cmd_num, cmd_str);
		// if(modid > 0)
		// {
		// 	// int res = sceKernelStartModule(modid, 0, NULL, 0, NULL, &status);
		// 	// printf("sceKernelStartModule onsjh.suprx res=0x%08x, status=%d", modid, status);
		// }
	}
}

//--------------------------------
struct RomInfo_s 
{
	string path;
	string name;
};

void end() 
{
	sceKernelExitProcess(0);
	exit(0);
}