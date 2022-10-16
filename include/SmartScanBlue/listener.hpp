/*******************************************************************************
* smartscan interrogator interface
*******************************************************************************/
#ifndef LISTENER_H
#define LISTENER_H

/*******************************************************************************
* included libraries
*******************************************************************************/
// C libraries
#include <stdint.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/select.h>
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

// local libraries
extern "C"
{
#include "../CommonLibraries/libutils/utils.h"
#include "../CommonLibraries/libsmartscan/socket.h"
#include "../CommonLibraries/libsmartscan/msg_queue.h"
#include "../CommonLibraries/libsmartscan/smartscan_utils.h"
}
#include "config.hpp"
#include "interrogator.hpp"
#include "parser.hpp"

using namespace std;
/*******************************************************************************
* constants
*******************************************************************************/
#define SELECT_TIMEOUT_SEC 20
#define MSG_QUEUE_SIZE 1024
#define LISTEN_IP "0.0.0.0"

/*******************************************************************************
* classes
*******************************************************************************/
class Listener : public Interrogator
{
private:

    // socket data
    int d_socket_rx, m_socket_rx, c_socket_rx, s_socket_rx, cfg_socket_tx;
    struct sockaddr_in d_src_rx, m_src_rx, c_src_rx, s_src_rx;
    struct sockaddr_in cfg_src_tx, d_dst_tx, m_dst_tx;
    // socket methods
    int ssi_socket_init();
    void ssi_socket_delete();
    // send message methods
    int ssi_send_maintenance_msg();
    int ssi_send_timestamp();

public:

    thread parse_th;
    uint8_t               ssi_state;    // smartscan interrogator state
    SSI_CONFIG            ssi_conf;     // interrogator configuration
    msg_queue             msg_q;        // UDP message queue
    sem_t lock_s;         // semaphore on parse thread (listen thread unlocks parsing)
    atomic<bool> i_connected;
    atomic<bool> i_running;
    pthread_mutex_t *msg_lock_m;    // mutex on message queue

    Listener();
   ~Listener();
    int  i_init();
    void ssi_listen();
};

#endif
