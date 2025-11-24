#ifndef PRS_H
#define PRS_H

#define PRS_ADDR     0x77
#define PRS_ADDR_ALT 0x76

#define PRS_REG_ID 0x0D

#define PRS_ID 0x11

int prs_find(uint8_t *addr);
int prs_get_id(uint8_t addr, uint8_t *id);

#endif //PRS_H
