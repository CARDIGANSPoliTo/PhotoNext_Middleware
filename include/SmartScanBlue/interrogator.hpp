/*******************************************************************************
* interrogator generic interface
*******************************************************************************/

#ifndef INTERROGATOR_H
#define INTERROGATOR_H

/*******************************************************************************
* included libraries
*******************************************************************************/
// C libraries
#include <stdint.h>
#include <inttypes.h>

// C++ libraries
#include <string>
#include <vector>
#include <fstream>

using namespace std;

/*******************************************************************************
* custom data types
*******************************************************************************/
typedef struct peakData
{
  uint32_t pd_timestamp_sc; // seconds
  uint32_t pd_timestamp_fr; // fractionp

  uint16_t pd_data;         // peak data
  double   pd_wavelength;   // wavelength
  uint8_t  pd_channel;      // channel number
  uint8_t  pd_grating;      // grating number
} peakData;

/*******************************************************************************
* classes
*******************************************************************************/
class Interrogator
{
protected:
  string i_vendor           = string("vendor");
  string i_model            = string("model");
  string i_sw_version       = string("0.1");
  string i_serial           = string("123456");

  //bool i_board_init         = false;

  int i_first_laser_channel = -1;
  int i_last_laser_channel  = -1;

  int i_channels            = -1;
  int i_gratings            = -1;

  string i_ip_address       = string("127.0.0.1");
  string i_subnet_mask      = string("255.255.255.0");
  string i_gateway          = string("127.0.0.1");

  bool i_header_set         = false;
  std::ofstream i_log_file;

public:
  Interrogator() {};
  ~Interrogator() {};

  //atomic<bool> i_connected;
  //atomic<bool> i_running;

  //virtual int  i_init() = 0;
  //virtual void i_close() = 0;

  //virtual void i_run() = 0;
  //virtual void i_stop() = 0;
  //virtual void ssi_parse_message()=0;
  //virtual void ssi_listen()=0;
  //virtual void ssi_insert_Peak_Data()=0;
  //virtual void ssi_keepalive()=0;
  //virtual void i_get_spectrum(vector<spectrumData> &v) = 0;
  //virtual void i_get_peak_data(vector<peakData> &v) = 0;

  //virtual void i_set_raw_data_speed(int value) = 0;
  //virtual void i_set_peak_data_speed(int value) = 0;

  //virtual void i_set_scan_speed(int mode, int steps, int cycle) = 0;
};

#endif
