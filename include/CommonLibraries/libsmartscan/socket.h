#ifndef SOCKET_H
#define SOCKET_H

/*******************************************************************************
* included libraries
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <syslog.h>

#include <libutils/utils.h>

/*******************************************************************************
* data structures
*******************************************************************************/
typedef struct socket_s
{
  uint8_t init;

  struct sockaddr_in src, dest;     // socket endpoints
  int socket_fd;                    // socket file descriptor

  uint32_t msg_limit;               // size limit
} socket_s;

/*******************************************************************************
* tx functions
// *******************************************************************************/
int init_tx_socket(socket_s *s, int domain, int type, int protocol, char *src_addr, uint16_t tx_port, uint32_t msg_limit);
int delete_tx_socket(socket_s *s);
int send_data(socket_s *s, char *dest_addr, uint16_t dest_port, uint8_t *data, size_t len);

/*******************************************************************************
* functions
*******************************************************************************/
int init_rx_socket(socket_s *s, int domain, int type, int protocol, char *src_addr, uint16_t rx_port, uint32_t msg_limit, int read_timeout);
int delete_rx_socket(socket_s *s);
int read_data(socket_s *s, uint8_t *data, size_t len);

#endif
