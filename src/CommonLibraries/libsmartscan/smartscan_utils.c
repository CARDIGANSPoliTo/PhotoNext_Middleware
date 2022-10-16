/*******************************************************************************
* smartscan interrogator interface
*******************************************************************************/

/*******************************************************************************
* included libraries
*******************************************************************************/
#include "../../../include/CommonLibraries/libsmartscan/smartscan_utils.h"

/*******************************************************************************
* interrogator functions
*******************************************************************************/
int ssi_conf_init(SSI_CONFIG *conf)
{
  int ret_code = STATUS_OK;

  if(!conf)
  {
    syslog(LOG_ERR, "Invalid configuration structure");
    ret_code = STATUS_ERROR;
  }
  else
  {

    conf->ssi_demo        = 0;
    conf->ssi_raw_speed   = 0;
    conf->ssi_cont_speed  = 25;
    conf->ssi_gratings    = 16;
    conf->ssi_channels    = 4;
    conf->ssi_scan_speed  = 400;
    conf->ssi_first_fr    = 0;
    conf->ssi_serial      = 123456;
    conf->ssi_log_level   = 7;
    #if EMU_LOCAL==0
    strncpy(conf->ssi_netif, "eth0", IF_NAMESIZE);
    strncpy(conf->ssi_smsc_ip, "10.0.0.150", 16);
    strncpy(conf->ssi_host_ip, "10.0.0.2", 16);
    strncpy(conf->ssi_subnet, "255.255.255.0", 16);
    strncpy(conf->ssi_gateway, "10.0.0.2", 16);
    #endif // EMU_LOCAL
    #if EMU_LOCAL==1
    strncpy(conf->ssi_netif, "eth0", IF_NAMESIZE);
    strncpy(conf->ssi_smsc_ip, "127.0.0.1", 16);
    strncpy(conf->ssi_host_ip, "127.0.0.1", 16);
    strncpy(conf->ssi_subnet, "255.255.255.0", 16);
    strncpy(conf->ssi_gateway, "127.0.0.1", 16);
    #endif // EMU_LOCAL

  }
  ssi_dump_config(conf);
  return ret_code;
};

void     get_net_if(SSI_CONFIG *conf, char *interface, ssize_t max_size)
{
  if(!conf)
  {
    syslog(LOG_ERR, "Invalid configuration structure");
  }
  else
  {
    strncpy(interface, conf->ssi_netif, max_size);
  }

  return;
};

void     get_smsc_ip(SSI_CONFIG *conf, char *ip, ssize_t max_size)
{
  if(!conf)
  {
    syslog(LOG_ERR, "Invalid configuration structure");
  }
  else
  {
    strncpy(ip, conf->ssi_smsc_ip, max_size);
  }

  return;
};

void     get_host_ip(SSI_CONFIG *conf, char *ip, ssize_t max_size)
{
  if(!conf)
  {
    syslog(LOG_ERR, "Invalid configuration structure");
  }
  else
  {
    strncpy(ip, conf->ssi_host_ip, max_size);
  }

  return;
};

void     get_subnet(SSI_CONFIG *conf, char *sn, ssize_t max_size)
{
  if(!conf)
  {
    syslog(LOG_ERR, "Invalid configuration structure");
  }
  else
  {
    strncpy(sn, conf->ssi_subnet, max_size);
  }

  return;
};

void     get_gateway(SSI_CONFIG *conf, char *gw, ssize_t max_size)
{
  if(!conf)
  {
    syslog(LOG_ERR, "Invalid configuration structure");
  }
  else
  {
    strncpy(gw, conf->ssi_gateway, max_size);
  }

  return;
}

/*******************************************************************************
* configurator functions
*******************************************************************************/
int ssi_read_config(SSI_CONFIG *cfg) {

  /****************************** declarations ******************************/

  FILE *cfg_fp;                   // file pointer

  char *buffer = NULL;            // read buffer (will be allocated by getline)
  size_t len = 0;                 // len set to 0 tells getline to allocate buffer
  ssize_t rd_count;               // ret value of getline (-1  error or EOF)
  unsigned int line_count;        // config file line count

  char *key_p;                    // key pointer
  char *value_p;                  // value pointer

  int ret_value = STATUS_OK;    // return value (0 -> ok, 1 -> error)

  // tokenize
  char *ptr = NULL;
  char delim[] = " \n\t";        // split on whitespace, tab and new line

  // local configuration
  SSI_CONFIG local_cfg;

  /**************************************************************************/

  // load default config before reading from file (avoid unknown behavior if config file is corrupted)
  ssi_conf_init(cfg);
  memcpy(&local_cfg, cfg, sizeof(SSI_CONFIG));

  syslog(LOG_INFO, "Reading configuration from config file %s\n", CONFIG_FILE_PATH);

  // open config file
  cfg_fp = fopen(CONFIG_FILE_PATH, "r");

  if(cfg_fp == NULL) {       // file error
    syslog(LOG_WARNING, "Error while reading configuration file: %s\n Loading default configuration", strerror(errno));
    ret_value = STATUS_ERROR;
  }
  else {
    line_count = 1;

    while((rd_count = getline(&buffer, &len, cfg_fp)) != -1) {

      ptr = strtok(buffer, delim);    // split line

      // first chunk: read key
      if(ptr != NULL) {
        if(*ptr != '#') {           // skip comments (first char is a #)

          key_p = ptr;            // save pointer to key string

          ptr = strtok(NULL, delim);

          // second chunk: read value
          if(ptr != NULL) {

            value_p = ptr;      // save pointer to value string

            // check iv key value pair is a valid option
            if((ssi_check_pair(&local_cfg, key_p, value_p)) != STATUS_OK) {
              syslog(LOG_WARNING, "\tInvalid option at line %u\n", line_count);
              ret_value = STATUS_ERROR;
            }
          }
          else {
            syslog(LOG_WARNING, "\tInvalid option at line %u\n", line_count);
            ret_value = STATUS_ERROR;
          }

        }

      }
      line_count++;
    }

    // check error returned by getline
    if((rd_count == -1) && (errno == EINVAL || errno == ENOMEM)) {
      syslog(LOG_WARNING, "Error while reading configuration file: %s\n Loading default configuration", strerror(errno));
      ret_value = STATUS_ERROR;
    }

    // free buffer allocated by getline
    if(buffer != NULL) {
      free(buffer);
      buffer = NULL;
    }

    // close config file (can't be NULL, checked after fopen())
    fclose(cfg_fp);
    cfg_fp = NULL;
  }

  // if no error occurred, commit configuration changes
  if(ret_value == STATUS_OK) {
    *cfg = local_cfg;
  }

  return ret_value;
}

int ssi_check_pair(SSI_CONFIG *cfg, const char *key_p, const char *value_p) {
  // key value string pair
  char key[MAX_KEY_SIZE];
  char value[MAX_VALUE_SIZE];

  // used to convert option values
  uint8_t  u8_value;
  uint16_t u16_value;

  // return value
  int ret_value = STATUS_OK;

  /**************************************************************************/

  strncpy(key, key_p, MAX_KEY_SIZE);
  strncpy(value, value_p, MAX_VALUE_SIZE);

  if(strcmp(key, "GRATINGS") == 0) {
    syslog(LOG_INFO, "\tSet option %-12s%s\n", key, value);
    u8_value = (uint8_t) atoi(value);
    cfg->ssi_gratings = u8_value;
  }
  else if (strcmp(key, "CHANNELS") == 0) {
    syslog(LOG_INFO, "\tSet option %-12s%s\n", key, value);
    u8_value = (uint8_t) atoi(value);
    cfg->ssi_channels = u8_value;
  }
  else if (strcmp(key, "DEMO") == 0) {
    syslog(LOG_INFO, "\tSet option %-12s%s\n", key, value);
    u8_value = (uint8_t) atoi(value);
    cfg->ssi_demo = u8_value;
  }
  else if(strcmp(key, "RAW_SPEED") == 0) {
    syslog(LOG_INFO, "\tSet option %-12s%s\n", key, value);
    u16_value = (uint16_t) atoi(value);
    cfg->ssi_raw_speed;
  }
  else if(strcmp(key, "CONT_SPEED") == 0) {
    syslog(LOG_INFO, "\tSet option %-12s%s\n", key, value);
    u16_value = (uint16_t) atoi(value);
    cfg->ssi_cont_speed;
  }
  else if(strcmp(key, "SCAN_SPEED") == 0) {
    syslog(LOG_INFO, "\tSet option %-12s%s\n", key, value);
    u16_value = (uint16_t) atoi(value);
    cfg->ssi_scan_speed;
  }
  else if(strcmp(key, "FIRST_FREQ") == 0) {
    syslog(LOG_INFO, "\tSet option %-12s%s\n", key, value);
    u16_value = (uint16_t) atoi(value);
    cfg->ssi_first_fr;
  }
  else if(strcmp(key, "SSI_NETIF") == 0) {
    syslog(LOG_INFO, "\tSet option %-12s%s\n", key, value);
    strncpy(cfg->ssi_netif, value, 16);
  }
  else if(strcmp(key, "SSI_IP") == 0) {
    syslog(LOG_INFO, "\tSet option %-12s%s\n", key, value);
    strncpy(cfg->ssi_smsc_ip, value, 16);
  }
  else if(strcmp(key, "HOST_IP") == 0) {
    syslog(LOG_INFO, "\tSet option %-12s%s\n", key, value);
    strncpy(cfg->ssi_host_ip, value, 16);
  }
  else if(strcmp(key, "SUBNET_MASK") == 0) {
    syslog(LOG_INFO, "\tSet option %-12s%s\n", key, value);
    strncpy(cfg->ssi_subnet, value, 16);
  }
  else if(strcmp(key, "GATEWAY") == 0) {
    syslog(LOG_INFO, "\tSet option %-12s%s\n", key, value);
    strncpy(cfg->ssi_gateway, value, 16);
  }
  else if(strcmp(key, "LOG_LEVEL") == 0) {
    syslog(LOG_INFO, "\tSet option %-12s%s\n", key, value);
    cfg->ssi_log_level = (uint8_t) atoi(value);
  }
 else if(strcmp(key, "SERIAL") == 0) {
    syslog(LOG_INFO, "\tSet option %-12s%s\n", key, value);
    cfg->ssi_serial = (uint32_t) atoi(value);
  }
  else {  // no valid option
    ret_value = STATUS_ERROR;
  }
   
  return ret_value;
}


/*******************************************************************************
* parsing functions
*******************************************************************************/
double ssi_laser_channel_2_peak(uint32_t data)
{
  double lcn = 0.0;
  double peak_freq = 0.0;
  double wavelength = 0.0;

  if (data != 0) {
    lcn = (double)data / LASER_CHANNEL_MULT;
    peak_freq = (lcn * SMARTSCAN_40_MULT) + CH0_FREQ;
    wavelength = SPEED_OF_LIGHT / peak_freq;
  }

  return wavelength;
};

/*******************************************************************************
* message creation functions
*******************************************************************************/
int ssi_create_diagnostic_msg(uint8_t *buffer, size_t len, uint8_t state)
{
  time_t ts;
  uint8_t zero = 0;

  int error_code = STATUS_OK;
  syslog(LOG_DEBUG, "ssi_create_diagnostic_msg: create diagnostic message");

  if(!buffer)
  {
    syslog(LOG_DEBUG, "ssi_create_diagnostic_msg: buffer pointer is NULL");
    error_code = STATUS_ERROR;
  }

  if(!error_code && len != MSG_DIAGNOSTIC_SIZE)
  {
    syslog(LOG_DEBUG, "ssi_create_diagnostic_msg: buffer length is invalid");
    error_code = STATUS_ERROR;
  }

  if(!error_code)
  {
    memset((void *) buffer, 0, len);
    ts = time(NULL);

    buffer += write_32((uint32_t *) (&ts), (void *)buffer, BE);
    buffer += write_8(&zero, (void *)buffer);
    buffer += write_8(&state, (void *)buffer);
  }

  return error_code;
};

/*******************************************************************************
* maintenance message functions
* - all functions get a pointer to a uint8_t buffer (message to be sent)
*   user must pass the pointer to the next byte to write
* - other parameters could be necessary, depending on the command (see manual)
* - all functions return a size_t integer, that is the number of bytes written
*******************************************************************************/
size_t ssi_write_maint_header(uint8_t *message)
{
  uint32_t maintenance_code = 0xaa55e00e;
  uint8_t zero = 0x00;

  size_t increment = 0;

  increment += write_32(&maintenance_code, message + increment, BE);
  increment += write_8(&zero, message + increment);
  increment += write_8(&zero, message + increment);

  return increment;
};

size_t ssi_write_maint_padding(uint8_t *message, size_t current_len)
{
  uint8_t zero = 0x00;
  uint8_t term = 0xFF;

  size_t increment = 0;

  int padding = 4 - ((current_len - HD_MAINTENANCE_SIZE) % 4);

  while(padding > 1) // zero padding
  {
    increment += write_8(&zero, message + increment);
    padding--;
  }

  increment += write_8(&term, message + increment);

  return increment;
};

size_t ssi_write_state(uint8_t *message, uint8_t state)
{
  // command 001
  size_t increment = 0;

  uint8_t cmd = CMD_SET_STATE_CMD;
  uint8_t cmd_len = CMD_SET_STATE_LEN;

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);
  increment += write_8(&state, message + increment);

  return increment;
};

size_t ssi_write_demo(uint8_t *message, uint8_t mode)
{
  // command 002
  size_t increment = 0;

  uint8_t cmd = CMD_SET_DEMO_MODE_CMD;
  uint8_t cmd_len = CMD_SET_DEMO_MODE_LEN;

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);
  increment += write_8(&mode, message + increment);

  return increment;
};

size_t ssi_write_raw_data_scan(uint8_t *message, uint16_t N) // a number between 0 and 32
{
  // command 003
  size_t increment = 0;

  uint8_t cmd = CMD_SET_SCAN_RATE_CMD;
  uint8_t cmd_len = CMD_SET_SCAN_RATE_LEN;

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);
  increment += write_16(&N, message + increment, BE);

  return increment;
};

size_t ssi_write_cont_tx_rate(uint8_t *message, uint16_t N) // a number between 0 and 255
{
  // command 004
  size_t increment = 0;

  uint8_t cmd = CMD_SET_CONT_RATE_CMD;
  uint8_t cmd_len = CMD_SET_CONT_RATE_LEN;

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);
  increment += write_16(&N, message + increment, BE);

  return increment;
};

size_t ssi_write_ch_format(uint8_t *message, uint8_t cpu_read_mode, uint8_t dsp_write_mode, uint8_t gratings, uint8_t channels)
{
  // command 005
  size_t increment = 0;

  uint8_t cmd = CMD_SET_CH_FORMAT_CMD;
  uint8_t cmd_len = CMD_SET_CH_FORMAT_LEN;

  uint16_t data = 0x0000;

  // bit 15
  data |= (cpu_read_mode << 15); // uint8_t is promoted to uint16_t by specification

  // bit 14
  data |= (dsp_write_mode << 14);

  // bits 8 to 4
  data |= (gratings << 4);

  // bits 3 to 0
  data |= channels;

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);
  increment += write_16(&data, message + increment, BE);

  return increment;
};

size_t ssi_write_channel_th(uint8_t *message, uint8_t channel, uint16_t threshold)
{
  // command 022 - ch0
  // command 023 - ch1
  // command 024 - ch2
  // command 025 - ch3

  size_t increment = 0;

  uint8_t cmd = CMD_SET_CH0_THR_CMD;
  uint8_t cmd_len = CMD_SET_CH0_THR_LEN;

  switch(channel)
  {
    case 0:
      cmd = CMD_SET_CH0_THR_CMD;
      cmd_len = CMD_SET_CH0_THR_LEN;
      break;

    case 1:
      cmd = CMD_SET_CH1_THR_CMD;
      cmd_len = CMD_SET_CH1_THR_LEN;
      break;

    case 2:
      cmd = CMD_SET_CH2_THR_CMD;
      cmd_len = CMD_SET_CH2_THR_LEN;
      break;

    case 3:
      cmd = CMD_SET_CH3_THR_CMD;
      cmd_len = CMD_SET_CH3_THR_LEN;
      break;

    default:
      cmd = CMD_SET_CH0_THR_CMD;
      cmd_len = CMD_SET_CH0_THR_LEN;
      break;
  }

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);
  increment += write_16(&threshold, message + increment, BE);

  return increment;
};

size_t ssi_write_all_channel_th(uint8_t *message, uint16_t threshold)
{
  // command 006
  size_t increment = 0;

  uint8_t cmd = CMD_SET_CH_THRESH_CMD;
  uint8_t cmd_len = CMD_SET_CH_THRESH_LEN;

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);
  increment += write_16(&threshold, message + increment, BE);

  return increment;
};

size_t ssi_write_start_laser_ch(uint8_t *message, uint16_t lc)
{
  // command 008
  size_t increment = 0;

  uint8_t cmd = CMD_SET_SCAN_BEG_CMD;
  uint8_t cmd_len = CMD_SET_SCAN_BEG_LEN;

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);
  increment += write_16(&lc, message + increment, BE);

  return increment;
};

size_t ssi_write_scan_speed(uint8_t *message, uint8_t mode, uint16_t scan, uint16_t cycle)
{
  // command 009
  size_t increment = 0;

  uint8_t cmd = CMD_SET_SCAN_SP_CMD;
  uint8_t cmd_len = CMD_SET_SCAN_SP_LEN;

  uint16_t data = 0x0000;

  if(mode == 1)
  {
    // bits 12 to 10
    data |= (cycle << 10);
  }
  else
  {
    // bits 5 to 3
    data |= (cycle << 3);
  }

  // bits 2 to 0 or 9 to 0 (according to mode)
  data |= scan;

  // bit 15
  data |= (mode << 15);

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);
  increment += write_16(&data, message + increment, BE);

  return increment;
};

size_t ssi_write_ip_add(uint8_t *message, uint32_t ip) // 4 bytes address
{
  // command 018
  size_t increment = 0;

  uint8_t cmd = CMD_SET_IP_ADDR_CMD;
  uint8_t cmd_len = CMD_SET_IP_ADDR_LEN;

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);

  increment += write_32(&ip, message + increment, BE);

  return increment;
};

size_t ssi_write_netmask(uint8_t *message, uint32_t nm)  // 4 bytes address
{
  // command 019
  size_t increment = 0;

  uint8_t cmd = CMD_SET_SUBNET_CMD;
  uint8_t cmd_len = CMD_SET_SUBNET_LEN;

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);

  increment += write_32(&nm, message + increment, BE);

  return increment;
};

size_t ssi_write_gateway(uint8_t *message, uint32_t gw)  // 4 bytes address
{
  // command 021
  size_t increment = 0;

  uint8_t cmd = CMD_SET_GATEWAY_CMD;
  uint8_t cmd_len = CMD_SET_GATEWAY_LEN;

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);

  increment += write_32(&gw, message + increment, BE);

  return increment;
};

size_t ssi_write_gain_slot(uint8_t *message, uint8_t channel, uint8_t type, uint8_t slot, uint8_t data)
{
  // command 026
  size_t increment = 0;

  uint8_t cmd = CMD_SET_GEN_SLOT_CMD;
  uint8_t cmd_len = CMD_SET_GEN_SLOT_LEN;

  uint16_t value = 0x0000;

  // bits 15 to 14
  value |= (channel << 14);

  // bits 13 to 12
  value |= (type << 12);

  // bits 11 to 8
  value |= (slot << 8);

  value |= data;

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);

  increment += write_16(&value, message + increment, BE);

  return increment;
};

size_t ssi_write_gain_slot_single(uint8_t *message, uint8_t channel, uint8_t slot, uint16_t data)
{
  // command 027
  size_t increment = 0;

  uint8_t cmd = CMD_SET_ONE_SLOT_CMD;
  uint8_t cmd_len = CMD_SET_ONE_SLOT_LEN;

  uint32_t value = 0x00000000;

  // bits 21 to 20
  value |= (channel << 20);

  // bits 19 to 16
  value |= (slot << 16);

  value |= data;

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);

  increment += write_32(&value, message + increment, BE);

  return increment;
}

size_t ssi_write_agc(uint8_t *message, uint16_t data)
{
  // command 029
  size_t increment = 0;

  uint8_t cmd = CMD_SET_AGC_PLUS_CMD;
  uint8_t cmd_len = CMD_SET_AGC_PLUS_LEN;

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);
  increment += write_16(&data, message + increment, BE);

  return increment;
};

size_t ssi_write_utc_local_time(uint8_t *message)
{
  // command 029
  size_t increment = 0;

  uint8_t cmd = CMD_SET_CPU_UTC_CMD;
  uint8_t cmd_len = CMD_SET_CPU_UTC_LEN;

  time_t lt = time(NULL);

  increment += write_8(&cmd, message + increment);
  increment += write_8(&cmd_len, message + increment);
  increment += write_32((uint32_t *) (&lt), message + increment, BE);

  return increment;
};

/*******************************************************************************
* dump functions
*******************************************************************************/
void ssi_dump_config(SSI_CONFIG *conf)
{
  int i;
  printf("----------------------------------------------------------\n");
  printf("*               SSI CONFIGURATION                        *\n");
  printf("----------------------------------------------------------\n");
  printf(" - ssi_demo        : %d\n",     conf->ssi_demo       );
  printf(" - ssi_gratings    : %d\n",     conf->ssi_gratings   );
  printf(" - ssi_channels    : %d\n",     conf->ssi_channels   );
  printf(" - ssi_raw_speed   : %d\n",     conf->ssi_raw_speed  );
  printf(" - ssi_cont_speed  : %d\n",     conf->ssi_cont_speed );
  printf(" - ssi_scan_speed  : %d\n",     conf->ssi_scan_speed );
  printf(" - ssi_first_fr    : %d\n",     conf->ssi_first_fr   );
  printf(" - ssi_netif       : %s\n",     conf->ssi_netif      );
  printf(" - ssi_smsc_ip     : %s\n",     conf->ssi_smsc_ip    );
  printf(" - ssi_host_ip     : %s\n",     conf->ssi_host_ip    );
  printf(" - ssi_subnet      : %s\n",     conf->ssi_subnet     );
  printf(" - ssi_gateway     : %s\n",     conf->ssi_gateway    );
  printf(" - ssi_serial      : 0x%.8x\n", conf->ssi_serial     );
  printf(" - ssi_log_level   : %d\n",     conf->ssi_log_level  );
  printf("----------------------------------------------------------\n");
  printf("*           STARTING DATA LISTENING                      *\n");
  printf("----------------------------------------------------------\n");



};

void ssi_dump_diagnostic_header(HD_DIAGNOSTIC *header)
{
  int i;

  char time_string[200];
  char ssi_state[30];
  print_time2string((time_t) header->ulTimestamp, time_string, 200);

  ssi_print_state(header->ucState, ssi_state, sizeof(ssi_state));

  printf("DUMP DIAGNOSTIC MESSAGE\n");
  printf(" - timestamp         : %s\n", time_string);
  printf(" - ucDamage          : 0x%.2x\n", header->ucDamage);
  printf(" - state             : %s\n", ssi_state);
  printf(" - ucvLevel1damage   : ");
  for (i=0;i<8;i++)
  {
    printf("0x%x ", header->ucvLevel1damage[i]);
  }
  printf("\n");
  printf(" - ucvLevel2damage   : ");
  for (i=0;i<8;i++)
  {
    printf("0x%x ", header->ucvLevel2damage[i]);
  }
  printf("\n");
  printf(" - ucvSpare          : ");
  for (i=0;i<8;i++)
  {
    printf("0x%x ", header->ucvSpare[i]);
  }
  printf("\n");
};

void ssi_dump_maintenance_header(HD_MAINTENANCE *header)
{
  char ssi_state[30];
  ssi_print_state(header->ucState, ssi_state, sizeof(ssi_state));

  printf("DUMP MAINTENANCE HEADER\n");
  printf(" - ulCodeStamp: 0x%.8x\n", header->ulCodeStamp);
  printf(" - ucSpare    : 0x%.2x\n", header->ucSpare);
  printf(" - state      : %s\n", ssi_state);
};

void ssi_dump_maintenance_command(uint8_t cmd, uint8_t *data)
{
  int i=0;
  printf("DUMP MAINTENANCE COMMAND\n");
  if(command_code[cmd-128])
  {
    printf(" - command        : %s\n", command_name[cmd-128]);
    printf(" - command length : %d\n", command_len[cmd-128]);
    printf(" - command data   : ");
    for(i=0; i<command_len[cmd-128]; i++)
    {
      printf("0x%.2x ", data[i]);
    }
    printf("\n");
  }
  else
  {
    printf("Invalid command\n");
  }
};

void ssi_dump_cont_data_header(HD_CONT_DATA *header)
{
  uint8_t channels=0, gratings=0;

  char time_string[200];
  print_time2string((time_t) header->ulTimeCodeH, time_string, 200);

  gratings = header->ucFrameFormat >> 4;  // 4 higher bits is the number of gratings
  if (!gratings) // 0 means 16
  {
    gratings = 16;
  }
  channels = header->ucFrameFormat & 15;  // 4 lower bits is the number of channels

  printf("DUMP CONTINUOUS DATA\n");
  printf("HEADER\n");
  printf(" - tx time       : %s\n", time_string);
  printf(" - usFrameSize   : 0x%.4x\n", header->usFrameSize);
  printf(" - ucHdrSizex4   : 0x%.4x\n", header->ucHdrSizex4);
  printf(" - ucFrameFormat\n");
  printf(" -   channels    : 0x%.2x\n", channels);
  printf(" -   gratings    : 0x%.2x\n", gratings);
  printf(" - ulFrameCount  : 0x%.8x\n", header->ulFrameCount);

  print_time2string((time_t) header->ulTimeStampH, time_string, 200);

  printf(" - scan time     : %s\n", time_string);
  printf(" - usTimeInterval: 0x%.4x\n", header->usTimeInterval);
  printf(" - usSpare       : 0x%.4x\n", header->usSpare);
  printf(" - usMinChannel  : 0x%.4x\n", header->usMinChannel);
  printf(" - usMaxChannel  : 0x%.4x\n", header->usMaxChannel);
  printf(" - ulMinWaveFreq : 0x%.8x\n", header->ulMinWaveFreq);
  printf(" - ulSpare       : 0x%.8x\n", header->ulSpare);
};

void ssi_dump_scan_data_header(HD_SCAN_DATA *header)
{
  char time_string[200];
  print_time2string((time_t) header->ulTimeCodeH, time_string, 200);

  printf("DUMP SCAN DATA\n");
  printf("HEADER\n");
  printf(" - tx time       : %s\n", time_string);
  printf(" - usFrameSize   : %d\n", header->usFrameSize);
  printf(" - ucHdrSizex4   : 0x%.2x\n", header->ucHdrSizex4);
  printf(" - ucFrameFormat : 0x%.2x\n", header->ucFrameFormat);
  printf(" - ulFrameCount  : 0x%.8x\n", header->ulFrameCount);

  print_time2string((time_t) header->ulTimeStampH, time_string, 200);
  printf(" - scan time     : %s\n", time_string);

  printf(" - usTimeInterval: 0x%.4x\n", header->usTimeInterval);
  printf(" - usNrSteps     : 0x%.4x\n", header->usNrSteps);
  printf(" - usMinChannel  : 0x%.4x\n", header->usMinChannel);
  printf(" - usMaxChannel  : 0x%.4x\n", header->usMaxChannel);
  printf(" - ulMinWaveFreq : 0x%.8x\n", header->ulMinWaveFreq);
  printf(" - ulMaxWaveFreq : 0x%.8x\n", header->ulMaxWaveFreq);
};

void ssi_dump_data(SSI_DATA *data)
{
  char time_string[200];
  print_time2string((time_t) data->timestamp_s, time_string, 200);

  printf("DUMP DATA\n");
  printf(" - timestamp : %s\n", time_string);
  printf(" - usecs     : %.u\n", data->timestamp_u);
  printf(" - channel   : 0x%.2x\n", data->channel);
  printf(" - grating   : 0x%.2x\n", data->grating);
  printf(" - peak_data : 0x%.4x\n", data->peak_data);
};

int ssi_print_state(uint8_t state, char *buffer, size_t len)
{
  int error_code = STATUS_OK;
  if(!buffer)
  {
    syslog(LOG_DEBUG, "ssi_print_state: Invalid buffer");
    error_code = STATUS_ERROR;
  }
  else
  {
    switch (state) {
      case SSI_STATE_NO_CHANGE:
        strncpy(buffer, "NO CHANGE", len > 10 ? 10 : len);
        break;
      case SSI_STATE_STAND_BY:
        strncpy(buffer, "STAND BY", len > 9 ? 9 : len);
        break;
      case SSI_STATE_OPERATIONAL:
        strncpy(buffer, "OPERATIONAL", len > 12 ? 12 : len);
        break;
      case SSI_STATE_MAINTENANCE:
        strncpy(buffer, "MAINTENANCE", len > 12 ? 12 : len);
        break;
      case SSI_STATE_TEST:
        strncpy(buffer, "TEST", len > 5 ? 5 : len);
        break;
      default:
        strncpy(buffer, "UNKNOWN", len > 8 ? 8 : len);
        break;
    }
  }
  return error_code;
}
