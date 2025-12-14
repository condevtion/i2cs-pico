#ifndef MEAS_H
#define MEAS_H

#define PRS_INT_PIN 22

void int_setup(uint gpio);

void setup_prs(uint8_t addr, prs_coefs_t *prs_coefs_t, int32_t *prs_k, int32_t *tmp_k);
void start_measure_prs(uint8_t addr, absolute_time_t *start);
void start_measure_prs_tmp(uint8_t addr, absolute_time_t *start);
void read_prs_raw_data(uint8_t addr, int32_t k, float *p_raw_sc, absolute_time_t start);
void read_prs_tmp_raw_data(uint8_t addr, int32_t tmp_k, float *t_raw_sc, absolute_time_t start);
void calc_prs(uint8_t addr, const prs_coefs_t *coefs, float t_raw_sc, float p_raw_sc);

void start_measure_rhs(uint8_t addr, absolute_time_t *deadline, absolute_time_t *start);
void read_rhs_data(uint8_t addr, absolute_time_t deadline, absolute_time_t start);

void probe_als(uint8_t addr, uint8_t *gain);
void start_measure_als(uint8_t addr, bool rgb, uint8_t gain, uint8_t res,
                       absolute_time_t *deadline, absolute_time_t *start);
void read_als_light_data(uint8_t addr, uint8_t gain, uint8_t res,
                         absolute_time_t deadline, absolute_time_t start);
void read_als_color_data(uint8_t addr, absolute_time_t deadline, absolute_time_t start);

#endif //MEAS_H
