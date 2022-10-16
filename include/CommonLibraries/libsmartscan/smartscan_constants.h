#ifndef SMARTSCAN_CONSTANTS_H
#define SMARTSCAN_CONSTANTS_H

/*******************************************************************************
* libraries
*******************************************************************************/
#include <net/if.h>

/*******************************************************************************
* SSI constants
*******************************************************************************/
#define MSG_TYPE_DIAG 0
#define MSG_TYPE_MAIN 1
#define MSG_TYPE_CONT 2
#define MSG_TYPE_SCAN 3

#define PORT_TX_HOST   30000  // 300xx
#define PORT_TX_CLIENT 30071

#define PORT_RX_DIAG 30001    // client receives on the same port
#define PORT_RX_MAIN 30070    // client receives on the same port
#define PORT_RX_CONT 30002
#define PORT_RX_SCAN 30072

#define SSI_STATE_NO_CHANGE   0
#define SSI_STATE_STAND_BY    1
#define SSI_STATE_OPERATIONAL 2
#define SSI_STATE_MAINTENANCE 3
#define SSI_STATE_TEST        4

#define LASER_CHANNEL_MULT  128
#define SPEED_OF_LIGHT      299792458.0
#define CH0_FREQ            191125.0
#define SMARTSCAN_40_MULT   12.5
#define SMARTSCAN_35_MULT   11.1

/*******************************************************************************
* RFC1497 options:
* arranged in the standard RFC format: [CMD][LEN][...DATA...][...PAD...][TERM].
* Note that all data values (2 and 4 byte) are in network order (big endian).
* Byte defined arrays (such as MAC addresses) are in normal byte order.
*
* CMD  - 1 byte
* LEN  - 1 bytes
* DATA - LEN bytes
* PAD  - 32 bit alignment padding
* TERM - terminator byte (0xFF)
*******************************************************************************/
// SET COMMANDS

// set SSI system state
#define CMD_SET_STATE_CMD     1
#define CMD_SET_STATE_LEN     1
// set demo mode variable
#define CMD_SET_DEMO_MODE_CMD 2
#define CMD_SET_DEMO_MODE_LEN 1
// set raw scan rate
#define CMD_SET_SCAN_RATE_CMD 3
#define CMD_SET_SCAN_RATE_LEN 2
// set continuous scan rate
#define CMD_SET_CONT_RATE_CMD 4
#define CMD_SET_CONT_RATE_LEN 2
// set channel format same as RS232
#define CMD_SET_CH_FORMAT_CMD 5
#define CMD_SET_CH_FORMAT_LEN 2
// set all four channel threshold values to same value
#define CMD_SET_CH_THRESH_CMD 6
#define CMD_SET_CH_THRESH_LEN 2
// set scan beg (i.e. start channel frequency)
#define CMD_SET_SCAN_BEG_CMD  8
#define CMD_SET_SCAN_BEG_LEN  2
// set scan speed
#define CMD_SET_SCAN_SP_CMD   9
#define CMD_SET_SCAN_SP_LEN   2
// set ip address
#define CMD_SET_IP_ADDR_CMD   18
#define CMD_SET_IP_ADDR_LEN   4
// set subnet
#define CMD_SET_SUBNET_CMD    19
#define CMD_SET_SUBNET_LEN    4
// set gateway
#define CMD_SET_GATEWAY_CMD   21
#define CMD_SET_GATEWAY_LEN   4
// set threshold for channel 0
#define CMD_SET_CH0_THR_CMD   22
#define CMD_SET_CH0_THR_LEN   4
// set threshold for channel 1
#define CMD_SET_CH1_THR_CMD   23
#define CMD_SET_CH1_THR_LEN   4
// set threshold for channel 2
#define CMD_SET_CH2_THR_CMD   24
#define CMD_SET_CH2_THR_LEN   4
// set threshold for channel 3
#define CMD_SET_CH3_THR_CMD   25
#define CMD_SET_CH3_THR_LEN   4
// write one generic slot table entry
#define CMD_SET_GEN_SLOT_CMD  26
#define CMD_SET_GEN_SLOT_LEN  2
// write one slot position
#define CMD_SET_ONE_SLOT_CMD  27
#define CMD_SET_ONE_SLOT_LEN  4
// set AGC on plus update speed
#define CMD_SET_AGC_PLUS_CMD  29
#define CMD_SET_AGC_PLUS_LEN  2
// set CPUs UTC local time code
#define CMD_SET_CPU_UTC_CMD   32
#define CMD_SET_CPU_UTC_LEN   4

// RET COMMANDS

// return SSI state
#define CMD_RET_STATE_CMD     129
#define CMD_RET_STATE_LEN     1
// return demo mode
#define CMD_RET_DEMO_MODE_CMD 130
#define CMD_RET_DEMO_MODE_LEN 2
// return scancode tx
#define CMD_RET_SCAN_TX_CMD   131
#define CMD_RET_SCAN_TX_LEN   2
// return data code
#define CMD_RET_DATA_CODE_CMD 132
#define CMD_RET_DATA_CODE_LEN 2
// return channel format
#define CMD_RET_CH_FORMAT_CMD 133
#define CMD_RET_CH_FORMAT_LEN 2
// return four threshold values
#define CMD_RET_ALL_THR_CMD   134
#define CMD_RET_ALL_THR_LEN   2
// return debug value
#define CMD_RET_DBG_VAL_CMD   135
#define CMD_RET_DBG_VAL_LEN   2
// return first scan frequency
#define CMD_RET_SCAN_FREQ_CMD 136
#define CMD_RET_SCAN_FREQ_LEN 2
// return scan code
#define CMD_RET_SCAN_CODE_CMD 137
#define CMD_RET_SCAN_CODE_LEN 2
// return scan direction format
#define CMD_RET_SCAN_DIR_CMD  138
#define CMD_RET_SCAN_DIR_LEN  2
// return scan count
#define CMD_RET_SCAN_CNT_CMD  139
#define CMD_RET_SCAN_CNT_LEN  2
// return SW version
#define CMD_RET_SW_VER_CMD    140
#define CMD_RET_SW_VER_LEN    2
// return IP address
#define CMD_RET_IP_ADDR_CMD   146
#define CMD_RET_IP_ADDR_LEN   4
// return subnet mask
#define CMD_RET_SUBNET_CMD    147
#define CMD_RET_SUBNET_LEN    4
// return MAC address
#define CMD_RET_MAC_ADD_CMD   148
#define CMD_RET_MAC_ADD_LEN   6
// return gateway address
#define CMD_RET_GATEWAY_CMD   149
#define CMD_RET_GATEWAY_LEN   4
// return local UTC time
#define CMD_RET_UTC_CMD       160
#define CMD_RET_UTC_LEN       4
// return unit's serial number
#define CMD_RET_SERIAL_CMD    222
#define CMD_RET_SERIAL_LEN    4

/*******************************************************************************
* message contants
*******************************************************************************/
// header + payload limit (define by MTU)
#define MSG_LIMIT_MTU         1464

// header size
#define HD_DIAGNOSTIC_SIZE    30
#define HD_MAINTENANCE_SIZE   6
#define HD_CONT_DATA_SIZE     36
#define HD_SCAN_DATA_SIZE     36

// payload size
#define PL_DIAGNOSTIC_SIZE    0
#define PL_MAINTENANCE_SIZE   1023
#define PL_CONT_DATA_SIZE     MSG_LIMIT_MTU - HD_CONT_DATA_SIZE
#define PL_SCAN_DATA_SIZE     800

// message size
#define MSG_DIAGNOSTIC_SIZE   HD_DIAGNOSTIC_SIZE + PL_DIAGNOSTIC_SIZE
#define MSG_MAINTENANCE_SIZE  HD_MAINTENANCE_SIZE + PL_MAINTENANCE_SIZE
#define MSG_CONT_DATA_SIZE    HD_CONT_DATA_SIZE + PL_CONT_DATA_SIZE
#define MSG_SCAN_DATA_SIZE    HD_SCAN_DATA_SIZE + PL_SCAN_DATA_SIZE

/*******************************************************************************
* config file contants
*******************************************************************************/
#define CONFIG_FILE_PATH "/usr/local/etc/libsmartscan/smartscan_config.txt"
#define MAX_KEY_SIZE 100
#define MAX_VALUE_SIZE 100

/*******************************************************************************
* 
*******************************************************************************/
#define CHANNEL_MASK 0x000F
#define GRATING_MASK 0x01F0

/*******************************************************************************
* configuration structure
*******************************************************************************/
typedef struct SSI_CONFIG
{
  uint8_t  ssi_demo;                // smartscan demo mode 
  uint8_t  ssi_gratings;            // number of gratings per channel
  uint8_t  ssi_channels;            // number of channels
  uint16_t ssi_raw_speed;           // raw message speed
  uint16_t ssi_cont_speed;          // continous message speed
  uint16_t ssi_scan_speed;          // smartscan scan speed
  uint16_t ssi_first_fr;            // first scan frequency
  char     ssi_netif[IF_NAMESIZE];  // smartscan network interface
  char     ssi_smsc_ip[16];         // smartscan IP address
  char     ssi_host_ip[16];         // host IP address
  char     ssi_subnet[16];          // subnet mask
  char     ssi_gateway[16];         // gateway address
  uint32_t ssi_serial;              // smartscan serial
  uint8_t  ssi_log_level;           // log level
} SSI_CONFIG;

/*******************************************************************************
* UDP message fields
*******************************************************************************/
typedef struct HD_DIAGNOSTIC
{
  uint32_t  ulTimestamp;        // UTC timestamp based on epoch. big endian
  uint8_t   ucDamage;           // not used
  uint8_t   ucState;            // PC instructs SSI to go to a state
  uint8_t   ucvLevel1damage[8]; // not used
  uint8_t   ucvLevel2damage[8]; // not used
  uint8_t   ucvSpare[8];        // not used
} HD_DIAGNOSTIC;

typedef struct HD_MAINTENANCE
{
  uint32_t  ulCodeStamp;        // fixed cookie number defining message as maintenance (0xAA55E00E big endian)
  uint8_t   ucSpare;            // not used
  uint8_t   ucState;            // PC instructs SSI to go to a state
} HD_MAINTENANCE;

typedef struct HD_CONT_DATA
{
  uint16_t  usFrameSize;        // total Nr of bytes - 2 (big endian)
  uint8_t   ucHdrSizex4;        // nr of bytes in header / 4, fixed to 0x09
  uint8_t   ucFrameFormat;      // frame format (nr of gratings/nr of channels)
  uint32_t  ulFrameCount;       // auto incremented index (big endian)
  uint32_t  ulTimeStampH;       // UTC seconds H, scan time (big endian)
  uint32_t  ulTimeStampL;       // UTC seconds L, scan time (big endian), expressed as fraction of a second (i.e., 0x80000000 is half a second)
  uint32_t  ulTimeCodeH;        // UTC seconds, tx time (big endian)
  uint16_t  usTimeInterval;     // Sample interval in uS, time for one scan (big endian)
  uint16_t  usSpare;            // not used
  uint16_t  usMinChannel;       // TODO
  uint16_t  usMaxChannel;       // TODO
  uint32_t  ulMinWaveFreq;      // frequency of min channel (big endian)
  uint32_t  ulSpare;            // not used
} HD_CONT_DATA;

typedef struct HD_SCAN_DATA
{
  uint16_t  usFrameSize;        // total Nr of bytes - 2 (big endian), fixed to 834
  uint8_t   ucHdrSizex4;        // nr of bytes in header / 4, fixed to 0x09
  uint8_t   ucFrameFormat;      // frame format (nr of gratings/nr of channels), fixed to 0xFF or 0xFn
  uint32_t  ulFrameCount;       // auto incremented index (big endian)
  uint32_t  ulTimeStampH;       // UTC seconds H, scan time (big endian)
  uint32_t  ulTimeStampL;       // UTC seconds L, scan time (big endian)
  uint32_t  ulTimeCodeH;        // UTC seconds, tx time (big endian)
  uint16_t  usTimeInterval;     // Sample interval in uS, time for one scan (big endian)
  uint16_t  usNrSteps;          // steps per scan
  uint16_t  usMinChannel;       // TODO
  uint16_t  usMaxChannel;       // TODO
  uint32_t  ulMinWaveFreq;      // not used
  uint32_t  ulMaxWaveFreq;      // not used
} HD_SCAN_DATA;

typedef struct SSI_DATA
{
  uint32_t  timestamp_s;  // seconds since epoch
  uint32_t  timestamp_u;  // useconds
  uint8_t   channel;
  uint8_t   grating;
  uint16_t  peak_data;
} SSI_DATA;

/*******************************************************************************
* maintenance messages
*******************************************************************************/
static const uint8_t maintenance01[] = {0xaa, 0x55, 0xe0, 0x0e, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0xff};
static const uint8_t maintenance02[] = {0xaa, 0x55, 0xe0, 0x0e, 0x00, 0x00, 0xc9, 0x01, 0x00, 0x00, 0x00, 0xff};

static const uint8_t maintenance_config_1[] =
{
  0xaa, 0x55, 0xe0, 0x0e, 0x00, 0x00,
  // 0x02, 0x01, 0x00,  // demo mode
  0x05, 0x02, 0x41, 0x04,  // CPU, DSP, 16 grating, 4 channels
  0x04, 0x02, 0x00, 0x19,  // tx time 10 ms
  0x08, 0x02, 0x00, 0x00,  // start channel freq
  0x09, 0x02, 0x81, 0x90,  // 100 nr scans, 10 us
  0x01, 0x01, 0x02,  // set operational state
  // 0x00, 0x00,
  0xff
};

static const uint8_t maintenance_config_2[] =
{
  0xaa, 0x55, 0xe0, 0x0e, 0x00, 0x00,
  0x02, 0x01, 0x00,  // demo mode
  0x05, 0x02, 0x41, 0x04,  // CPU, DSP, 16 grating, 4 channels
  0x04, 0x02, 0x00, 0x05,  // tx time
  0x08, 0x02, 0x00, 0x00,  // start channel freq
  0x09, 0x02, 0x81, 0x90,  //nr scans
  0x01, 0x01, 0x02,  // set operational state
  0x00, 0x00, 0x00,  // padding
  0xff               // terminator byte
};

static const uint8_t maintenance_config_all[] =
{
  0xaa, 0x55, 0xe0, 0x0e, 0x00, 0x00,
  // 0x02, 0x01, 0x00,  // demo mode
  0x03, 0x02, 0x00, 0x20,  // 32 scan data per second
  0x04, 0x02, 0x00, 0x19,  // tx time 10 ms
  0x05, 0x02, 0x41, 0x04,  // CPU, DSP, 16 grating, 4 channels

  0x08, 0x02, 0x00, 0x00,  // start channel freq
  0x09, 0x02, 0x00, 0x1a,  // 100 nr scans, 10 us
  0x12, 0x04, 0x0a, 0x00, 0x00, 0x96,   // IP 10.0.0.150
  0x13, 0x04, 0xff, 0xff, 0xff, 0xff,   // subnet
  0x15, 0x04, 0x0a, 0x00, 0x00, 0x02,   // gateway

  0x16, 0x02, 0x26, 0x66,  // set th value
  0x17, 0x02, 0x26, 0x66,  // set th value
  0x18, 0x02, 0x26, 0x66,  // set th value
  0x19, 0x02, 0x26, 0x66,  // set th value

  // 0x1b, 0x04, 0x00, 0x08, 0x01, 0x90,   //one slot?
  0x01, 0x01, 0x02,  // set operational state
  0xff
};

/*******************************************************************************
* static vars
*******************************************************************************/
static const uint8_t command_code[128] =
{
  0, 1, 1, 1, 1, 1, 1, 1, // 128
  1, 1, 1, 1, 1, 0, 0, 0, // 136
  0, 0, 1, 1, 1, 1, 0, 0, // 144
  0, 0, 0, 0, 0, 0, 0, 0, // 152
  1, 0, 0, 0, 0, 0, 0, 0, // 160
  0, 0, 0, 0, 0, 0, 0, 0, // 168
  0, 0, 0, 0, 0, 0, 0, 0, // 176
  0, 0, 0, 0, 0, 0, 0, 0, // 184
  0, 0, 0, 0, 0, 0, 0, 0, // 192
  0, 0, 0, 0, 0, 0, 0, 0, // 200
  0, 0, 0, 0, 0, 0, 0, 0, // 208
  0, 0, 0, 0, 0, 0, 1, 0, // 216
  0, 0, 0, 0, 0, 0, 0, 0, // 224
  0, 0, 0, 0, 0, 0, 0, 0, // 232
  0, 0, 0, 0, 0, 0, 0, 0, // 240
  0, 0, 0, 0, 0, 0, 0, 0, // 284
};

static const uint8_t command_len[128] =
{
  0, 1, 2, 2, 2, 2, 2, 2, // 128
  2, 2, 2, 2, 2, 0, 0, 0, // 136
  0, 0, 4, 4, 6, 4, 0, 0, // 144
  0, 0, 0, 0, 0, 0, 0, 0, // 152
  4, 0, 0, 0, 0, 0, 0, 0, // 160
  0, 0, 0, 0, 0, 0, 0, 0, // 168
  0, 0, 0, 0, 0, 0, 0, 0, // 176
  0, 0, 0, 0, 0, 0, 0, 0, // 184
  0, 0, 0, 0, 0, 0, 0, 0, // 192
  0, 0, 0, 0, 0, 0, 0, 0, // 200
  0, 0, 0, 0, 0, 0, 0, 0, // 208
  0, 0, 0, 0, 0, 0, 4, 0, // 216
  0, 0, 0, 0, 0, 0, 0, 0, // 224
  0, 0, 0, 0, 0, 0, 0, 0, // 232
  0, 0, 0, 0, 0, 0, 0, 0, // 240
  0, 0, 0, 0, 0, 0, 0, 0, // 284
};

static const char *command_name[128] =
{
  "Not Valid", // 128
  "State",
  "Demo",
  "Scan Code TX",
  "Data Code TX",
  "Channel Format",
  "Thr values",
  "Debug Value",
  "First Scan freq", // 136
  "Scan code",
  "Scan direction",
  "Scan count",
  "SW version",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid", // 144
  "Not Valid",
  "IP address",
  "Subnet mask",
  "MAC address",
  "Gateway",
  "Not Valid",
  "Not Valid",
  "Not Valid", // 152
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "UTC Time", // 160
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid", // 168
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid", // 176
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid", // 184
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid", // 192
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid", // 200
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid", // 208
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid", // 216
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Serial number",
  "Not Valid",
  "Not Valid", // 224
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid", // 232
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid", // 240
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid", // 248
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
  "Not Valid",
};

#endif
