#include "mf.h"

#include <stdint.h>

static uint8_t _mf_data[MF_FLASH_BLOCK_SIZE];
static mf_flash_info_t *mf_data = (mf_flash_info_t *)_mf_data;
static mf_flash_info_t *info_main = (mf_flash_info_t *)MF_FLASH_MAIN_ADDR;
static mf_flash_info_t *info_backup = (mf_flash_info_t *)MF_FLASH_BACKUP_ADDR;

static void mf_init_block(mf_flash_info_t *block) {
  mf_erase((uint32_t)block);
  mf_write((uint32_t)block, _mf_data);
}

static bool mf_block_inited(mf_flash_info_t *block) {
  return block->header == MF_FLASH_HEADER;
}

static bool mf_block_empty(mf_flash_info_t *block) {
  return block->key.name_length == 0;
}

static bool mf_block_err(mf_flash_info_t *block) {
  return ((uint8_t *)block)[MF_FLASH_BLOCK_SIZE - 1] != 0x56;
}

void mf_init() {
  mf_flash_info_t info = {
      .header = MF_FLASH_HEADER,
      .key = {.next_key = false, .name_length = 0, .data_size = 0}};

  memset(_mf_data, -1, MF_FLASH_BLOCK_SIZE);
  memcpy(_mf_data, &info, sizeof(info));
  _mf_data[MF_FLASH_BLOCK_SIZE - 1] = 0x56;

  if (!mf_block_inited(info_backup) || mf_block_err(info_backup)) {
    mf_init_block(info_backup);
  }

  if (!mf_block_inited(info_main)) {
    mf_init_block(info_main);
  }

  if (mf_block_err(info_main)) {
    if (mf_block_empty(info_backup)) {
      mf_init_block(info_main);
    } else {
      mf_write((uint32_t)info_main, info_backup);
    }
  }

  memcpy(_mf_data, info_main, MF_FLASH_BLOCK_SIZE);
}

void mf_save() {
  mf_erase((uint32_t)(info_backup));
  mf_write((uint32_t)info_backup, info_main);
  mf_erase((uint32_t)info_main);
  mf_write((uint32_t)info_main, _mf_data);
}

void mf_load() { memcpy(_mf_data, info_main, MF_FLASH_BLOCK_SIZE); }

static size_t mf_get_key_size(mf_key_info_t *key) {
  return sizeof(mf_key_info_t) + key->data_size + key->name_length;
}

static mf_key_info_t *mf_get_next_key(mf_key_info_t *key) {
  return (mf_key_info_t *)(((uint8_t *)key) + mf_get_key_size(key));
}

static mf_key_info_t *mf_get_last_key(mf_flash_info_t *block) {
  if (mf_block_empty(block)) {
    return NULL;
  }

  mf_key_info_t *ans = &block->key;

  while (ans->next_key) {
    ans = mf_get_next_key(ans);
  }

  return ans;
}

mf_status_t mf_add_key(const char *name, void *data, size_t size) {
  if (mf_search_key(name) != NULL) {
    return mf_set_key(name, data, size);
  }

  size_t name_len = strlen(name) + 1;

  mf_key_info_t *key_buf = NULL;
  uint8_t *data_name_buf = NULL;
  mf_key_info_t *last_key = NULL;

  if (mf_block_empty(mf_data)) {
    key_buf = &mf_data->key;
    data_name_buf = (uint8_t *)&mf_data->key + sizeof(mf_key_info_t);
  } else {
    last_key = mf_get_last_key(mf_data);
    key_buf =
        (mf_key_info_t *)(((uint8_t *)last_key) + mf_get_key_size(last_key));
    data_name_buf = (uint8_t *)key_buf + sizeof(mf_key_info_t);
  }

  if ((uint8_t *)key_buf + sizeof(mf_key_info_t) + size + name_len >
      _mf_data + MF_FLASH_BLOCK_SIZE - 1) {
    return MF_ERR_FULL;
  }

  memcpy(data_name_buf, name, name_len);
  data_name_buf += name_len;
  memcpy(data_name_buf, data, size);

  *key_buf = (mf_key_info_t){
      .next_key = false, .name_length = name_len, .data_size = size};

  if (last_key != NULL) {
    last_key->next_key = true;
  }

  return MF_OK;
}

const char *mf_get_key_name(mf_key_info_t *key) {
  return (char *)((uint8_t *)key + sizeof(mf_key_info_t));
}

uint8_t *mf_get_key_data(mf_key_info_t *key) {
  return ((uint8_t *)key + sizeof(mf_key_info_t) + key->name_length);
}

mf_key_info_t *mf_search_key(const char *name) {
  if (mf_block_empty(mf_data)) {
    return NULL;
  }

  mf_key_info_t *ans = &mf_data->key;

  while (ans->next_key) {
    if (strcmp(name, mf_get_key_name(ans)) == 0) {
      return ans;
    }
    ans = mf_get_next_key(ans);
  }

  if (strcmp(name, mf_get_key_name(ans)) == 0) {
    return ans;
  } else {
    return NULL;
  }
}

mf_status_t mf_set_key(const char *name, const void *data, size_t size) {
  mf_key_info_t *key = mf_search_key(name);
  if (key == NULL) {
    return MF_ERR_NULL;
  }

  if (key->data_size != size) {
    return MF_ERR;
  }

  memcpy(mf_get_key_data(key), data, size);

  return MF_OK;
}

void mf_foreach(bool (*fun)(mf_key_info_t *key, void *arg), void *arg) {
  if (mf_block_empty(mf_data)) {
    return;
  }

  mf_key_info_t *ans = &mf_data->key;

  while (ans->next_key) {
    if (!fun(ans, arg)) {
      return;
    }
    ans = mf_get_next_key(ans);
  }

  fun(ans, arg);

  return;
}
