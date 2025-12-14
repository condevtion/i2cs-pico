#ifndef PRS_H
#define PRS_H

#define PRS_ADDR     0x77
#define PRS_ADDR_ALT 0x76

#define PRS_PRS      0x00
#define PRS_TMP      0x03
#define PRS_PRS_CFG  0x06
#define PRS_TMP_CFG  0x07
#define PRS_MEAS_CFG 0x08
#define PRS_CFG_REG  0x09
#define PRS_INT_STS  0x0A
#define PRS_REG_ID   0x0D
#define RRS_COEF     0x10

#define PRS_RATE_MASK 0x0f
#define PRS_PRC_MASK  0x07

#define PRS_RATE_1_MPS     0x00
#define PRS_RATE_2_MPS     0x10
#define PRS_RATE_4_MPS     0x20
#define PRS_RATE_8_MPS     0x30
#define PRS_RATE_16_MPS    0x40
#define PRS_RATE_32_MPS    0x50
#define PRS_RATE_64_MPS    0x60
#define PRS_RATE_128_MPS   0x70
#define PRS_RATE_25_16_MPS 0x80
#define PRS_RATE_25_8_MPS  0x90
#define PRS_RATE_25_4_MPS  0xa0
#define PRS_RATE_25_2_MPS  0xb0
#define PRS_RATE_25_MPS    0xc0
#define PRS_RATE_50_MPS    0xd0
#define PRS_RATE_100_MPS   0xe0
#define PRS_RATE_200_MPS   0xf0

#define PRS_PRC_1   0x00
#define PRS_PRC_2   0x01
#define PRS_PRC_4   0x02
#define PRS_PRC_8   0x03
#define PRS_PRC_16  0x04
#define PRS_PRC_32  0x05
#define PRS_PRC_64  0x06
#define PRS_PRC_128 0x07

#define PRS_JIFFY 3600 // Minimal measurement time - 3.6 ms

#define PRS_PRS_RDY  0x10
#define PRS_TMP_RDY  0x20
#define PRS_COEF_RDY 0x80

#define PRS_MEAS_CTRL_MASK 0x07

#define PRS_MEAS_CTRL_IDLE 0
#define PRS_MEAS_CTRL_P    1
#define PRS_MEAS_CTRL_T    2
#define PRS_MEAS_CTRL_CP   5
#define PRS_MEAS_CTRL_CT   6
#define PRS_MEAS_CTRL_CPT  7

#define PRS_PRC_SHIFT_CHK 0x04

#define PRS_CFG_SPI_MODE 0x01
#define PRS_CFG_FIFO_EN  0x02
#define PRS_CFG_P_SHIFT  0x04
#define PRS_CFG_T_SHIFT  0x08
#define PRS_CFG_INT_PRS  0x10
#define PRS_CFG_INT_TMP  0x20
#define PRS_CFG_INT_FIFO 0x40
#define PRS_CFG_INT_HL   0x80

#define PRS_INT_PRS       0x01
#define PRS_INT_TMP       0x02
#define PRS_INT_FIFO_FULL 0x04

#define PRS_ID 0x11

#define PRS_COEF_JIFFY 10 // ms
#define PRS_COEF_SIZE 21

typedef struct _prs_coefs {
	int16_t c0;
	int16_t c1;
	int32_t c00;
	int32_t c10;
	int16_t c01;
	int16_t c11;
	int16_t c20;
	int16_t c21;
	int16_t c30;
	int16_t c31;
	int16_t c40;
} prs_coefs_t;

int prs_find(uint8_t *addr);
int prs_get_id(uint8_t addr, uint8_t *id);
int prs_check_coefs(uint8_t addr, bool *rdy);
int prs_wait_coefs(uint8_t addr, bool *rdy);
int prs_read_coefs(uint8_t addr, prs_coefs_t *coefs);
int prs_config(uint8_t addr, uint8_t pm_prc, uint8_t tmp_prc);
int prs_meas_config(uint8_t addr);
int prs_meas_stop(uint8_t addr);
int prs_get_prs_raw(uint8_t addr, int32_t *prs);
int prs_get_tmp_raw(uint8_t addr, int32_t *tmp);

int32_t prs_prc_to_k(uint8_t prc);

#endif //PRS_H
