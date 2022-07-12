/*******************************************************************************
* Created by Mauro Guerrera on 05/07/18.
*******************************************************************************/

#ifndef DATA_QUEUE_H
#define DATA_QUEUE_H

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
* data queue structure
*******************************************************************************/
typedef struct data_queue
{
    void **queue;
    size_t *data_size;
    size_t head;
    size_t tail;
    size_t cur_size;  // current queue size
    size_t max_size;  // max queue size
} data_queue;

/*******************************************************************************
* data queue functions (_ts are thread safe versions)
*******************************************************************************/
int data_q_init(data_queue *q, size_t q_size);
int data_q_deinit(data_queue *q);

int data_q_put(data_queue *q, const void* data, size_t size);
int data_q_put_ts(data_queue *q, const void* data, size_t size, pthread_mutex_t *lock);

int data_q_get(data_queue *q, void *data, size_t size);
int data_q_get_ts(data_queue *q, void *data, size_t size, pthread_mutex_t *lock);

int data_q_empty(data_queue *q);
int data_q_full(data_queue *q);

size_t data_q_get_size(data_queue *q);

#endif //DATA_QUEUE_H
