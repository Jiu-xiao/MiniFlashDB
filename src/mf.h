#ifndef __MF_H__
#define __MF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mf_config.h"

#define MF_FLASH_HEADER 0x12345678

typedef struct __attribute__((packed)) {
  uint8_t next_key;
  uint32_t name_length : 24;
  size_t data_size;
} mf_key_info_t;

typedef struct {
  uint32_t header;
  mf_key_info_t key;
} mf_flash_info_t;

typedef enum {
  MF_OK,
  MF_ERR,
  MF_ERR_FULL,
  MF_ERR_NULL,
  MF_ERR_EXIST
} mf_status_t;

void mf_init();

void mf_save();

void mf_load();

mf_key_info_t *mf_search_key(const char *name);

mf_status_t mf_add_key(const char *name, void *data, size_t size);

mf_status_t mf_set_key(const char *name, const void *data, size_t size);

uint8_t *mf_get_key_data(mf_key_info_t *key);

const char *mf_get_key_name(mf_key_info_t *key);

void mf_foreach(bool (*fun)(mf_key_info_t *key, void *arg), void *arg);

#ifdef __cplusplus
}
#endif

#endif
