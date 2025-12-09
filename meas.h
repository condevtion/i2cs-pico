#ifndef MEAS_H
#define MEAS_H

#define PRS_INT_PIN 22

void int_setup(uint gpio);

void setup_prs(uint8_t addr, prs_coefs_t *prs_coefs_t, int32_t *prs_k, int32_t *tmp_k);
void measure_prs(uint8_t addr, size_t n, const prs_coefs_t *coefs, int32_t prs_k, int32_t tmp_k);


#endif //MEAS_H
