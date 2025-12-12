#ifndef MEAS_H
#define MEAS_H

#define PRS_INT_PIN 22

void int_setup(uint gpio);

void setup_prs(uint8_t addr, prs_coefs_t *prs_coefs_t, int32_t *prs_k, int32_t *tmp_k);
void start_measure_prs(uint8_t addr);
void read_prs_data(uint8_t addr, size_t n, const prs_coefs_t *coefs, int32_t prs_k, int32_t tmp_k);

void start_measure_rhs(uint8_t addr, absolute_time_t *deadline, absolute_time_t *start);
void read_rhs_data(uint8_t addr, absolute_time_t deadline, absolute_time_t start);

#endif //MEAS_H
