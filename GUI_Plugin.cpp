
//#include <psp2/ctrl.h>

//#include "GUI_common.h"
#include "GUI_Plugin.h"
//#include "GUI_Utils.h"
//#include "Utils.h"
/*
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/touch.h>
#include <taihen.h>


#define HOOKS_NUM 4

static uint8_t current_hook = 0;
static SceUID hooks[HOOKS_NUM];
static tai_hook_ref_t refs[HOOKS_NUM];

void hookFunction(uint32_t nid, const void *func) {
	hooks[current_hook] = taiHookFunctionImport(&refs[current_hook], (const char*)TAI_MAIN_MODULE, TAI_ANY_LIBRARY, nid, func);
	current_hook++;
}

int sceTouchRead_patched(SceUInt32 port, SceTouchData *pData, SceUInt32 nBufs) {
	int ret = TAI_CONTINUE(int, refs[0], port, pData, nBufs);
	if (port == SCE_TOUCH_PORT_BACK) pData->reportNum = 0;
	return ret;
}

int sceTouchRead2_patched(SceUInt32 port, SceTouchData *pData, SceUInt32 nBufs) {
	int ret = TAI_CONTINUE(int, refs[1], port, pData, nBufs);
	if (port == SCE_TOUCH_PORT_BACK) pData->reportNum = 0;
	return ret;
}

int sceTouchPeek_patched(SceUInt32 port, SceTouchData *pData, SceUInt32 nBufs) {
	int ret = TAI_CONTINUE(int, refs[2], port, pData, nBufs);
	if (port == SCE_TOUCH_PORT_BACK) pData->reportNum = 0;
	return ret;
}

int sceTouchPeek2_patched(SceUInt32 port, SceTouchData *pData, SceUInt32 nBufs) {
	int ret = TAI_CONTINUE(int, refs[3], port, pData, nBufs);
	if (port == SCE_TOUCH_PORT_BACK) pData->reportNum = 0;
	return ret;
}

int module_start() {

	// Hooking touch functions
	hookFunction(0x169A1D58, sceTouchRead_patched);
	hookFunction(0x39401BEA, sceTouchRead2_patched);
	hookFunction(0xFF082DF0, sceTouchPeek_patched);
	hookFunction(0x3AD3D0A1, sceTouchPeek2_patched);

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop() {

	// Freeing hooks
	while (current_hook-- > 0) {
		taiHookRelease(hooks[current_hook], refs[current_hook]);
	}

	return SCE_KERNEL_STOP_SUCCESS;

}*/