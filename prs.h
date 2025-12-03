#ifndef PRS_H
#define PRS_H

#define PRS_ADDR     0x77
#define PRS_ADDR_ALT 0x76

#define PRS_MEAS_CFG 0x08
#define PRS_REG_ID   0x0D
#define RRS_COEF     0x10

#define PRS_COEF_RDY 0x80

#define PRS_ID 0x11

#define PRS_COEF_JIFFY 10 // ms
#define PRS_COEF_SIZE 21

typedef struct _prs_coefs {
	short c0;
	short c1;
	int c00;
	int c10;
	short c01;
	short c11;
	short c20;
	short c21;
	short c30;
	short c31;
	short c40;
} prs_coefs_t;

int prs_find(uint8_t *addr);
int prs_get_id(uint8_t addr, uint8_t *id);
int prs_check_coefs(uint8_t addr, bool *rdy);
int prs_wait_coefs(uint8_t addr, bool *rdy);
int prs_read_coefs(uint8_t addr, prs_coefs_t *coefs);

#endif //PRS_H
