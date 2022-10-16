/*******************************************************************************
* Created by Mauro Guerrera on 05/07/18.
*******************************************************************************/

/*******************************************************************************
* included libraries
*******************************************************************************/
#include "../../../include/CommonLibraries/libsmartscan/data_queue.h"

/*******************************************************************************
* data queue functions (_ts are thread safe versions)
*******************************************************************************/
int data_q_init(data_queue *q, size_t q_size)
{
    int exit_code = STATUS_OK;

    syslog(LOG_DEBUG, "data_q_init: initialising data queue of size %ld", q_size);

    if(!q)
    {
        syslog(LOG_DEBUG, "data_q_init: Invalid pointer to data queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        q->queue = (void **) malloc(q_size * sizeof(void *));

        if(!q->queue)
        {
            syslog(LOG_DEBUG, "data_q_init: unable to allocate memory");
            exit_code = STATUS_ERROR;
        }

        q->data_size = (size_t *) malloc(q_size * (sizeof(size_t)));

        if(!q->data_size)
        {
            syslog(LOG_DEBUG, "data_q_init: unable to allocate memory");
            exit_code = STATUS_ERROR;
        }

        if(exit_code != STATUS_ERROR)
        {
            q->head = 0;
            q->tail = 0;
            q->cur_size = 0;
            q->max_size = q_size;
        }
    }

    return exit_code;
};

int data_q_deinit(data_queue *q)
{
    int exit_code = STATUS_OK;
    int i;

    syslog(LOG_DEBUG, "data_q_deinit: Deinit data queue");

    if(!q)
    {
      syslog(LOG_DEBUG, "data_q_deinit: invalid pointer to data queue structure");
      exit_code = STATUS_ERROR;
    }
    else
    {
      while(!data_q_empty(q))
      {
        free(q->queue[q->tail]);
        q->data_size[q->tail] = 0;

        q->tail = ((q->tail + 1) % q->max_size) % q->max_size;
        q->cur_size--;
      }

      free(q->data_size);

      free(q->queue);

      q->head = 0;
      q->tail = 0;
      q->cur_size = 0;
      q->max_size = 0;
    }

    return exit_code;
};

int data_q_put(data_queue *q, const void* data, size_t size)
{
    int exit_code = STATUS_OK;

    if(!q)
    {
        syslog(LOG_DEBUG, "data_q_put: invalid pointer to data queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        if(((q->head + 1) % q->max_size) == q->tail)
        {
            syslog(LOG_DEBUG, "data_q_put: Reached max buffer size");

        }
        else
        {
            syslog(LOG_DEBUG, "data_q_put: Insert data into data queue");

            if(!(q->queue[q->head] = (void *) malloc(size)))
            {
                syslog(LOG_DEBUG, "data_q_put: Unable to allocate memory");
                exit_code = STATUS_ERROR;
            }
            else
            {
              memcpy(q->queue[q->head], data, size);
              q->data_size[q->head] = size;

              q->head = (q->head + 1) % q->max_size;
              q->cur_size++;
            }
        }
    }

    return exit_code;
};

int data_q_put_ts(data_queue *q, const void* data, size_t size, pthread_mutex_t *lock)
{
    int exit_code = STATUS_OK;

    if(!q)
    {
        syslog(LOG_DEBUG, "data_q_put_ts: invalid pointer to data queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        if(((q->head + 1) % q->max_size) == q->tail)
        {
            syslog(LOG_DEBUG, "data_q_put_ts: reached max buffer size");

        }
        else
        {
            syslog(LOG_DEBUG, "data_q_put_ts: insert data into data queue");

            if(!(q->queue[q->head] = (void *) malloc(size)))
            {
                syslog(LOG_DEBUG, "data_q_put_ts: unable to allocate memory");
                exit_code = STATUS_ERROR;
            }
            else
            {
              pthread_mutex_lock(lock);

              memcpy(q->queue[q->head], data, size);
              q->data_size[q->head] = size;

              q->head = (q->head + 1) % q->max_size;
              q->cur_size++;

              pthread_mutex_unlock(lock);
            }
        }
    }

    return exit_code;
};

int data_q_get(data_queue *q, void *data, size_t size)
{
    int exit_code = STATUS_OK;

    if(!q)
    {
        syslog(LOG_DEBUG, "data_q_get: invalid pointer to data queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        if(!data)
        {
            syslog(LOG_DEBUG, "data_q_get: invalid pointer to data");
            exit_code = STATUS_ERROR;
        }
        else
        {
            if(q->head == q->tail)
            {
                syslog(LOG_DEBUG, "data_q_get: data queue is empty");
                exit_code = STATUS_ERROR;
            }
            else
            {
                syslog(LOG_DEBUG, "data_q_get: get data from data queue");

                if(q->data_size[q->tail] != size)
                {
                  syslog(LOG_DEBUG, "data_q_get: size of objects mismatch");
                  exit_code = STATUS_ERROR;
                }
                else
                {
                  memcpy(data, q->queue[q->tail], q->data_size[q->tail]);

                  free(q->queue[q->tail]);
                  q->data_size[q->tail] = 0;

                  q->tail = ((q->tail + 1) % q->max_size) % q->max_size;
                  q->cur_size--;
                }
            }
        }

    }

    return exit_code;
};

int data_q_get_ts(data_queue *q, void *data, size_t size, pthread_mutex_t *lock)
{
    int exit_code = STATUS_OK;

    if(!q)
    {
        syslog(LOG_DEBUG, "data_q_get_ts: invalid pointer to data queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        if(!data)
        {
            syslog(LOG_DEBUG, "data_q_get_ts: invalid pointer to data");
            exit_code = STATUS_ERROR;
        }
        else
        {
            if(q->head == q->tail)
            {
                syslog(LOG_DEBUG, "data_q_get_ts: data queue is empty");
                exit_code = STATUS_ERROR;
            }
            else
            {
                syslog(LOG_DEBUG, "data_q_get_ts: get data from data queue");

                if(q->data_size[q->tail] != size)
                {
                  syslog(LOG_DEBUG, "data_q_get_ts: size of objects mismatch");
                  exit_code = STATUS_ERROR;
                }
                else
                {
                  pthread_mutex_lock(lock);

                  memcpy(data, q->queue[q->tail], q->data_size[q->tail]);

                  free(q->queue[q->tail]);
                  q->data_size[q->tail] = 0;

                  q->tail = ((q->tail + 1) % q->max_size) % q->max_size;
                  q->cur_size--;

                  pthread_mutex_unlock(lock);
                }
            }
        }

    }

    return exit_code;
};

int data_q_empty(data_queue *q)
{
    int exit_code = 0;

    if(!q)
    {
        syslog(LOG_DEBUG, "data_q_empty: invalid pointer to data queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        exit_code = (q->head == q->tail);
    }

    return exit_code;
};

int data_q_full(data_queue *q)
{
    int exit_code = 0;

    if(!q)
    {
        syslog(LOG_DEBUG, "data_q_full: invalid pointer to data queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        exit_code = (((q->head + 1) % q->max_size) == q->tail);
    }

    return exit_code;
};

size_t data_q_get_size(data_queue *q)
{
    // char log_message[MAX_LOG_MSG_SIZE];
    size_t ret_value = 0;

    if(!q)
    {
        syslog(LOG_DEBUG, "data_q_get_size: invalid pointer to data queue structure");
    }
    else
    {
        ret_value = q->cur_size;
    }

    return ret_value;
};
