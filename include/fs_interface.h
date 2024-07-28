#ifndef FS_INTERFACE_H
#define FS_INTERFACE_H

#include <FS.h>
#include <LittleFS.h>

#include <mongoose_config.h>
#include <mongoose.h>

int littlefs_stat(const char *path, size_t *size, time_t *mtime);
void littlefs_ls(const char *path, void (*fn)(const char *, void *), void *data);
void *littlefs_op(const char *path, int flags);
void littlefs_cl(void *fd);
size_t littlefs_rd(void *fd, void *buf, size_t len);
size_t littlefs_wr(void *fd, const void *buf, size_t len);
size_t littlefs_sk(void *fd, size_t offset);
bool littlefs_mv(const char *from, const char *to);
bool littlefs_rm(const char *path);
bool littlefs_mkd(const char *path);

#endif