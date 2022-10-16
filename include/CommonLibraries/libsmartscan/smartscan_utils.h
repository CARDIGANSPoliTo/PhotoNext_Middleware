/*******************************************************************************
* smartscan utils methods
*******************************************************************************/
#ifndef SMARTSCAN_UTILS_H
#define SMARTSCAN_UTILS_H

/*******************************************************************************
* included libraries
*******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include <syslog.h>

#include <libutils/utils.h>

#include "smartscan_constants.h"

/*******************************************************************************
* interrogator functions
*******************************************************************************/
int ssi_conf_init(SSI_CONFIG *conf);

void     get_net_if(SSI_CONFIG *conf, char *interface, ssize_t max_size);
void     get_smsc_ip(SSI_CONFIG *conf, char *ip, ssize_t max_size);
void     get_host_ip(SSI_CONFIG *conf, char *ip, ssize_t max_size);
void     get_subnet(SSI_CONFIG *conf, char *sn, ssize_t max_size);
void     get_gateway(SSI_CONFIG *conf, char *gw, ssize_t max_size);

/*******************************************************************************
* configurator functions
*******************************************************************************/
int ssi_read_config(SSI_CONFIG *cfg);
int ssi_check_pair(SSI_CONFIG *cfg, const char *key_p, const char *value_p);

/*******************************************************************************
* parsing functions
*******************************************************************************/

// laser channel to peak conversion
double ssi_laser_channel_2_peak(uint32_t data);

/*******************************************************************************
* message creation functions
*******************************************************************************/
int ssi_create_diagnostic_msg(uint8_t *buffer, size_t len, uint8_t state);
// int ssi_create_maintenance_msg(uint8_t *buffer, size_t len, SSI_CONFIG *conf);

/*******************************************************************************
* maintenance message functions
* - all functions get a pointer to a uint8_t buffer (message to be sent)
*   user must pass the pointer to the next byte to write
* - other parameters could be necessary, depending on the command (see manual)
* - all functions return a size_t integer, that is the number of bytes written
*******************************************************************************/
size_t ssi_write_maint_header(uint8_t *message);
size_t ssi_write_maint_padding(uint8_t *message, size_t current_len);

size_t ssi_write_state(uint8_t *message, uint8_t state);
size_t ssi_write_demo(uint8_t *message, uint8_t mode);
size_t ssi_write_raw_data_scan(uint8_t *message, uint16_t N); // a number between 0 and 32
size_t ssi_write_cont_tx_rate(uint8_t *message, uint16_t N); // a number between 0 and 255
size_t ssi_write_ch_format(uint8_t *message, uint8_t cpu_read_mode, uint8_t dsp_write_mode, uint8_t gratings, uint8_t channels);
size_t ssi_write_channel_th(uint8_t *message, uint8_t channel, uint16_t threshold);
size_t ssi_write_all_channel_th(uint8_t *message, uint16_t threshold);
size_t ssi_write_start_laser_ch(uint8_t *message, uint16_t lc);
size_t ssi_write_scan_speed(uint8_t *message, uint8_t mode, uint16_t scan, uint16_t cycle);
size_t ssi_write_ip_add(uint8_t *message, uint32_t ip);  // 4 bytes address
size_t ssi_write_netmask(uint8_t *message, uint32_t nm);  // 4 bytes address
size_t ssi_write_gateway(uint8_t *message, uint32_t gw);  // 4 bytes address
size_t ssi_write_gain_slot(uint8_t *message, uint8_t channel, uint8_t type, uint8_t slot, uint8_t data);
size_t ssi_write_gain_slot_single(uint8_t *message, uint8_t channel, uint8_t slot, uint16_t data);
size_t ssi_write_agc(uint8_t *message, uint16_t data);
size_t ssi_write_utc_local_time(uint8_t *message);

/*******************************************************************************
* dump functions
*******************************************************************************/
void ssi_dump_config(SSI_CONFIG *conf);

void ssi_dump_diagnostic_header(HD_DIAGNOSTIC *header);

void ssi_dump_maintenance_header(HD_MAINTENANCE *header);
void ssi_dump_maintenance_command(uint8_t cmd, uint8_t *data);

void ssi_dump_cont_data_header(HD_CONT_DATA *header);
void ssi_dump_scan_data_header(HD_SCAN_DATA *header);

void ssi_dump_data(SSI_DATA *data);

int ssi_print_state(uint8_t state, char *buffer, size_t len);

#endif
