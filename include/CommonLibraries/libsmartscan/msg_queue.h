/*******************************************************************************
* Created by Mauro Guerrera on 05/07/18.
*******************************************************************************/

#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

/*******************************************************************************
* included libraries
*******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <syslog.h>

#include <libutils/utils.h>

/*******************************************************************************
* message queue structure
*******************************************************************************/
typedef struct msg_queue
{
    uint8_t **queue;
    uint8_t *msg_type;
    size_t *msg_len;
    size_t head;
    size_t tail;
    size_t cur_size;  // current queue size
    size_t max_size;  // max queue size
    size_t buf_size;  // size of a single line
} msg_queue;

/*******************************************************************************
* message queue functions (_ts are thread safe versions)
*******************************************************************************/
int msg_q_init(msg_queue *q, size_t q_size, size_t buf_size);
int msg_q_deinit(msg_queue *q);

int msg_q_put(msg_queue *q, const uint8_t* data, size_t size, uint8_t type);
int msg_q_put_ts(msg_queue *q, const uint8_t* data, size_t size, uint8_t type, pthread_mutex_t *lock);

int msg_q_get(msg_queue *q, uint8_t *data, size_t *size, uint8_t *type);
int msg_q_get_ts(msg_queue *q, uint8_t *data, size_t *size, uint8_t *type, pthread_mutex_t *lock);

int msg_q_empty(msg_queue *q);
int msg_q_full(msg_queue *q);

size_t msg_q_get_size(msg_queue *q);

#endif //MSG_QUEUE_H
