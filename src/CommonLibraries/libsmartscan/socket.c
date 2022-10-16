/*******************************************************************************
* included libraries
*******************************************************************************/
#include "../../../include/CommonLibraries/libsmartscan/socket.h"

/*******************************************************************************
* rx functions
*******************************************************************************/
int init_tx_socket(socket_s *s, int domain, int type, int protocol, char *src_addr, uint16_t tx_port, uint32_t msg_limit)
{
  int error_code = STATUS_OK;
  char log_message[MAX_LOG_MSG_SIZE];

  if(!s)
  {
    syslog(LOG_DEBUG, "init_tx_socket: invalid socket structure");
    error_code = STATUS_ERROR;
  }
  else
  {
    if(s->init)
    {
      syslog(LOG_DEBUG, "init_tx_socket: socket is already initialised");
      error_code = STATUS_ERROR;
    }

    // create socket
    if ((s->socket_fd = socket(domain, type, protocol)) == -1)
    {
      syslog(LOG_DEBUG, "init_tx_socket: unable to open socket");
      error_code = STATUS_ERROR;
    }

    if(!error_code)
    {
      // zero out the socket structures
      memset((char *) &(s->src), 0, sizeof(s->src));
      memset((char *) &(s->dest), 0, sizeof(s->dest));

      // set local and remote endpoint domain
      s->src.sin_family = domain;
      s->dest.sin_family = domain;

      s->msg_limit = msg_limit;

      // set local port and address
      s->src.sin_port = htons(tx_port);

      // set src address
      if (inet_aton(src_addr, &(s->src.sin_addr)) == 0)
      {
        syslog(LOG_DEBUG, "init_tx_socket: invalid source IP address");
        error_code = STATUS_ERROR;
      }
      else
      {
        //bind socket to port
        if( bind(s->socket_fd , (struct sockaddr*)&(s->src), sizeof(s->src) ) == -1)
        {
          syslog(LOG_DEBUG, "init_tx_socket: socket bind failed");
          error_code = STATUS_ERROR;
        }
        else
        {
          syslog(LOG_DEBUG, "init_tx_socket: open tx socket on %s:%d", inet_ntoa(s->src.sin_addr), ntohs(s->src.sin_port));
          s->init = 1;
        }
      }
    }
  }

  return error_code;
};

int delete_tx_socket(socket_s *s)
{
  int error_code = STATUS_OK;

  if(!s)
  {
    syslog(LOG_DEBUG, "delete_tx_socket: invalid socket structure");
    error_code = STATUS_ERROR;
  }
  else
  {
    if(s->init)
    {
      s->init = 0;
      close(s->socket_fd);
    }
    else
    {
      syslog(LOG_DEBUG, "delete_tx_socket: socket not initialised");
      error_code = STATUS_ERROR;
    }
  }

  return error_code;
};

int send_data(socket_s *s, char *dest_addr, uint16_t dest_port, uint8_t *data, size_t len)
{
  int error_code = STATUS_OK;

  if(!s)
  {
    syslog(LOG_DEBUG, "send_data: invalid socket structure");
    error_code = STATUS_ERROR;
  }
  else
  {
    if(s->init)
    {
      // set remote endpoint
      s->dest.sin_port = htons(dest_port);

      if (inet_aton(dest_addr, &(s->dest.sin_addr)) == 0)
      {
        syslog(LOG_DEBUG, "send_data: invalid destination IP address");
        error_code = STATUS_ERROR;
      }
      else
      {
        if(len > s->msg_limit)
        {
          syslog(LOG_DEBUG, "send_data: message is longer than maximum allowed size");
          error_code = STATUS_ERROR;
        }
        else
        {
          if (sendto(s->socket_fd, data, len, 0 , (struct sockaddr *) &(s->dest),(socklen_t) sizeof(s->dest)) == -1)
          {
            syslog(LOG_DEBUG, "send_data: unable to send message");
            error_code = STATUS_ERROR;
          }
          else
          {
            //print details of the client/peer and the data sent
            snprintf(LOG_DEBUG, "send_data: sent packet from %s:%d to %s:%d", inet_ntoa(s->src.sin_addr), ntohs(s->src.sin_port), inet_ntoa(s->dest.sin_addr), ntohs(s->dest.sin_port));
          }
        }
      }
    }
    else
    {
      syslog(LOG_DEBUG, "send_data: socket not initialised");
      error_code = STATUS_ERROR;
    }
  }

  return error_code;
};

/*******************************************************************************
* rx functions
*******************************************************************************/
int init_rx_socket(socket_s *s, int domain, int type, int protocol, char *src_addr, uint16_t rx_port, uint32_t msg_limit, int read_timeout)
{
  int error_code = STATUS_OK;

  struct timeval tv;
  tv.tv_sec = read_timeout;
  tv.tv_usec = 0;

  if(!s)
  {
    syslog(LOG_DEBUG, "init_rx_socket: invalid socket structure");
    error_code = STATUS_ERROR;
  }
  else
  {
    if(s->init)
    {
      syslog(LOG_DEBUG, "init_rx_socket: socket is already initialised");
      error_code = STATUS_ERROR;
    }

    // create socket
    if ((s->socket_fd = socket(domain, type, protocol)) == -1)
    {
      syslog(LOG_DEBUG, "init_rx_socket: unable to open socket");
      error_code = STATUS_ERROR;
    }

    if(!error_code)
    {
      // zero out the socket structures
      memset((char *) &(s->src), 0, sizeof(s->src));
      memset((char *) &(s->dest), 0, sizeof(s->dest));

      // set local endpoint
      s->dest.sin_family = domain;
      s->dest.sin_port = htons(rx_port);

      s->msg_limit = msg_limit;

      if (inet_aton(src_addr, &(s->dest.sin_addr)) == 0)
      {
        syslog(LOG_DEBUG, "init_rx_socket: invalid source IP address");
        error_code = STATUS_ERROR;
      }
      else
      {
        //bind socket to port
        if( bind(s->socket_fd , (struct sockaddr*)&(s->dest), sizeof(s->dest) ) == -1)
        {
          syslog(LOG_DEBUG, "init_rx_socket: socket bind failed");
          error_code = STATUS_ERROR;
        }
        else
        {
          if (setsockopt(s->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            syslog(LOG_DEBUG, "init_rx_socket: unable to set read timeout");
          }

          syslog(LOG_DEBUG, "init_rx_socket: open rx socket on %s:%d", inet_ntoa(s->dest.sin_addr), ntohs(s->dest.sin_port));
          s->init = 1;
        }
      }
    }
  }

  return error_code;
};

int delete_rx_socket(socket_s *s)
{
  int error_code = STATUS_OK;

  if(!s)
  {
    syslog(LOG_DEBUG, "delete_rx_socket: invalid socket structure");
    error_code = STATUS_ERROR;
  }
  else
  {
    if(s->init)
    {
      s->init = 0;
      close(s->socket_fd);
    }
    else
    {
      syslog(LOG_DEBUG, "delete_rx_socket: socket not initialised");
      error_code = STATUS_ERROR;
    }
  }

  return error_code;
};

int read_data(socket_s *s, uint8_t *data, size_t len)
{
  int rec_len = -1;
  socklen_t src_len = sizeof(s->src);

  if(!s)
  {
    syslog(LOG_DEBUG, "read_data: invalid socket structure");
  }
  else
  {
    if(s->init)
    {
      syslog(LOG_DEBUG, "read_data: waiting for data");

      // zero out the message buffer
      memset((void *)data,'\0', len);

      //try to receive some data, this is a blocking call
      if ((rec_len = recvfrom(s->socket_fd, data, s->msg_limit, 0, (struct sockaddr *) &(s->src), &src_len)) < 0)
      {
        syslog(LOG_DEBUG, "read_data: data read from socket failed");
      }
      else
      {
        //print details of the client/peer and the data received
        syslog("read_data: received packet of size %d from %s:%d on %s:%d", rec_len, inet_ntoa(s->src.sin_addr), ntohs(s->src.sin_port), inet_ntoa(s->dest.sin_addr), ntohs(s->dest.sin_port));
      }
    }
    else
    {
      syslog(LOG_DEBUG, "read_data: socket not initialised");
    }
  }

  return rec_len;
};
