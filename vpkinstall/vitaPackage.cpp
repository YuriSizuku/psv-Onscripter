#include "vitaPackage.h"
#include "sha1.h"
#include "filesystem.h"
#include "../Utils.h"
#include <psp2/promoterutil.h>

#define ntohl __builtin_bswap32

extern unsigned char _binary_res_head_bin_start;
extern unsigned char _binary_res_head_bin_size;

#define SFO_MAGIC 0x46535000

#define PSF_TYPE_BIN 0
#define PSF_TYPE_STR 2
#define PSF_TYPE_VAL 4

std::string VitaPackage::Package_path = PACKAGE_TEMP_FOLDER;

static void fpkg_hmac(const uint8_t *data, unsigned int len, uint8_t hmac[16]) {
    SHA1_CTX ctx;
	char sha1[20];
	char buf[64];

	sha1_init(&ctx);
	sha1_update(&ctx, (BYTE*)data, len);
    sha1_final(&ctx, (BYTE*)sha1);

	memset(buf, 0, 64);
	memcpy(&buf[0], &sha1[4], 8);
	memcpy(&buf[8], &sha1[4], 8);
	memcpy(&buf[16], &sha1[12], 4);
	buf[20] = sha1[16];
	buf[21] = sha1[1];
	buf[22] = sha1[2];
	buf[23] = sha1[3];
	memcpy(&buf[24], &buf[16], 8);

	sha1_init(&ctx);
	sha1_update(&ctx, (BYTE*)buf, 64);
    sha1_final(&ctx, (BYTE*)sha1);
	memcpy(hmac, sha1, 16);
}

typedef struct SfoHeader {
	uint32_t magic;
	uint32_t version;
	uint32_t keyofs;
	uint32_t valofs;
	uint32_t count;
} __attribute__((packed)) SfoHeader;

typedef struct SfoEntry {
	uint16_t nameofs;
	uint8_t  alignment;
	uint8_t  type;
	uint32_t valsize;
	uint32_t totalsize;
	uint32_t dataofs;
} __attribute__((packed)) SfoEntry;

int getSfoString(char *buffer, const char *name, char *string, int length) {
	SfoHeader *header = (SfoHeader *)buffer;
	SfoEntry *entries = (SfoEntry *)((uint32_t)buffer + sizeof(SfoHeader));

	if (header->magic != SFO_MAGIC)
    	return -1;

	int i;
	for (i = 0; i < header->count; i++) {
		if (strcmp(buffer + header->keyofs + entries[i].nameofs, name) == 0) {
			memset(string, 0, length);
			strncpy(string, buffer + header->valofs + entries[i].dataofs, length);
			string[length-1] = '\0';
			return 0;
		}
	}

	return -2;
}
int setSfoString(char *buffer, const char *name, const char *string) {
	SfoHeader *header = (SfoHeader *)buffer;
	SfoEntry *entries = (SfoEntry *)((unsigned int)buffer + sizeof(SfoHeader));

	if (header->magic != SFO_MAGIC)
		return -1;

	int i;
	for (i = 0; i < header->count; i++) {
		if (strcmp(buffer + header->keyofs + entries[i].nameofs, name) == 0) {
			strcpy(buffer + header->valofs + entries[i].dataofs, string);
			return 0;
		}
	}

	return -2;
}
int WriteFile(const char *file, const void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}



int allocateReadFile(const char *file, char **buffer) {
	SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	int size = sceIoLseek32(fd, 0, SCE_SEEK_END);
	sceIoLseek32(fd, 0, SCE_SEEK_SET);

	*buffer = (char *)malloc(size);
	if (!*buffer) {
		sceIoClose(fd);
		return -1;
	}

	int read = sceIoRead(fd, *buffer, size);
	sceIoClose(fd);

	return read;
}

int makeHeadBin()
{
    uint8_t hmac[16];
    uint32_t off;
    uint32_t len;
    uint32_t out;

    if (checkFileExist((VitaPackage::Package_path + "sce_sys/package/head.bin").c_str()))
        return 0;

    // Read param.sfo
    char *sfo_buffer = nullptr;
    int res = allocateReadFile((VitaPackage::Package_path + "sce_sys/param.sfo").c_str(), &sfo_buffer);
    if (res < 0)
        return res;

    // Get title id
    char titleid[12];
    memset(titleid, 0, sizeof(titleid));
    getSfoString(sfo_buffer, "TITLE_ID", titleid, sizeof(titleid));

    // Enforce TITLE_ID format
    if (strlen(titleid) != 9)
        return -1;

    // Get content id
    char contentid[48];
    memset(contentid, 0, sizeof(contentid));
    getSfoString(sfo_buffer, "CONTENT_ID", contentid, sizeof(contentid));

    // Free sfo buffer
    free(sfo_buffer);

    // Allocate head.bin buffer
    uint8_t *head_bin = (uint8_t *)malloc((int)&_binary_res_head_bin_size);
    memcpy(head_bin, (void *)&_binary_res_head_bin_start, (int)&_binary_res_head_bin_size);

    // Write full title id
    char full_title_id[48];
    snprintf(full_title_id, sizeof(full_title_id), "EP9000-%s_00-0000000000000000", titleid);
    strncpy((char *)&head_bin[0x30], strlen(contentid) > 0 ? contentid : full_title_id, 48);

    // hmac of pkg header
    len = ntohl(*(uint32_t *)&head_bin[0xD0]);
    fpkg_hmac(&head_bin[0], len, hmac);
    memcpy(&head_bin[len], hmac, 16);

    // hmac of pkg info
    off = ntohl(*(uint32_t *)&head_bin[0x8]);
    len = ntohl(*(uint32_t *)&head_bin[0x10]);
    out = ntohl(*(uint32_t *)&head_bin[0xD4]);
    fpkg_hmac(&head_bin[off], len-64, hmac);
    memcpy(&head_bin[out], hmac, 16);

    // hmac of everything
    len = ntohl(*(uint32_t *)&head_bin[0xE8]);
    fpkg_hmac(&head_bin[0], len, hmac);
    memcpy(&head_bin[len], hmac, 16);

    // Make dir
    sceIoMkdir((VitaPackage::Package_path + "sce_sys/package").c_str(), 0777);

    // Write head.bin
    WriteFile((VitaPackage::Package_path + "sce_sys/package/head.bin").c_str(), head_bin, (int)&_binary_res_head_bin_size);

    free(head_bin);

    return 0;
}



//TITLE = STITLE TITLE_ID
int VitaPackage::SetSFOString(std::string title, std::string title_id) {
	// Read param.sfo
	if (title_id.length() != 9) {
		return -1;
	}
	char *sfo_buffer = nullptr;
	int res = allocateReadFile((VitaPackage::Package_path + "sce_sys/param.sfo").c_str(), &sfo_buffer);
	if (res < 0)
		return res;
	sceIoRemove((VitaPackage::Package_path + "sce_sys/param.sfo").c_str());
	setSfoString(sfo_buffer, "TITLE", title.c_str());
	setSfoString(sfo_buffer, "STITLE", title.c_str());
	setSfoString(sfo_buffer, "TITLE_ID", title_id.c_str());
	WriteFile((VitaPackage::Package_path + "sce_sys/param.sfo").c_str(), sfo_buffer, SFO_SIZE);

}
#define ntohl __builtin_bswap32
VitaPackage::VitaPackage(const std::string path) :
    path_(path)
{
	VitaPackage::Package_path = path;


    // ScePaf is required for PromoterUtil
    uint32_t ptr[0x100] = {0};
	ptr[0] = 0;
	ptr[1] = (uint32_t)&ptr[0];
	uint32_t scepaf_argp[] = {0x400000, 0xEA60, 0x40000, 0, 0};
    sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(scepaf_argp), scepaf_argp, (const SceSysmoduleOpt*)ptr);

    sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
    scePromoterUtilityInit();
}

VitaPackage::~VitaPackage()
{
    scePromoterUtilityExit();
    sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
}
/*
int VitaPackage::Install(InfoProgress progress)
{
    return Install(&progress);
}*/

int VitaPackage::Install()
{
    
    int ret = makeHeadBin();
    if (ret < 0) {
		utils::printInfo("Can't make head.bin for : 0x%08X\n", path_.c_str(), ret);
		return 0;
    }

    //InfoProgress progress3;
    //if(progress) progress3 = progress->Range(60, 100);
    ret = scePromoterUtilityPromotePkg(path_.c_str(), 0);
    if (ret < 0) {
		utils::printInfo("Can't Promote %s: scePromoterUtilityPromotePkgWithRif() = 0x%08X\n", path_.c_str(), ret);
		return 0;
    }

    int state = 0;
    unsigned int i = 0;
    do {
        ret = scePromoterUtilityGetState(&state);
        if (ret < 0) {
			utils::printInfo("Can't Promote %s: scePromoterUtilityGetState() = 0x%08X\n", path_.c_str(), ret);
			return 0;
        }

        i+= 1;
        //if (i<50 && progress) progress3.percent(i*2);
        sceKernelDelayThread(150 * 1000);
    } while (state);

    int result = 0;
    ret = scePromoterUtilityGetResult(&result);
    if (ret < 0) {
		utils::printInfo("Can't Promote %s: scePromoterUtilityGetResult() = 0x%08X\n", path_.c_str(), ret);
		return 0;
    }

    removePath(path_);

    //if(progress) progress->percent(100);
    return 1;
}
