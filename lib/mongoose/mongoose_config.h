#ifndef MONGOOSE_CONFIG_H
#define MONGOOSE_CONFIG_H

#define MG_ARCH MG_ARCH_ESP32

#define MG_ENABLE_EPOLL 0
#define MG_FATFS_ROOT "/"
#define MG_HTTP_INDEX "index.html"
#define MG_TLS MG_TLS_BUILTIN	// change to 'MG_TLS_MBED' to enable TLS
#define MG_OTA MG_OTA_CUSTOM
#define MG_DEVICE MG_DEVICE_CUSTOM

#endif