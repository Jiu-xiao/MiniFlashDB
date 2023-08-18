#include "mf.h"

bool print_fn(mf_key_info_t *key, void *arg) {
  (void)arg;
  printf("%s ", mf_get_key_name(key));
  uint32_t *tmp = (uint32_t *)mf_get_key_data(key);

  for (int i = 0; i < key->data_size / 4; i++) {
    printf("%d ", tmp[i]);
  }
  printf("\n");

  return true;
}

int main() {
  mf_init();
  char key1_name[] = "key1";
  char key2_name[] = "keasdasy2";
  char key3_name[] = "keaasdasdy3";
  char key4_name[] = "keyaskdhasjh4";
  uint32_t key1_data[1] = {1};
  uint32_t key2_data[2] = {11, 22};
  uint32_t key3_data[3] = {111, 222, 333};
  uint32_t key4_data[4] = {1111, 2222, 3333, 4444};
  mf_add_key(key1_name, key1_data, sizeof(key1_data));
  mf_add_key(key2_name, key2_data, sizeof(key2_data));
  mf_add_key(key3_name, key3_data, sizeof(key3_data));
  mf_add_key(key4_name, key4_data, sizeof(key4_data));

  key4_data[1] = 123456;

  mf_set_key(key4_name, key4_data, sizeof(key4_data));

  mf_save();

  mf_load();

  mf_foreach(print_fn, NULL);
}
