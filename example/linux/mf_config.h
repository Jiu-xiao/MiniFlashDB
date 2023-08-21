#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint8_t flash_1[2048], flash_2[2048];

/* 一块FLASH空间的大小 */
#define MF_FLASH_BLOCK_SIZE (2048)

/* 主FLASH地址 */
#define MF_FLASH_MAIN_ADDR (flash_1)

/* 备份FLASH地址 */
#define MF_FLASH_BACKUP_ADDR (flash_2)

/* Flash读写函数 */
#define MF_ERASE mf_erase
#define MF_WRITE mf_write

static void mf_erase(void *addr) { memset(addr, 0, MF_FLASH_BLOCK_SIZE); }

static void mf_write(void *addr, void *buf) {
  memcpy(addr, buf, MF_FLASH_BLOCK_SIZE);
}
