/***********************************************************************************
* Listener Interrogator Interface                                                  *
* Listener functions:                                                              *
*                                                                                  *
* - i_init :                                                                       *
* Take care of the semphores,mutexes and msg_queue initialization.                 *
* Reading the board Config.                                                        *
* Socket Initialization.                                                           *
* log File Creation.                                                               *
*                                                                                  *
* - i_listen :                                                                     *
* Will create the parse thread that will start parsing the data gather             *
* inside the listener function.                                                    *
* Every socket is responsible for a specific type of data to be sent               *                                                                                     *
* File Descriptor are set for both write and read.                                 *
* Sem_post(&lock_s) is used to release the parsing thread that is waiting for data *
* This data is put inside the msg_q.                                               *
*                                                                                  *
* - ssi_socket_init/ssi_socket_delete:                                             *
* Socket initialization at beginning.                                              *
* Socket deletion at the end.                                                      *
*                                                                                  *
* - ssi_send_maintenance_msg:                                                      *
* Is called when the connection is established with the interrogator.              *
* Contains the initial configuration.                                              *
*                                                                                  *
* - ssi_send_timestamp:                                                            *
* Is called to send current timestamp for synchronization.                         *
*                                                                                  *
* - ~Listener(): (DESTRUCTOR)                                                      *
* A check is done since in the client we create a global "listener ssi " but if    *
* this object is not used while we check the name of the interrogator then joining *
* deinitialization,destroying and freeing will cause a dump after the automatic    *
* call of the destructor when we close the client.                                 *
* Check is done on listenerInitializedWell==true (prevent Dump Core)               *
* then we are sure that the listener is initialized well, and thread,variables,    *
* mutex,semaphores,locks... are created and need to be joined,destroyed...         *
* It will call ssi_socket_delete to delete sockets.                                *
* It will make a sem_post to release the parse thread to exit.                     *
* It will wait until the parse_th thread finish its work --> parse_th.join()       *
* It will start deinitialization,destroying,freeing for semaphors,mutex...         *
*                                                                                  *
* - Listener(): (CONSTRUCTOR)                                                      *
* i_running and i_connected are initialized to false so then after connection      *
* they will be updated to true in the listen method.                               *
***********************************************************************************/

/*******************************************************************************
* included libraries
*******************************************************************************/
#include "../../include/SmartScanBlue/listener.hpp"

struct tm * timeinfo;
extern string logFilePath;
bool listenerInitializedWell=false;
/*******************************************************************************
* listener methods
*******************************************************************************/
int Listener::i_init()
{
    int error_code = STATUS_OK;
    //struct tm * timeinfo;

    time_t current_time;
    char buffer[80];

    if ((sem_init(&lock_s, 0, 0)) == -1)
    {
        syslog(LOG_CRIT, "Unable to create semaphore");
        error_code = STATUS_ERROR;
    }

    // init mutex for queue critical section
    msg_lock_m = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    if (!msg_lock_m)
    {
        syslog(LOG_CRIT, "Unable to allocate mutex");
        error_code = STATUS_ERROR;
    }
    else
    {
        pthread_mutex_init(msg_lock_m, NULL);
    }

    // init queue
    if((msg_q_init(&msg_q, MSG_QUEUE_SIZE, MSG_LIMIT_MTU)) != STATUS_OK)
    {
        syslog(LOG_CRIT, "Unable to init queue");
        error_code = STATUS_ERROR;
    }

    // set interrogator state
    ssi_state = SSI_STATE_STAND_BY;

    if(ssi_read_config(&ssi_conf) != STATUS_OK)
    {
        syslog(LOG_CRIT, "Unable to init board");
        error_code = STATUS_ERROR;
    }

    if(ssi_socket_init() != STATUS_OK)
    {
        syslog(LOG_CRIT, "Unable to init sockets");
        error_code = STATUS_ERROR;
    }
    else
    {
        if(ssi_send_timestamp() != STATUS_OK)
        {
            syslog(LOG_CRIT, "Unable to send timestamp message");
            error_code = STATUS_ERROR;
        }
    }


    if(error_code == STATUS_OK)
    {
        time (&current_time);
        timeinfo = localtime(&current_time);

        std::string logfile_path("/home/");

        if((getlogin_r(buffer, 79)) == 0)
        {
            logfile_path.append(buffer);
            logfile_path.append("/smartscan_log/");
        }
        else
        {
            logfile_path.append("pi/smartscan_log/");
        }

        const int dir_err = mkdir(logfile_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (dir_err == -1 )
        {
            if (errno == EEXIST)
            {
                syslog(LOG_INFO, "Log folder already exists");
            }
            else
            {
                syslog(LOG_ERR, "Error creating log folder");
                exit(1);
            }
        }
        else
        {
            syslog(LOG_INFO, "Created log folder");
        }

        strftime(buffer,sizeof(buffer),"%Y%m%d_%H%M%S", timeinfo);
        std::string log_filename(buffer);
        log_filename += ".log";

        i_log_file.open(logfile_path + log_filename);

        i_header_set = false;

        syslog(LOG_INFO, "Open log file");
        logFilePath = logfile_path + log_filename;
    }

    //i_log_file.close();


    syslog(LOG_INFO, "Started smartscan interface");
    listenerInitializedWell=true;
    return error_code;

}

int Listener::ssi_socket_init()
{
    int exit_code = STATUS_OK;

    // rx socket
    if((d_socket_rx = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        syslog(LOG_CRIT, "Unable to open diagnostic socket");
        exit_code = STATUS_ERROR;
    }

    if((m_socket_rx = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == - 1)
    {
        syslog(LOG_CRIT, "Unable to open maintenance socket");
        exit_code = STATUS_ERROR;
    }

    if((c_socket_rx = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        syslog(LOG_CRIT, "Unable to open continuous data socket");
        exit_code = STATUS_ERROR;
    }

    if((s_socket_rx = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        syslog(LOG_CRIT, "Unable to open scan data socket");
        exit_code = STATUS_ERROR;
    }

    // tx socket
    if ((cfg_socket_tx = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        syslog(LOG_CRIT, "Unable to open send socket");
        exit_code = STATUS_ERROR;
    }

    // rx socket sources
    d_src_rx.sin_family = AF_INET;
    m_src_rx.sin_family = AF_INET;
    c_src_rx.sin_family = AF_INET;
    s_src_rx.sin_family = AF_INET;

    // tx socket sources
    cfg_src_tx.sin_family = AF_INET;
    d_dst_tx.sin_family = AF_INET;
    m_dst_tx.sin_family = AF_INET;

    // socket ports
    d_src_rx.sin_port = htons(PORT_RX_DIAG);
    m_src_rx.sin_port = htons(PORT_RX_MAIN);
    c_src_rx.sin_port = htons(PORT_RX_CONT);
    s_src_rx.sin_port = htons(PORT_RX_SCAN);

    cfg_src_tx.sin_port = htons(PORT_TX_HOST);


#if EMU_LOCAL == 1
  d_dst_tx.sin_port = htons(30011);
  m_dst_tx.sin_port = htons(30012);
#endif

#if EMU_LOCAL == 0
  d_dst_tx.sin_port = htons(PORT_RX_DIAG);
  m_dst_tx.sin_port = htons(PORT_RX_MAIN);
#endif

    if(inet_aton(LISTEN_IP, &(d_src_rx.sin_addr)) == 0)
    {
        syslog(LOG_CRIT, "Invalid source IP address");
        exit_code = STATUS_ERROR;
    }
    if(inet_aton(LISTEN_IP, &(m_src_rx.sin_addr)) == 0)
    {
        syslog(LOG_CRIT, "Invalid source IP address");
        exit_code = STATUS_ERROR;
    }
    if(inet_aton(LISTEN_IP, &(c_src_rx.sin_addr)) == 0)
    {
        syslog(LOG_CRIT, "Invalid source IP address");
        exit_code = STATUS_ERROR;
    }
    if(inet_aton(LISTEN_IP, &(s_src_rx.sin_addr)) == 0)
    {
        syslog(LOG_CRIT, "Invalid source IP address");
        exit_code = STATUS_ERROR;
    }

    if (inet_aton(ssi_conf.ssi_host_ip, &(cfg_src_tx.sin_addr)) == 0)
    {
        syslog(LOG_CRIT, "Invalid source IP address");
        exit_code = STATUS_ERROR;
    }

    if (inet_aton(ssi_conf.ssi_smsc_ip, &(d_dst_tx.sin_addr)) == 0)
    {
        syslog(LOG_CRIT, "Invalid destination IP address");
        exit_code = STATUS_ERROR;
    }

    if (inet_aton(ssi_conf.ssi_smsc_ip, &(m_dst_tx.sin_addr)) == 0)
    {
        syslog(LOG_CRIT, "Invalid destination IP address");
        exit_code = STATUS_ERROR;
    }

    // rx socket bind
    if(bind(d_socket_rx, (struct sockaddr*)&(d_src_rx), sizeof(d_src_rx)) == -1)
    {
        syslog(LOG_CRIT, "Socket bind failed");
        exit_code = STATUS_ERROR;
    }
    if(bind(m_socket_rx, (struct sockaddr*)&(m_src_rx), sizeof(m_src_rx)) == -1)
    {
        syslog(LOG_CRIT, "Socket bind failed");
        exit_code = STATUS_ERROR;
    }
    if(bind(c_socket_rx, (struct sockaddr*)&(c_src_rx), sizeof(c_src_rx)) == -1)
    {
        syslog(LOG_CRIT, "Socket bind failed");
        exit_code = STATUS_ERROR;
    }
    if(bind(s_socket_rx, (struct sockaddr*)&(s_src_rx), sizeof(s_src_rx)) == -1)
    {
        syslog(LOG_CRIT, "Socket bind failed");
        exit_code = STATUS_ERROR;
    }

    // tx socket bind
    if((bind(cfg_socket_tx, (struct sockaddr*)&(cfg_src_tx), sizeof(cfg_src_tx))) == -1)
    {
        syslog(LOG_CRIT, "Socket bind failed");
        exit_code = STATUS_ERROR;
    }

    syslog(LOG_INFO, "Open rx socket on %s:%d", inet_ntoa(d_src_rx.sin_addr), ntohs(d_src_rx.sin_port));
    syslog(LOG_INFO, "Open rx socket on %s:%d", inet_ntoa(m_src_rx.sin_addr), ntohs(m_src_rx.sin_port));
    syslog(LOG_INFO, "Open rx socket on %s:%d", inet_ntoa(c_src_rx.sin_addr), ntohs(c_src_rx.sin_port));
    syslog(LOG_INFO, "Open rx socket on %s:%d", inet_ntoa(s_src_rx.sin_addr), ntohs(s_src_rx.sin_port));
    syslog(LOG_INFO, "Open tx socket on %s:%d", inet_ntoa(cfg_src_tx.sin_addr), ntohs(cfg_src_tx.sin_port));

    return exit_code;
}

void Listener::ssi_socket_delete()
{
    close(d_socket_rx);
    syslog(LOG_INFO, "Closing diagnostic socket");
    close(m_socket_rx);
    syslog(LOG_INFO, "Closing maintenance socket");
    close(c_socket_rx);
    syslog(LOG_INFO, "Closing continuous data socket");
    close(s_socket_rx);
    syslog(LOG_INFO, "Closing scan data socket");
    close(cfg_socket_tx);
    syslog(LOG_INFO, "Closing configuration socket");
    return;
}

void Listener::ssi_listen()
{


    Parser parser(&lock_s,msg_lock_m,&msg_q,&ssi_conf,&ssi_state,&i_running);
    parse_th = thread(&Parser::ssi_parse_message,&parser);

    struct sockaddr_in src;
    socklen_t l = sizeof(src);

    uint8_t buffer[MSG_LIMIT_MTU];
    int rec_len;

    size_t msg_q_size;

    struct timeval select_to;
    select_to.tv_sec = 5;
    select_to.tv_usec = 0;

    double c_data_interval = 0.0;
    double s_data_interval = 0.0;

    clock_t t_c_old = 0, t_c_now = 0;
    clock_t t_s_old = 0, t_s_now = 0;

    fd_set read_fds;
    //added:
    fd_set write_fds;
    int fd_ready;

    syslog(LOG_INFO, "Started listen thread");

    while(i_running.load())
    {
        /**
        * added the write_fds.
        * so we added and wait for the writing descriptor.
        */
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        FD_SET(d_socket_rx, &read_fds);
        FD_SET(m_socket_rx, &read_fds);
        FD_SET(c_socket_rx, &read_fds);
        FD_SET(s_socket_rx, &read_fds);
        FD_SET(cfg_socket_tx, &write_fds);

        select_to.tv_sec = 1;
        select_to.tv_usec = 0;
        fd_ready = select(FD_SETSIZE, &read_fds,&write_fds,0, &select_to);
        if(fd_ready < 0)
       {
            syslog(LOG_CRIT, "Select error");
        }
        else if(fd_ready == 0)
        {
            i_connected.store(false); // no message received before timeout -> board is not active
            syslog(LOG_INFO, "Timeout expired, interrogator is not available");
        }
        else
        {
            if(!i_connected.load())  // first connection, send configuration
            {
                syslog(LOG_INFO, "Connection established with interrogator, sending initial configuration");
                ssi_send_maintenance_msg();
                i_connected.store(true); // message received -> board is active
            }

            if(FD_ISSET(d_socket_rx, &read_fds))
            {
                if((rec_len = recvfrom(d_socket_rx, buffer, MSG_LIMIT_MTU, 0, (struct sockaddr *) &(src), &l)) <0 )
                {
                    syslog(LOG_ERR, "Unable to read message");
                }
                else
                {
                    syslog(LOG_DEBUG, "Received packet of size %d from %s:%d on %s:%d", rec_len, inet_ntoa(src.sin_addr), ntohs(src.sin_port), inet_ntoa(d_src_rx.sin_addr), ntohs(d_src_rx.sin_port));
                    msg_q_put_ts(&msg_q, buffer, rec_len, MSG_TYPE_DIAG, msg_lock_m);
                    sem_post(&lock_s); //release parse thread
                }
                FD_CLR(d_socket_rx, &read_fds);
            }
            if(FD_ISSET(m_socket_rx, &read_fds))
            {
                if((rec_len = recvfrom(m_socket_rx, buffer, MSG_LIMIT_MTU, 0, (struct sockaddr *) &(src), &l)) <0 )
                {
                    syslog(LOG_ERR, "Unable to read message");
                }
                else
                {
                    syslog(LOG_DEBUG, "Received packet of size %d from %s:%d on %s:%d", rec_len, inet_ntoa(src.sin_addr), ntohs(src.sin_port), inet_ntoa(m_src_rx.sin_addr), ntohs(m_src_rx.sin_port));
                    msg_q_put_ts(&msg_q, buffer, rec_len, MSG_TYPE_MAIN, msg_lock_m);
                    sem_post(&lock_s); //release parse thread
                }
                FD_CLR(m_socket_rx, &read_fds);
            }
            if(FD_ISSET(c_socket_rx, &read_fds))
            {
                if((rec_len = recvfrom(c_socket_rx, buffer, MSG_LIMIT_MTU, 0, (struct sockaddr *) &(src), &l)) <0 )
                {
                    syslog(LOG_ERR, "Unable to read message");
                }
                else
                {
                    t_c_now = clock();
                    c_data_interval = (double)(t_c_now - t_c_old) / CLOCKS_PER_SEC * 1000;
                    syslog(LOG_DEBUG, "Interval between continuous data : %.6f ms", c_data_interval);

                    t_c_old = clock();

                    syslog(LOG_DEBUG, "Received packet of size %d from %s:%d on %s:%d", rec_len, inet_ntoa(src.sin_addr), ntohs(src.sin_port), inet_ntoa(c_src_rx.sin_addr), ntohs(c_src_rx.sin_port));
                    msg_q_put_ts(&msg_q, buffer, rec_len, MSG_TYPE_CONT, msg_lock_m);
                    sem_post(&lock_s); //release parse thread
                }
                FD_CLR(c_socket_rx, &read_fds);
            }
            if(FD_ISSET(s_socket_rx, &read_fds))
            {
                if((rec_len = recvfrom(s_socket_rx, buffer, MSG_LIMIT_MTU, 0, (struct sockaddr *) &(src), &l)) <0 )
                {
                    syslog(LOG_ERR, "Unable to read message");
                }
                else
                {
                    t_s_now = clock();
                    s_data_interval = (double)(t_s_now - t_s_old) / CLOCKS_PER_SEC * 1000;
                    syslog(LOG_DEBUG, "Interval between scan data : %.6f ms", s_data_interval);
                    t_s_old = clock();

                    syslog(LOG_DEBUG, "Received packet of size %d from %s:%d on %s:%d", rec_len, inet_ntoa(src.sin_addr), ntohs(src.sin_port), inet_ntoa(s_src_rx.sin_addr), ntohs(s_src_rx.sin_port));

                    msg_q_put_ts(&msg_q, buffer, rec_len, MSG_TYPE_SCAN, msg_lock_m);
                    sem_post(&lock_s); //release parse thread
                }
                FD_CLR(s_socket_rx, &read_fds);
            }
        }

        if((msg_q_size = msg_q_get_size(&msg_q)) > MSG_QUEUE_SIZE/2)
        {
            syslog(LOG_WARNING, "Current queue size is %lu. Reducing tx speed", msg_q_size);
        }
        
        usleep(100000);
    }

    sem_post(&lock_s); // release parse thread to exit

    return;
}

int Listener::ssi_send_maintenance_msg()
{
    socklen_t m_len = sizeof(m_dst_tx);

    int error_code = STATUS_OK;

    uint8_t tx_message[MSG_LIMIT_MTU];
    size_t current_index = 0;

    current_index += ssi_write_maint_header(tx_message);

    current_index += ssi_write_ch_format(tx_message + current_index, 0, 1, ssi_conf.ssi_gratings, ssi_conf.ssi_channels);

    current_index += ssi_write_cont_tx_rate(tx_message + current_index, ssi_conf.ssi_cont_speed);

    current_index += ssi_write_start_laser_ch(tx_message + current_index, 0);

    current_index += ssi_write_scan_speed(tx_message + current_index, 1, 400, 0);

    current_index += ssi_write_state(tx_message + current_index, SSI_STATE_OPERATIONAL);

    current_index += ssi_write_maint_padding(tx_message + current_index, current_index);

    syslog(LOG_INFO, "Sending default configuration");
    if((sendto(cfg_socket_tx, tx_message, current_index, 0, (struct sockaddr *) &m_dst_tx, m_len)) == -1)
    {
        syslog(LOG_ERR, "Unable to send message");
        error_code = STATUS_ERROR;
    }
    else
    {
        syslog(LOG_DEBUG, "Sent packet of length %lu from %s:%d to %s:%d", current_index, inet_ntoa(cfg_src_tx.sin_addr), ntohs(cfg_src_tx.sin_port), inet_ntoa(m_dst_tx.sin_addr), ntohs(m_dst_tx.sin_port));
    }

    return error_code;
}

int Listener::ssi_send_timestamp()
{
    uint8_t tx_message[MSG_LIMIT_MTU];
    socklen_t m_len = sizeof(m_dst_tx);

    int error_code = STATUS_OK;

    size_t current_index = 0;

    current_index += ssi_write_maint_header(tx_message);
    current_index += ssi_write_utc_local_time(tx_message);
    current_index += ssi_write_maint_padding(tx_message + current_index, current_index);

    syslog(LOG_INFO, "Sending current timestamp for time synchronization");
    if((sendto(cfg_socket_tx, tx_message, current_index, 0, (struct sockaddr *) &m_dst_tx, m_len)) == -1)
    {
        syslog(LOG_ERR, "Unable to send message");
        error_code = STATUS_ERROR;
    }
    else
    {
        syslog(LOG_DEBUG, "Sent packet of length %lu from %s:%d to %s:%d", current_index, inet_ntoa(cfg_src_tx.sin_addr), ntohs(cfg_src_tx.sin_port), inet_ntoa(m_dst_tx.sin_addr), ntohs(m_dst_tx.sin_port));
    }

    return error_code;
}

Listener::~Listener()
{
    if(listenerInitializedWell==true){
    ssi_socket_delete();

    // release parse thread to exit
    sem_post(&lock_s);
    parse_th.join();
    cout << "Parser  Thread  Joined  -->     Successfully             *" <<endl;
    syslog(LOG_DEBUG, "Parse Thread Joined Successfully");
    cout << "Client  Process Exited  -->     Successfully             *" <<endl;
    syslog(LOG_DEBUG, "Client Exited Successfully");
    cout <<"----------------------------------------------------------" << std::endl;

    msg_q_deinit(&msg_q);
    syslog(LOG_INFO, "Deleted message queue");

    // delete mutex
    pthread_mutex_destroy(msg_lock_m);

    //free(msg_lock_m);
    syslog(LOG_INFO, "Deleted mutexes");

    sem_destroy(&lock_s);
    syslog(LOG_INFO, "Deleted semaphore");
    //free(timeinfo);
}
};

Listener::Listener()
{
    i_running.store(false);
    i_connected.store(false);

};

