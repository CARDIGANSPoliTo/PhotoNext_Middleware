/*******************************************************************************
* included libraries
*******************************************************************************/
#include "../../../include/CommonLibraries/libutils/utils.h"

/*******************************************************************************
* utils
*******************************************************************************/
void print_time2string(time_t ts, char *time_string, size_t str_len)
{
  struct tm ts_struct;

  ts_struct = *localtime(&ts);
  strftime(time_string, str_len, "%a %Y-%m-%d %H:%M:%S %Z", &ts_struct);

  return;
};

void dump_raw_message(uint8_t *buffer, size_t len)
{
  int i = 0;

  printf("DUMP RAW MESSAGE\n - LENGTH : %ld\n", len);
  printf(" - Message:");

  for(i=0; i<len; i++)
  {
    if(i%16 == 0)
    {
      printf("\n [0x%.4x] - ", i);
    }
    else if(i%4 == 0)
    {
      printf(" ");
    }

    printf("0x%.2x ", buffer[i]);
  }
  printf("\n");
};

/*******************************************************************************
* read/write functions
*******************************************************************************/
int read_64(void *buffer, uint64_t *data, endianness e)
{
  int i = 0;

  uint8_t *src, *dest;

  src = (uint8_t *) buffer;
  dest = (uint8_t *) data;

  for(i=0; i< sizeof(uint64_t); i++)
  {
    if(e == BE)
    {
      memset((void *)(dest + i), *(src + sizeof(uint64_t) - 1 - i), 1);
    }
    else
    {
      memset((void *)(dest + i), *(src + i), 1);
    }
  }

  return sizeof(uint64_t);
};

int read_32(void *buffer, uint32_t *data, endianness e)
{
  int i = 0;

  uint8_t *src, *dest;

  src = (uint8_t *) buffer;
  dest = (uint8_t *) data;

  for(i=0; i< sizeof(uint32_t); i++)
  {
    if(e == BE)
    {
      memset((void *)(dest + i), *(src + sizeof(uint32_t) - 1 - i), 1);
    }
    else
    {
      memset((void *)(dest + i), *(src + i), 1);
    }
  }

  return sizeof(uint32_t);
};

int read_16(void *buffer, uint16_t *data, endianness e)
{
  int i = 0;

  uint8_t *src, *dest;

  src = (uint8_t *) buffer;
  dest = (uint8_t *) data;

  for(i=0; i< sizeof(uint16_t); i++)
  {
    if(e == BE)
    {
      memset((void *)(dest + i), *(src + sizeof(uint16_t) - 1 - i), 1);
    }
    else
    {
      memset((void *)(dest + i), *(src + i), 1);
    }
  }

  return sizeof(uint16_t);
};

int read_n(void *buffer, void *data, size_t size, endianness e)
{
  int i = 0;

  uint8_t *src, *dest;

  src = (uint8_t *) buffer;
  dest = (uint8_t *) data;

  switch (size) {
    case sizeof(uint8_t):
    case sizeof(uint16_t):
    case sizeof(uint32_t):
      for(i=0; i< size; i++)
      {
        if(e == BE)
        {
          memset((void *)(dest + i), *(src + size - 1 - i), 1);
        }
        else
        {
          memset((void *)(dest + i), *(src + i), 1);
        }
      }
      break;
  }

  return size;
};

int read_8(void *buffer, uint8_t *data)
{
  uint8_t *src = (uint8_t *) buffer;

  memset((void *)data, *src, 1);

  return sizeof(uint8_t);
};

int write_32(uint32_t *data, void* buffer, endianness e)
{
  int i = 0;

  uint8_t *src, *dest;

  src = (uint8_t *) data;
  dest = (uint8_t *) buffer;

  for(i=0; i < sizeof(uint32_t); i++)
  {
    if(e == BE)
    {
      memset((void *)(dest + i), *(src + sizeof(uint32_t) - 1 - i), 1);
    }
    else
    {
      memset((void *)(dest + i), *(src + i), 1);
    }
  }

  return sizeof(uint32_t);
};

int write_64(uint64_t *data, void* buffer, endianness e)
{
  int i = 0;

  uint8_t *src, *dest;

  src = (uint8_t *) data;
  dest = (uint8_t *) buffer;

  for(i=0; i < sizeof(uint64_t); i++)
  {
    if(e == BE)
    {
      memset((void *)(dest + i), *(src + sizeof(uint64_t) - 1 - i), 1);
    }
    else
    {
      memset((void *)(dest + i), *(src + i), 1);
    }
  }

  return sizeof(uint64_t);
};


int write_16(uint16_t *data, void* buffer, endianness e)
{
  int i = 0;

  uint8_t *src, *dest;

  src = (uint8_t *) data;
  dest = (uint8_t *) buffer;

  for(i=0; i < sizeof(uint16_t); i++)
  {
    if(e == BE)
    {
      memset((void *)(dest + i), *(src + sizeof(uint16_t) - 1 - i), 1);
    }
    else
    {
      memset((void *)(dest + i), *(src + i), 1);
    }
  }

  return sizeof(uint16_t);
};

int write_8(uint8_t *data, void* buffer)
{
  uint8_t *dest;

  dest = (uint8_t *) buffer;
  memset((void *)dest, *data, 1);

  return sizeof(uint8_t);
};
