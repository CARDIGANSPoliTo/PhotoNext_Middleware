#ifndef UTILS_H
#define UTILS_H

/*******************************************************************************
* included libraries
*******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*******************************************************************************
* error codes (TODO: move to dedicated header)
*******************************************************************************/
#define STATUS_OK 0
#define STATUS_ERROR 1

/*******************************************************************************
* log levels (TODO: move to dedicated header)
*******************************************************************************/
#define MAX_LOG_MSG_SIZE 200
#define MAX_LOG_LEVEL LOG_DEBUG

/*******************************************************************************
* endianness constants
*******************************************************************************/
typedef enum
{
  BE, // big endian
  LE  // little endian
} endianness;

/*******************************************************************************
* utils
*******************************************************************************/
void print_time2string(time_t ts, char *time_string, size_t str_len);
void dump_raw_message(uint8_t *buffer, size_t len);

/*******************************************************************************
* read/write functions
*******************************************************************************/
int read_64(void *buffer, uint64_t *data, endianness e);
int read_32(void *buffer, uint32_t *data, endianness e);
int read_16(void *buffer, uint16_t *data, endianness e);
int read_8(void *buffer, uint8_t *data);
int read_n(void *buffer, void *data, size_t size, endianness e);

int write_64(uint64_t *data, void* buffer, endianness e);
int write_32(uint32_t *data, void* buffer, endianness e);
int write_16(uint16_t *data, void* buffer, endianness e);
int write_8(uint8_t *data, void* buffer);

#endif
