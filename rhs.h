#ifndef RHS_H
#define RHS_H

#define RHS_ADDR 0x43

#define RHS_REG_PART_ID    0x00
#define RHS_REG_DIE_REV    0x02
#define RHS_REG_UID        0x04
#define RHS_REG_SYS_CTRL   0x10
#define RHS_REG_SYS_STAT   0x11
#define RHS_REG_SENS_START 0x22
#define RHS_REG_T_VAL      0x30
#define RHS_REG_H_VAL      0x33

#define RHS_SYS_CTRL_RESET     0x80
#define RHS_SYS_CTRL_LOW_POWER 0x01

#define RHS_SYS_STAT_ACTIVE 0x01

#define RHS_T_BOOTING 1250 // 1.25 ms

#define RHS_ID  0x0210
#define RHS_REV 0x0002

#define RHS_T_START 0x01
#define RHS_H_START 0x02

#define RHS_T_CONV_TH_SINGLE 130000 // 130 ms

int rhs_find(uint8_t *addr);
int rhs_activate(uint8_t addr);
int rhs_deactivate(uint8_t addr);
int rhs_get_id(uint8_t addr, uint16_t *id, uint16_t *rev, uint64_t *uid);

uint8_t rhs_crc7(uint32_t x);
int rhs_start(uint8_t addr);
int rhs_read_values(uint8_t addr,
                    uint32_t *t_data, bool *t_valid, uint8_t *t_crc,
                    uint32_t *h_data, bool *h_valid, uint8_t *h_crc);

#endif //RHS_H
