#include "fs_interface.h"

// Function prototypes
int littlefs_stat(const char *path, size_t *size, time_t *mtime) {
  File f = LittleFS.open(path, "r");
  if (!f) return 0;
  if (size) *size = f.size();
  if (mtime) *mtime = f.getLastWrite();
  f.close();
  return 1;
}

void littlefs_ls(const char *path, void (*fn)(const char *, void *), void *data) {
  File dir = LittleFS.open(path, "r");
  if (!dir || !dir.isDirectory()) return;
  File file = dir.openNextFile();
  while (file) {
    fn(file.name(), data);
    file = dir.openNextFile();
  }
}

void *littlefs_op(const char *path, int flags) {
  const char *mode = (flags & MG_FS_WRITE) ? "w" : "r";
  File *f = new File(LittleFS.open(path, mode));
  if (!f->available()) {
    delete f;
    return NULL;
  }
  return (void *) f;
}

void littlefs_cl(void *fd) {
  File *f = (File *) fd;
  if (f) {
    f->close();
    delete f;
  }
}

size_t littlefs_rd(void *fd, void *buf, size_t len) {
  File *f = (File *) fd;
  return f->read((uint8_t *) buf, len);
}

size_t littlefs_wr(void *fd, const void *buf, size_t len) {
  File *f = (File *) fd;
  return f->write((const uint8_t *) buf, len);
}

size_t littlefs_sk(void *fd, size_t offset) {
  File *f = (File *) fd;
  return f->seek(offset);
}

bool littlefs_mv(const char *from, const char *to) {
  return LittleFS.rename(from, to);
}

bool littlefs_rm(const char *path) {
  return LittleFS.remove(path);
}

bool littlefs_mkd(const char *path) {
  return LittleFS.mkdir(path);
}
