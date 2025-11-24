#ifndef ALS_H
#define ALS_H

#define ALS_ADDR 0x52

#define ALS_REG_ID 0x06

#define ALS_ID 0xC2

int als_find(uint8_t *addr);
int als_get_id(uint8_t addr, uint8_t *id);

#endif //ALS_H
