/*******************************************************************************
* Parser interface
*******************************************************************************/
#ifndef PARSER_H
#define PARSER_H

/*******************************************************************************
* included libraries
*******************************************************************************/
// C libraries
#include <stdint.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <syslog.h>
#include <iostream>

// C++ libraries
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <string>
#include <algorithm>
#include <list>

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

extern "C"
{
#include "../CommonLibraries/libutils/utils.h"
#include "../CommonLibraries/libsmartscan/socket.h"
#include "../CommonLibraries/libsmartscan/msg_queue.h"
#include "../CommonLibraries/libsmartscan/smartscan_utils.h"
}
#include "config.hpp"
#include "interrogator.hpp"
#include "client.hpp"

using namespace std;
/*******************************************************************************
* constants
*******************************************************************************/
//#define EMULATOR_WAIT 10000
//#define INTERROGATOR_WAIT 3000
#define PEAK_DATA_NUMBER 20
#define PEAK_QUEUE_SIZE 10000
#define READ_TIMEOUT_SEC 8
#define DIAGNOSTIC_TIMEOUT 4
#define SELECT_TIMEOUT_SEC 20
#define QUEUE_SIZE 1024
#define MAX_INIT_TENTATIVE 1024
#define TotalNumberOfSensorGrating 64
/*******************************************************************************
* struct
*******************************************************************************/
struct vec3
{
    float x, y, z;

    vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {};
};

struct cleanPeakData
{
    int index;
    float wavelength;
    uint64_t timestamp;
};

struct sensorConfig
{
    uint8_t channel, grating;
    bool is_active;
    vec3 position;
    float wavelength_idle;
    float wavelength_var;

    sensorConfig(uint8_t c, uint8_t g, bool ia, vec3 pos, float wi, float wv) :
        channel(c), grating(g), is_active(ia), position(pos),
        wavelength_idle(wi), wavelength_var(wv) {};
};

/*******************************************************************************
* mongodbDAO class definition
*******************************************************************************/
class mongodbDAO
{

public:
    mongocxx::instance instance{};
    mongocxx::uri uri = mongocxx::uri{"mongodb://user:password@ip:port/authDB"}; // TO FILL (only locally)
    mongocxx::client client{uri};
    mongocxx::database db = client["database"]; // TO FILL (only locally)
    mongodbDAO(std::string collname);
    ~mongodbDAO(){};
    void insertUnityData(int index, uint64_t timestamp, float wavelength, uint64_t curr_time);
    void insertMultipleData(std::vector<cleanPeakData> data, uint64_t curr_time);
    void insertConfig(sensorConfig config);
    void insertConfigMultiple(std::vector<sensorConfig> config);
};

/*******************************************************************************
* classes
*******************************************************************************/
class Parser : public Interrogator
{
private:

    /*******************************************************************************
    * Parsing Functions
    *******************************************************************************/
    int ssi_parse_diagnostic_msg(uint8_t* buffer, size_t len, uint8_t *status);
    int ssi_parse_maintenance_msg(uint8_t* buffer, size_t len, SSI_CONFIG *conf);
    int ssi_parse_cont_data_msg(uint8_t* buffer, size_t len);

    /*******************************************************************************
    * Mongo Insertion Wrapper Functions
    *******************************************************************************/
    void insert_config_mongo(mongodbDAO* mongo,std::vector<bool> is_active);
    void insert_unity_data_mongo(mongodbDAO* mongo,int index,float wavelength,uint64_t timestamp);
    void insert_config_mongo_multiple(std::vector<bool> is_active);
    void insert_multiple_data_mongo(std::vector<cleanPeakData> data);
    void set_up_confg(vector<bool> &is_active);
    double ssi_start_time = 0.0;

public:
    thread mongo_th;

    uint8_t               *ssi_state;    // smartscan interrogator state
    SSI_CONFIG            *ssi_conf;
    msg_queue             *msg_q;        // UDP message queue
    struct timespec wait_timeout_cont;
    mutex peak_q_lock;
    sem_t *lock_s_p;         // semaphore on parse thread (listen thread unlocks parsing)
    atomic<bool> *i_running;
    pthread_mutex_t *msg_lock_m_p;    // mutex on message queue

    Parser(sem_t *lock_s,pthread_mutex_t *msg_lock_m,msg_queue *msg_q,SSI_CONFIG *ssi_conf,uint8_t *ssi_state,atomic<bool> *i_running);
    ~Parser();
    void ssi_parse_message();
    void ssi_insert_Peak_Data();
};
#endif
