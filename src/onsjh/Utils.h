/* -*- C++ -*-
 *
 *  Utils.h
 *
 *  Copyright (C) 2014 jh10001 <jh10001@live.cn>
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

#ifndef __UTILS_H__
#define __UTILS_H__


#include <stdio.h>
#include <stdarg.h>
#include <vitasdk.h>


static char *config_path;

namespace utils{

	inline void printInfo(const char *format, ...){
		va_list va;
		va_start(va, format);
		char str[512] = { 0 };
		vsnprintf(str, 512, format, va);
		va_end(va);

		sceClibPrintf(str);

	}

	inline void printError(const char *format, ...) {
		va_list va;
		va_start(va, format);
		char str[512] = { 0 };
		vsnprintf(str, 512, format, va);
		va_end(va);

		sceClibPrintf(str);

	}
}

#endif
