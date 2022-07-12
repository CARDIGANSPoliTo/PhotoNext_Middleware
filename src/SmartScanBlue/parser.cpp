/********************************
* Parser Interrogator Interface *
********************************/

/********************************
*                               *
* ############################  *
*       COMMENTS ARE ADDED      *
*        ABOVE AND INSIDE       *
*         EVERY FUNCTION        *
* ############################  *
*                               *
********************************/


/*******************************************************************************
* included libraries
*******************************************************************************/
#include "../../include/SmartScanBlue/parser.hpp"

/*******************************************************************************
* Global Variables.
*******************************************************************************/
using namespace std;
using namespace std::chrono;
std::vector<cleanPeakData> cleaned_peak_data;
std::vector<cleanPeakData> cleaned_peak_data_local;
std::vector<bool> is_active(64, false);
std::condition_variable condVar;

mutex clean_peak_data_lock;
mongodbDAO mongo(collectionName);
std::vector<sensorConfig> sensors_config;
std::mutex mutex_;
std::unique_lock<std::mutex> lck(mutex_);
std::string logFilePath;
long long int TotalPeakDataInDb=0;
std::vector<cleanPeakData> filteredPeakVector;

/*******************************************************************************
*                                                                              *
*                           MONGO WRAPPER FUNCTIONS                            *
*                                                                              *
*******************************************************************************/

/*******************************************
* set up data structure for config packet. *
*******************************************/
void Parser::set_up_confg(vector<bool> &is_active)
{
    /*******************************************
    * channel start from 0 --> < 4 if we need  *
    * to have a range 0->3 while 1 --> <=4 if  *
    * we need to have a range 1->4 same as for *
    * grating from 0->15 or from 1->16.        *
    *******************************************/
    //for(uint8_t channel = 1; channel <= 4; channel++)
    for(uint8_t channel = 0; channel < 4; channel++)
    {
        //for(uint8_t grating = 1; grating <= 16; grating++)
        for(uint8_t grating = 0; grating < 16; grating++)
        {
            {
                /*******************************************
                 * Inserting In The Sensor_Config.
                 * Only The Active Channel/Grating.
                 *******************************************/
                if(is_active[channel*16 + grating]==true)
                    sensors_config.push_back(sensorConfig(channel, grating, is_active[channel*16 + grating], vec3(0,0,0), 0.0, 0.0));

            }
        }
    }
}

/******************************************************************************
*                       SINGLE DATA/CONFIG INSERTION                          *
*                       INTO MONGODB SERVER FUCNTIONS.                        *
*                         (INSERTING ONE BY ONE ).                            *
******************************************************************************/

/********************************************
* Insert Config Packet into Mongodb server  *
*        (INSERTING ONE BY ONE ).           *
********************************************/
/**
void insert_config_mongo(std::vector<bool> is_active)
{
    set_up_confg(is_active);

    for(sensorConfig sc : sensors_config)
    {
        mongo.insertConfig(sc);
    }
}
*/
/*****************************************
* Insert Unity Data into Mongodb server  *
*        (INSERTING ONE BY ONE ).        *
*****************************************/
/**
void insert_unity_data_mongo(int index,float wavelength,uint64_t timestamp)
{
    struct timespec current_time;
    uint64_t curr_time;
    clock_getgettime(CLOCK_REALTIME, &current_time);
    curr_time = (uint64_t)current_time.tv_sec * 1000 + (uint64_t)current_time.tv_nsec/1000000;
    mongo.insertUnityData(index, timestamp, wavelength,curr_time);
}

*/
/******************************************************************************
*                    MULTIPLE DATA/CONFIG INSERTION                           *
*                    INTO MONGODB SERVER FUCNTIONS.                           *
*                    (INSERTING MULTIPLE (VECTOR)).                           *
******************************************************************************/

/********************************************
* Insert Config Packet into Mongodb server  *
* (INSERTING MULTIPLE (VECTOR)).            *
********************************************/
void Parser::insert_config_mongo_multiple(std::vector<bool> is_active)
{

    set_up_confg(is_active);

    /**************************************************************
    * If sensors_config is empty --> Exception will be thrown.    *
    * So we make a check before inserting and before clearing.    *
    **************************************************************/
    if(!sensors_config.empty())
    {

        TotalPeakDataInDb = TotalPeakDataInDb + sensors_config.size();
        mongo.insertConfigMultiple(sensors_config);
    }
}
/*****************************************
* Insert Unity Data into Mongodb server  *
* (INSERTING MULTIPLE (VECTOR)).         *
*****************************************/
void Parser::insert_multiple_data_mongo(std::vector<cleanPeakData> data)
{
    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);

    uint64_t curr_time = (uint64_t)current_time.tv_sec * 1000 + (uint64_t)current_time.tv_nsec/1000000;
    mongo.insertMultipleData(data,curr_time);

}

/***************************************
* DATA CONSUMER FUNCTION               *
*--------------------------------------*
* CONDITION VARIABLE IS USED           *
***************************************/
void Parser::ssi_insert_Peak_Data()
{
    syslog(LOG_INFO, "Started mongo thread");
    bool is_setup = false;

    /*****************************************
    * Used For One By One MongoDB Insertion. *
    *****************************************/
    std::cout << "Collection Created On MongoDB Server:" <<endl;
    std::cout << collectionName << std::endl;
    cout<<"----------------------------------------------------------" <<std::endl;

    while (true)
    {
        /*******************************************
        * condVar will wait until PEAK_DATA_NUMBER *
        * is ready to be inserted                  *
        *******************************************/
        condVar.wait(lck);
        //lock used for Shared Variable Synchronization Usage.
        clean_peak_data_lock.lock();

        /**************************************
        *                                     *
        *  Inserting Data in Mongo Multiple.  *
        *                                     *
        **************************************/
        if (!is_setup)
        {
            insert_config_mongo_multiple(is_active);
            is_setup = true;
        }

        /**************************************************************
        * If cleaned_peak_data is empty --> Exception will be thrown. *
        * So we make a check before inserting and before clearing.    *
        **************************************************************/
        if(!cleaned_peak_data.empty())
        {
            insert_multiple_data_mongo(cleaned_peak_data);
            cleaned_peak_data.clear();
        }
         
        clean_peak_data_lock.unlock();

    
        /***************************************
        * Inserting Data in Mongo one by one.  *
        ***************************************/
        /**
        while (!cleaned_peak_data.empty())
        {

            data = cleaned_peak_data.back();
            cleaned_peak_data.pop_back();

            if (!is_setup)
            {
                insert_config_mongo(mongo,is_active);
                is_setup = true;
            }

            //we insert data after reading that at each simulation
            insert_unity_data_mongo(mongo,data.index,data.wavelength,data.timestamp);

            //we can add also conditional variable here or using sleep.
        }
        **/

        /***********************************************
        *  Check if the user Exit Client from terminal *
        *  Sleep is used to wait for the termination   *
        *  Of data parsing and saving all remaining    * 
        ***********************************************/
        if(!i_running->load())
        {
            usleep(100000);
            if(cleaned_peak_data.empty()  && filteredPeakVector.empty())
            {
                break;
            }
        }
    }
    return ;
}

/*******************************************************************************
*                                                                              *
*                            PARSER THREAD FUCNTION                            *
*                                                                              *
*******************************************************************************/
void Parser::ssi_parse_message()
{
    syslog(LOG_INFO, "Started parse thread");
    uint8_t buffer[MSG_LIMIT_MTU];
    uint8_t msg_type;
    size_t msg_size;
    clock_t t = 0;
    double parse_time = 0.0;
    i_log_file.open(logFilePath);
    syslog(LOG_INFO, "Open log file");
    mongo_th = thread(&Parser::ssi_insert_Peak_Data,this);


    while(true)
    {

        sem_wait(lock_s_p); // wait for data in queue

        if(i_running->load())
        {

            msg_q_get_ts(msg_q, buffer, &msg_size, &msg_type,msg_lock_m_p);
            t = clock();
            switch (msg_type)
            {
            case MSG_TYPE_DIAG:
                ssi_parse_diagnostic_msg(buffer, msg_size, ssi_state);
                break;
            case MSG_TYPE_MAIN:
                ssi_parse_maintenance_msg(buffer, msg_size, ssi_conf);
                break;
            case MSG_TYPE_CONT:
                ssi_parse_cont_data_msg(buffer, msg_size);
                break;
            default:
                syslog(LOG_ERR, "Invalid message type");
                break;
            }

            t = clock() - t;
            parse_time = (double)t / CLOCKS_PER_SEC * 1000;
            syslog(LOG_DEBUG, "Parse function time : %.6f ms", parse_time);
        }

        if(!i_running->load())
        {
            break;
        }

        usleep(100000);
    }


    /**************************************************************
    * Sending Data if it is less than given PEAK_DATA_NUMBER      *
    * Data is sent to Consumer then a break quits the thread      *
    **************************************************************/
    if(filteredPeakVector.size()>0)
    {
        std::cout << "Sending The Last Remaining " << filteredPeakVector.size() << " Peak Data " << endl;
        std::cout <<"----------------------------------------------------------" <<std::endl;
        clean_peak_data_lock.lock();
        TotalPeakDataInDb = TotalPeakDataInDb + filteredPeakVector.size();
        cleaned_peak_data=filteredPeakVector;
        clean_peak_data_lock.unlock();
        filteredPeakVector.clear();

    }

    condVar.notify_one();

    return;
}

/*******************************************************************************
*                           PARSING MESSAGES                                   *
*                       (DEPEND ON MESSAGE TYPE)                               *
*******************************************************************************/
int Parser::ssi_parse_diagnostic_msg(uint8_t* buffer, size_t len, uint8_t *status)
{
    int error_code = STATUS_OK;
    int i;

    size_t current_index = 0;

    HD_DIAGNOSTIC header;

    if(!buffer) // check buffer pointer
    {
        syslog(LOG_ERR, "Buffer pointer is NULL");
        error_code = STATUS_ERROR;
    }

    if(!error_code && len != MSG_DIAGNOSTIC_SIZE) // check buffer length
    {
        syslog(LOG_ERR, "Buffer length is invalid");
        error_code = STATUS_ERROR;
    }

    if(!error_code) // start message parse
    {
        current_index += read_32((void *)(buffer + current_index), &(header.ulTimestamp), BE);
        current_index += read_8((void *)(buffer + current_index), &(header.ucDamage));
        current_index += read_8((void *)(buffer + current_index), &(header.ucState));
        for(i=0; i<8; i++)
        {
            current_index += read_8((void *)(buffer + current_index), &(header.ucvLevel1damage[i]));
        }
        for(i=0; i<8; i++)
        {
            current_index += read_8((void *)(buffer + current_index), &(header.ucvLevel2damage[i]));
        }
        for(i=0; i<8; i++)
        {
            current_index += read_8((void *)(buffer + current_index), &(header.ucvSpare[i]));
        }
    }

    *status = header.ucState;

    return error_code;
}

int Parser::ssi_parse_maintenance_msg(uint8_t* buffer, size_t len, SSI_CONFIG *conf)
{
    int error_code = STATUS_OK;

    size_t current_index = 0;
    int i;
    uint8_t ip[4];
    uint16_t u16_value;

    char ip_str[16];

    uint8_t cmd, cmd_len, *cmd_data;

    HD_MAINTENANCE header;

    if(!buffer) // check buffer pointer
    {
        syslog(LOG_ERR, "Buffer pointer is NULL");
        error_code = STATUS_ERROR;
    }

    if(!error_code && (len < HD_MAINTENANCE_SIZE || len > MSG_MAINTENANCE_SIZE)) // check buffer length
    {
        syslog(LOG_ERR, "Buffer length is invalid");
        error_code = STATUS_ERROR;
    }

    if(!error_code && (((len - HD_MAINTENANCE_SIZE) % 4) != 0))  // maintenance payload is 32 bits aligned
    {
        syslog(LOG_ERR, "Payload is not correctly aligned");
        error_code = STATUS_ERROR;
    }

    if(!error_code) // start message parse
    {
        current_index += read_32((void *)(buffer + current_index), &(header.ulCodeStamp), BE);
        current_index += read_8((void *)(buffer + current_index), &(header.ucSpare));
        current_index += read_8((void *)(buffer + current_index), &(header.ucState));

        while(current_index < len && !error_code)
        {
            // parse command
            current_index += read_8((void *) (buffer + current_index), &cmd);

            if(cmd >= 128)  // check if is a ret code
            {
                if(command_code[cmd-128])
                {
                    current_index += read_8((void *) (buffer + current_index), &cmd_len);

                    if(cmd_len == command_len[cmd-128])
                    {
                        cmd_data = (uint8_t *) malloc(cmd_len * sizeof(uint8_t));

                        if(cmd_data)
                        {
                            for(i=0; i<cmd_len; i++)
                            {
                                current_index += read_8((void *) (buffer + current_index), cmd_data + i);
                            }

                            switch(cmd)
                            {
                            case CMD_RET_STATE_CMD:
                                read_8(cmd_data, ssi_state);
                                break;
                            case CMD_RET_DEMO_MODE_CMD:
                                read_8(cmd_data, &(conf->ssi_demo));
                                break;
                            case CMD_RET_SCAN_TX_CMD:
                                read_16(cmd_data, &(conf->ssi_raw_speed), BE);
                                break;
                            case CMD_RET_DATA_CODE_CMD:
                                read_16(cmd_data, &(conf->ssi_cont_speed), BE);
                                break;
                            case CMD_RET_CH_FORMAT_CMD:
                                read_16(cmd_data, &(u16_value), BE);
                                conf->ssi_gratings = ((u16_value & GRATING_MASK) >> 4);
                                conf->ssi_channels = u16_value & CHANNEL_MASK;
                                break;
                            case CMD_RET_SCAN_FREQ_CMD:
                                read_16(cmd_data, &(conf->ssi_first_fr), BE);
                                break;
                            case CMD_RET_IP_ADDR_CMD:
                                for(i=0; i<cmd_len; i++)
                                {
                                    ip[i] = (uint8_t) *(cmd_data + i);
                                }
                                snprintf(ip_str, 16, "%hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);
                                strncpy(conf->ssi_smsc_ip, ip_str, 15);
                                break;
                            case CMD_RET_SUBNET_CMD:
                                for(i=0; i<cmd_len; i++)
                                {
                                    ip[i] = (uint8_t) *(cmd_data + i);
                                }
                                snprintf(ip_str, 16, "%hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);
                                strncpy(conf->ssi_subnet, ip_str, 15);
                                break;
                            case CMD_RET_GATEWAY_CMD:
                                for(i=0; i<cmd_len; i++)
                                {
                                    ip[i] = (uint8_t) *(cmd_data + i);
                                }
                                snprintf(ip_str, 16, "%hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);
                                strncpy(conf->ssi_gateway, ip_str, 15);
                                break;
                            case CMD_RET_SERIAL_CMD:
                                read_32(cmd_data, &(conf->ssi_serial), BE);
                                break;
                            default:
                                syslog(LOG_ERR, "Command not recognised");
                                break;
                            }
                            free(cmd_data);
                        }
                        else
                        {
                            syslog(LOG_CRIT, "Unable to allocate memory");
                            error_code = STATUS_ERROR;
                        }
                    }
                    else
                    {
                        syslog(LOG_ERR, "Invalid command length");
                    }
                }
            }
        }
    }
    return error_code;
}

int Parser::ssi_parse_cont_data_msg(uint8_t* buffer, size_t len)
{

    int error_code = STATUS_OK;
    size_t current_index = 0;
    int i,j,f;
    cleanPeakData CleanedData;
    //std::vector<cleanPeakData> cleaned_peak_data;
    bool found=false;
    uint32_t us_base = 0, us_increment = 0;
    uint32_t timestamp_sc = 0, timestamp_fr = 0;

    uint8_t channels=0, gratings=0;
    uint16_t frame_len = 0, frames=0, exp_payload_len = 0;

    HD_CONT_DATA header;
    uint16_t grating_data;

    double timestamp_us;

    peakData data;

    char log_line[600];
    std::string log_buffer;

    char header_line[100]="";
    char grating_name[16];
    std::string header_string(header_line);


    if(!buffer) // check buffer pointer
    {
        syslog(LOG_ERR, "Buffer pointer is NULL");
        error_code = STATUS_ERROR;
    }

    if(!error_code && (len < HD_CONT_DATA_SIZE || len > MSG_CONT_DATA_SIZE)) // check buffer length
    {
        syslog(LOG_ERR, "Buffer length is invalid");
        error_code = STATUS_ERROR;
    }

    if(!error_code) // start message parse
    {
        current_index += read_16((void *)(buffer + current_index), &(header.usFrameSize), BE);
        current_index += read_8((void *)(buffer + current_index), &(header.ucHdrSizex4));
        current_index += read_8((void *)(buffer + current_index), &(header.ucFrameFormat));
        current_index += read_32((void *)(buffer + current_index), &(header.ulFrameCount), BE);
        current_index += read_32((void *)(buffer + current_index), &(header.ulTimeStampH), BE);
        current_index += read_32((void *)(buffer + current_index), &(header.ulTimeStampL), BE);
        current_index += read_32((void *)(buffer + current_index), &(header.ulTimeCodeH), BE);
        current_index += read_16((void *)(buffer + current_index), &(header.usTimeInterval), BE);
        current_index += read_16((void *)(buffer + current_index), &(header.usSpare), BE);
        current_index += read_16((void *)(buffer + current_index), &(header.usMinChannel), BE);
        current_index += read_16((void *)(buffer + current_index), &(header.usMaxChannel), BE);
        current_index += read_32((void *)(buffer + current_index), &(header.ulMinWaveFreq), BE);
        current_index += read_32((void *)(buffer + current_index), &(header.ulSpare), BE);
    }

    gratings = header.ucFrameFormat >> 4;  // 4 higher bits is the number of gratings
    if (gratings == 0) // 0 means 16
    {
        gratings = 16;
    }
    channels = header.ucFrameFormat & 0x0F;  // 4 lower bits is the number of channels

    frame_len = gratings*channels*sizeof(uint16_t);
    frames = (MSG_CONT_DATA_SIZE - HD_CONT_DATA_SIZE)/(frame_len);
    exp_payload_len = frame_len * frames;

    us_base = (uint32_t) ((double) header.ulTimeStampL / 0xffffffff * 1000000); // timestamp (usec fraction) of first data
    if(!i_header_set)
    {
        timestamp_us = (double)header.ulTimeStampH * 1000000 + us_base;

        timestamp_sc = (uint32_t) (timestamp_us/1000000);
        timestamp_fr = (uint32_t) ((us_base) % 1000000);

        snprintf(header_line, 100, "Start Time (UTC) = %" PRIu32 ",%" PRIu32 "\r\nTime interval = %" PRIu32 " us\r\n\r\nTime", timestamp_sc, timestamp_fr, header.usTimeInterval);
        header_string.append(header_line);
        header_string.append("\r\n");
        ssi_start_time = timestamp_us;
        i_header_set = true;
        i_log_file << header_string;
    }
    if((len - HD_CONT_DATA_SIZE) == exp_payload_len) // check payload len
    {
        for(f=0; f<frames; f++)
        {        

            us_increment = f*header.usTimeInterval; // usec increment for each sample
            timestamp_us = (double)header.ulTimeStampH * 1000000 + us_base + us_increment;

            timestamp_sc = (uint32_t) (timestamp_us/1000000);
            timestamp_fr = (uint32_t) ((us_base + us_increment) % 1000000);

            //to save the timestamp same as MongoDB server:
            snprintf(log_line, 600, "%.0f", timestamp_us);

            //to save the increments us starting from zero.
            //snprintf(log_line, 600, "%.6f", (timestamp_us - ssi_start_time)/1000000);
            log_buffer.append(log_line);


            for(i=0; i<channels; i++)
            {
                for(j=0; j<gratings; j++)
                {

                    current_index += read_16((void *)(buffer + current_index), &grating_data, BE);

                    data.pd_timestamp_sc = timestamp_sc;
                    data.pd_timestamp_fr = timestamp_fr;

                    data.pd_channel     = i;
                    data.pd_grating     = j;
                    data.pd_data        = grating_data;
                    data.pd_wavelength  = ssi_laser_channel_2_peak(data.pd_data);


                    /**************************************************************
                    * Data filtering start here according to wavelength greater   *
                    * Than 0 and also according to the indexes so we will take    *
                    * from every frame a value coming from the channel-grating    *
                    * so we will not have a floud of data arriving to MongoDB     *
                    * This is used to make a more efficient real-time analyzing   *
                    * for the new data arriving MongoDB server using the viewer   *
                    **************************************************************/
                    if(data.pd_wavelength>0)
                    {

                        //CONFIG MESSAGE
                        is_active[data.pd_channel * 16 + data.pd_grating] = (data.pd_wavelength == 0 ? false : true);
                        snprintf(log_line, 600, "\t%.13f", (float)data.pd_wavelength);
                        log_buffer.append(log_line);

                        CleanedData.timestamp=((uint64_t)data.pd_timestamp_sc) * 1000000 + ((uint64_t)data.pd_timestamp_fr);
                        CleanedData.wavelength=(float)data.pd_wavelength;
                        CleanedData.index=data.pd_channel * 16 + data.pd_grating;

                        if(cleaned_peak_data_local.empty())
                        {

                            cleaned_peak_data_local.push_back(CleanedData);
                        }
                        else
                        {

                            for(unsigned int k = 0; k < cleaned_peak_data_local.size(); k++)
                            {

                                if(cleaned_peak_data_local[k].index==CleanedData.index)
                                {

                                    cleaned_peak_data_local[k]=CleanedData;
                                    found=true;

                                }

                            }
                            if(found==false)
                            {

                                cleaned_peak_data_local.push_back(CleanedData);
                            }
                        }
                        found=false;

                    }

                    /*******************************************************************************
                    * Queue Structure:
                    *******************************************************************************/
                    /*
                    if (peak_data_q.size() == PEAK_QUEUE_SIZE)
                      {
                        syslog(LOG_CRIT, "Reached queue size limit");
                      }
                    else
                      {
                    peak_q_lock.lock();
                    peak_data_q.push(data);
                    peak_q_lock.unlock();
                    }
                    */

                    /*******************************************************************************
                    * Circular Queue Pattern:
                    *******************************************************************************/
                    /*
                    peak_q_lock.lock();
                    peak_data_q.enQueue(data);
                    peak_q_lock.unlock();
                    */

                    /*******************************************************************************
                    * Observer-Subject Pattern:
                    *******************************************************************************/
                    //need to be updated to send a collection of data and not only one data structure: Queue<peak> and not peak.
                    //subject->GetPeakData(data);

                    /*******************************************************************************
                    * Three i_running->load() check since we have 3 for loops.                     *
                    * so when we Exit the client we don't need to read more data                   *
                    * we just break the loops --> and on the listener we join this thread.         *
                    * REMARK : because of this 3 breaks miss --> Core Dump.                        *
                    *******************************************************************************/

                    if(!i_running->load())
                    {
                        break;
                    }

                }

                if(!i_running->load())
                {
                    break;
                }

            }

            if(!i_running->load())
            {
                break;
            }

            log_buffer.append("\r\n");
        }
        
        if(!cleaned_peak_data_local.empty()){

        for(unsigned int l = 0; l < cleaned_peak_data_local.size(); l++)
        {

            filteredPeakVector.push_back(cleaned_peak_data_local[l]);
        }
        cleaned_peak_data_local.clear();
        }

    
        if(filteredPeakVector.size()>=PEAK_DATA_NUMBER )
        {

            std::cout << "Sending " << PEAK_DATA_NUMBER << " Peak Data ..." <<std::endl;
            clean_peak_data_lock.lock();
            //here we must check
            TotalPeakDataInDb = TotalPeakDataInDb + PEAK_DATA_NUMBER;
            cleaned_peak_data=filteredPeakVector;
            clean_peak_data_lock.unlock();
            filteredPeakVector.clear();

            /************************************
            * condVar will notify and wake      *
            * up the mongo_th thread to start   *
            * filling into the mongoDB server   *
            ************************************/
            condVar.notify_one();
           
        }

        std::replace(log_buffer.begin(), log_buffer.end(), '.', ',');
        i_log_file << log_buffer; //, log_buffer.length());
        log_buffer.clear();
        
    }
    else
    {
        syslog(LOG_ERR, "Invalid payload length");
        error_code = STATUS_ERROR;
    }

    return error_code;
}

/***************************************************
* PARSER CONSTRUCTOR:                              *
*--------------------------------------------------*
* Arguments are passed from the listener object    *
* and they are initiliazed here.                   *
* Passed by reference is used inside the listener  *
* And here we received it by a pointer.            *
* So we can have a shared value check or update    *
* between the listener and the parser objects.     *
***************************************************/
Parser::Parser(sem_t *lock_s,pthread_mutex_t *msg_lock_m,msg_queue *msg_q,SSI_CONFIG *ssi_conf,uint8_t *ssi_state,atomic<bool> *i_running)
{
    this->lock_s_p = lock_s;
    this->msg_lock_m_p=msg_lock_m;
    this->msg_q=msg_q;
    this->ssi_conf=ssi_conf;
    this->ssi_state=ssi_state;
    this->i_running=i_running;
};

/***************************************************
* PARSER DESTRUCTOR:                               *
*--------------------------------------------------*
* The destructor will wait for the mongo_th thread *
* to finish its work using mongo_th.join()         *
***************************************************/
Parser::~Parser()
{

    //condVar.notify_one();
    mongo_th.join();
    i_log_file.close();
    std::cout << "Total Active Gratings: " << sensors_config.size() <<std::endl;
    std::cout << "Total Peak Data: " << TotalPeakDataInDb-sensors_config.size() <<std::endl;
    std::cout << "Total Config/Peak Data Saved On MongoDB Server: " << TotalPeakDataInDb <<endl;
    std::cout <<"----------------------------------------------------------" <<std::endl;
    cout << "MongoDB Thread  Joined  -->     Successfully             *" <<endl;
    syslog(LOG_DEBUG, "MongoDB Thread Joined Successfully");

};
