#ifndef _HTTPCLIENT_H_
#define _HTTPCLIENT_H_

#include <stdint.h>

typedef struct {
    uint8_t* bufptr;
    uint32_t buflen;
} https_upload_t;

void https_upload(https_upload_t* content);

#endif