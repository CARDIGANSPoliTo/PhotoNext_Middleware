/*******************************************************************************
* Created by Mauro Guerrera on 05/07/18.
*******************************************************************************/

/*******************************************************************************
* included libraries
*******************************************************************************/
#include "../../../include/CommonLibraries/libsmartscan/msg_queue.h"

/*******************************************************************************
* message queue functions (_ts are thread safe versions)
*******************************************************************************/
int msg_q_init(msg_queue *q, size_t q_size, size_t buf_size)
{
    int exit_code = STATUS_OK;
    int i;

    syslog(LOG_DEBUG, "msg_q_init: initialising message queue of size %ld", q_size);

    if(!q)
    {
        syslog(LOG_DEBUG, "msg_q_init: invalid pointer to message queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        q->queue = (uint8_t **) malloc(q_size * sizeof(uint8_t *));

        if(!q->queue)
        {
            syslog(LOG_DEBUG, "msg_q_init: unable to allocate memory");
            exit_code = STATUS_ERROR;
        }
        else
        {
            i = 0;
            while(i<q_size && exit_code != STATUS_ERROR)
            {
                if(!(q->queue[i] = (uint8_t *) malloc(buf_size * sizeof(uint8_t))))
                {
                    syslog(LOG_DEBUG, "msg_q_init: unable to allocate memory");
                    exit_code = STATUS_ERROR;
                }
                i++;
            }
        }

        q->msg_type = (uint8_t *) malloc(q_size * sizeof(uint8_t));

        if(!q->msg_type)
        {
            syslog(LOG_DEBUG, "msg_q_init: unable to allocate memory");
            exit_code = STATUS_ERROR;
        }

        q->msg_len = (size_t *) malloc(q_size * (sizeof(size_t)));

        if(!q->msg_len)
        {
            syslog(LOG_DEBUG, "msg_q_init: unable to allocate memory");
            exit_code = STATUS_ERROR;
        }

        if(exit_code != STATUS_ERROR)
        {
            q->head = 0;
            q->tail = 0;
            q->cur_size = 0;
            q->max_size = q_size;
            q->buf_size = buf_size;
        }
    }

    return exit_code;
};

int msg_q_deinit(msg_queue *q)
{
    int exit_code = STATUS_OK;
    int i;

    syslog(LOG_DEBUG, "msg_q_deinit: deinit message queue");

    if(!q)
    {
        syslog(LOG_DEBUG, "msg_q_deinit: invalid pointer to message queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        for(i=0; i < q->max_size; i++)
        {
            free(q->queue[i]);
        }

        free(q->msg_len);

        free(q->msg_type);

        free(q->queue);

        q->head = 0;
        q->tail = 0;
        q->cur_size = 0;
        q->max_size = 0;
    }

    return exit_code;
};

int msg_q_put(msg_queue *q, const uint8_t* data, size_t size, uint8_t type)
{
    int exit_code = STATUS_OK;

    if(!q)
    {
        syslog(LOG_DEBUG, "msg_q_put: invalid pointer to message queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        if(((q->head + 1) % q->max_size) == q->tail)
        {
            syslog(LOG_DEBUG, "msg_q_put: reached max buffer size");

        }
        else
        {
            syslog(LOG_DEBUG, "msg_q_put: insert data into message queue");

            memcpy(q->queue[q->head], data, size);
            q->msg_type[q->head] = type;
            q->msg_len[q->head] = size;

            q->head = (q->head + 1) % q->max_size;
            q->cur_size++;

            // q_get_size(q);
        }
    }

    return exit_code;
};

int msg_q_put_ts(msg_queue *q, const uint8_t* data, size_t size, uint8_t type, pthread_mutex_t *lock)
{
    int exit_code = STATUS_OK;

    if(!q)
    {
        syslog(LOG_DEBUG, "msg_q_put_ts: invalid pointer to message queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        if(((q->head + 1) % q->max_size) == q->tail)
        {
            syslog(LOG_DEBUG, "msg_q_put_ts: reached max buffer size");

        }
        else
        {
            syslog(LOG_DEBUG, "msg_q_put_ts: insert data into message queue");

            pthread_mutex_lock(lock);

            memcpy(q->queue[q->head], data, size);
            q->msg_type[q->head] = type;
            q->msg_len[q->head] = size;

            q->head = (q->head + 1) % q->max_size;
            q->cur_size++;

            pthread_mutex_unlock(lock);

            // q_get_size(q);
        }
    }

    return exit_code;
};

int msg_q_get(msg_queue *q, uint8_t *data, size_t *size, uint8_t *type)
{
    int exit_code = STATUS_OK;

    if(!q)
    {
        syslog(LOG_DEBUG, "msg_q_get: invalid pointer to message queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        if(!data)
        {
            syslog(LOG_DEBUG, "msg_q_get: invalid pointer to data");
            exit_code = STATUS_ERROR;
        }
        else
        {
            if(q->head == q->tail)
            {
                syslog(LOG_DEBUG, "msg_q_get: message queue is empty");
                exit_code = STATUS_ERROR;
            }
            else
            {
                syslog(LOG_DEBUG, "msg_q_get: get data from message queue");

                memcpy(data, q->queue[q->tail], q->msg_len[q->tail]);
                *type = q->msg_type[q->tail];
                *size = q->msg_len[q->tail];

                q->tail = ((q->tail + 1) % q->max_size) % q->max_size;
                q->cur_size--;

                // q_get_size(q);
            }
        }

    }

    return exit_code;
};

int msg_q_get_ts(msg_queue *q, uint8_t *data, size_t *size, uint8_t *type, pthread_mutex_t *lock)
{
    int exit_code = STATUS_OK;

    if(!q)
    {
        syslog(LOG_DEBUG, "msg_q_get_ts: invalid pointer to message queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        if(!data)
        {
            syslog(LOG_DEBUG, "msg_q_get_ts: invalid pointer to data");
            exit_code = STATUS_ERROR;
        }
        else
        {
            if(q->head == q->tail)
            {
                syslog(LOG_DEBUG, "msg_q_get_ts: message queue is empty");
                exit_code = STATUS_ERROR;
            }
            else
            {
                syslog(LOG_DEBUG, "msg_q_get_ts: get data from message queue");

                pthread_mutex_lock(lock);

                memcpy(data, q->queue[q->tail], q->msg_len[q->tail]);
                *type = q->msg_type[q->tail];
                *size = q->msg_len[q->tail];

                q->tail = ((q->tail + 1) % q->max_size) % q->max_size;
                q->cur_size--;

                pthread_mutex_unlock(lock);

                // q_get_size(q);
            }
        }

    }

    return exit_code;
};

int msg_q_empty(msg_queue *q)
{
    int exit_code = 0;

    if(!q)
    {
        syslog(LOG_DEBUG, "msg_q_empty: invalid pointer to message queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        exit_code = (q->head == q->tail);
    }

    return exit_code;
};

int msg_q_full(msg_queue *q)
{
    int exit_code = 0;

    if(!q)
    {
        syslog(LOG_DEBUG, "msg_q_full: invalid pointer to message queue structure");
        exit_code = STATUS_ERROR;
    }
    else
    {
        exit_code = (((q->head + 1) % q->max_size) == q->tail);
    }

    return exit_code;
};

size_t msg_q_get_size(msg_queue *q)
{
    size_t ret_value = 0;

    if(!q)
    {
        syslog(LOG_DEBUG, "msg_q_get_size: invalid pointer to message queue structure");
    }
    else
    {
        ret_value = q->cur_size;
    }

    return ret_value;
};
