#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H
#include <vitasdk.h>
#include <iostream>
#include <string>
int removePath(std::string path);
int hasEndSlash(const char *path);
int copyFile(const char *src_path, const char *dst_path);
int copyPath(const char *src_path, const char *dst_path);
int checkFolderExist(const char *folder);
int checkFileExist(const char *file);
std::string getFileName(std::string path);
void getSizeString(char string[16], uint64_t size);
//int getPathInfo(const char *path, uint64_t *size, uint32_t *folders,uint32_t *files, int(*handler)(const char *path));
int getPathInfo(char *path, uint64_t *size, uint32_t *folders, uint32_t *files);
#endif