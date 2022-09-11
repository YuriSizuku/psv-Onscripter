#include "filesystem.h"
#include <psp2/paf/stdc.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define strcasecmp sce_paf_strcasecmp
#define strncasecmp sce_paf_strncasecmp

#define MAX_PATH_LENGTH 1024
#define MAX_NAME_LENGTH 256
#define MAX_SHORT_NAME_LENGTH 64

#define TRANSFER_SIZE (128 * 1024)
#define SCE_ERROR_ERRNO_EEXIST 0x80010011
#define SCE_ERROR_ERRNO_ENODEV 0x80010013

std::string getFileName(std::string path) {

	int pos = path.find_last_of('/');
	std::string s(path.substr(pos + 1));
	return s;

}

int hasEndSlash(const char *path) {
	return path[strlen(path) - 1] == '/';
}

int checkFileExist(const char *file) {
	SceIoStat stat;
	memset(&stat, 0, sizeof(SceIoStat));

	return sceIoGetstat(file, &stat) >= 0;
}
int checkFolderExist(const char *folder) {
	SceUID dfd = sceIoDopen(folder);
	if (dfd < 0)
		return 0;

	sceIoDclose(dfd);
	return 1;
}
int copyFile(const char *src_path, const char *dst_path) {
	// The source and destination paths are identical
	if (strcasecmp(src_path, dst_path) == 0) {
		return -1;
	}

	// The destination is a subfolder of the source folder
	int len = strlen(src_path);
	if (strncasecmp(src_path, dst_path, len) == 0 && (dst_path[len] == '/' || dst_path[len - 1] == '/')) {
		return -2;
	}

	SceUID fdsrc = sceIoOpen(src_path, SCE_O_RDONLY, 0);
	if (fdsrc < 0) {

		return fdsrc;
	}
	SceUID fddst = sceIoOpen(dst_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fddst < 0) {
		sceIoClose(fdsrc);
		return fddst;
	}

	void *buf = malloc( TRANSFER_SIZE);

	while (1) {
		int read = sceIoRead(fdsrc, buf, TRANSFER_SIZE);

		if (read < 0) {
			free(buf);

			sceIoClose(fddst);
			sceIoClose(fdsrc);

			sceIoRemove(dst_path);

			return read;
		}

		if (read == 0)
			break;

		int written = sceIoWrite(fddst, buf, read);

		if (written < 0) {
			free(buf);

			sceIoClose(fddst);
			sceIoClose(fdsrc);

			sceIoRemove(dst_path);

			return written;
		}


	}

	free(buf);

	// Inherit file stat
	SceIoStat stat;
	memset(&stat, 0, sizeof(SceIoStat));
	sceIoGetstatByFd(fdsrc, &stat);
	sceIoChstatByFd(fddst, &stat, 0x3B);

	sceIoClose(fddst);
	sceIoClose(fdsrc);

	return 1;
}

int copyPath(const char *src_path, const char *dst_path) {
	// The source and destination paths are identical
	if (strcasecmp(src_path, dst_path) == 0) {
		return -1;
	}

	// The destination is a subfolder of the source folder
	int len = strlen(src_path);
	if (strncasecmp(src_path, dst_path, len) == 0 && (dst_path[len] == '/' || dst_path[len - 1] == '/')) {
		return -2;
	}

	SceUID dfd = sceIoDopen(src_path);
	if (dfd >= 0) {
		SceIoStat stat;
		memset(&stat, 0, sizeof(SceIoStat));
		sceIoGetstatByFd(dfd, &stat);

		stat.st_mode |= SCE_S_IWUSR;

		int ret = sceIoMkdir(dst_path, 0777);
		if (ret < 0 && ret != SCE_ERROR_ERRNO_EEXIST) {
			sceIoDclose(dfd);
			return ret;
		}

		if (ret == SCE_ERROR_ERRNO_EEXIST) {
			sceIoChstat(dst_path, &stat, 0x3B);
		}

		int res = 0;

		do {
			SceIoDirent dir;
			memset(&dir, 0, sizeof(SceIoDirent));

			res = sceIoDread(dfd, &dir);
			if (res > 0) {
				char *new_src_path = (char*)malloc(strlen(src_path) + strlen(dir.d_name) + 2);
				snprintf(new_src_path, MAX_PATH_LENGTH - 1, "%s%s%s", src_path, hasEndSlash(src_path) ? "" : "/", dir.d_name);

				char *new_dst_path = (char*)malloc(strlen(dst_path) + strlen(dir.d_name) + 2);
				snprintf(new_dst_path, MAX_PATH_LENGTH - 1, "%s%s%s", dst_path, hasEndSlash(dst_path) ? "" : "/", dir.d_name);

				int ret = 0;

				if (SCE_S_ISDIR(dir.d_stat.st_mode)) {
					ret = copyPath(new_src_path, new_dst_path);
				}
				else {
					ret = copyFile(new_src_path, new_dst_path);
				}

				free(new_dst_path);
				free(new_src_path);

				if (ret <= 0) {
					sceIoDclose(dfd);
					return ret;
				}
			}
		} while (res > 0);

		sceIoDclose(dfd);
	}
	else {
		return copyFile(src_path, dst_path);
	}

	return 1;
}
// Path must end with '/'
int removePath(std::string path) {
	// sceIoDopen doesn't work if there is a '/' at the end
	if (path.back() == '/')
		path.pop_back();

	SceUID dfd = sceIoDopen(path.c_str());
	if (dfd >= 0) {
		path += "/";
		int res = 0;

		do {
			SceIoDirent dir;
			memset(&dir, 0, sizeof(SceIoDirent));

			res = sceIoDread(dfd, &dir);
			if (res > 0) {
                auto new_path = path + dir.d_name;

				if (SCE_S_ISDIR(dir.d_stat.st_mode)) {
					int ret = removePath(new_path);
					if (ret <= 0) {
						sceIoDclose(dfd);
						return ret;
					}
				} else {
					int ret = sceIoRemove(new_path.c_str());
					if (ret < 0) {
						sceIoDclose(dfd);
						return ret;
					}
				}
			}
		} while (res > 0);

		sceIoDclose(dfd);

		int ret = sceIoRmdir(path.c_str());
		if (ret < 0)
			return ret;
	} else {
		int ret = sceIoRemove(path.c_str());
		if (ret < 0)
			return ret;
	}

	return 1;
}
void getSizeString(char string[16], uint64_t size) {
	double double_size = (double)size;

	int i = 0;
	static const char *units[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
	while (double_size >= 1024.0) {
		double_size /= 1024.0;
		i++;
	}

	snprintf(string, 16, "%.*f %s", (i == 0) ? 0 : 2, double_size, units[i]);
}

int getPathInfo(char *path, uint64_t *size, uint32_t *folders, uint32_t *files) {
	SceUID dfd = sceIoDopen(path);
	if (dfd >= 0) {
		int res = 0;

		do {
			SceIoDirent dir;
			memset(&dir, 0, sizeof(SceIoDirent));

			res = sceIoDread(dfd, &dir);
			if (res > 0) {
				if (strcmp(dir.d_name, ".") == 0 || strcmp(dir.d_name, "..") == 0)
					continue;

				char *new_path = (char *)malloc(strlen(path) + strlen(dir.d_name) + 2);
				snprintf(new_path, MAX_PATH_LENGTH, "%s%s%s", path, hasEndSlash(path) ? "" : "/", dir.d_name);

				if (SCE_S_ISDIR(dir.d_stat.st_mode)) {
					int ret = getPathInfo(new_path, size, folders, files);
					if (ret <= 0) {
						free(new_path);
						sceIoDclose(dfd);
						return ret;
					}
				}
				else {
					if (size)
						(*size) += dir.d_stat.st_size;

					if (files)
						(*files)++;
				}

				free(new_path);
			}
		} while (res > 0);

		sceIoDclose(dfd);

		if (folders)
			(*folders)++;
	}
	else {
		if (size) {
			SceIoStat stat;
			memset(&stat, 0, sizeof(SceIoStat));

			int res = sceIoGetstat(path, &stat);
			if (res < 0)
				return res;

			(*size) += stat.st_size;
		}

		if (files)
			(*files)++;
	}

	return 1;
}