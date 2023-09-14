#include "transport.h"
#include "session.h"

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

int ftp_recv_data(struct socket_data* socket_data, int recv_socket)
{
    int res = -1;
    socket_data->trans_length = 0;
    socket_data->index_begin = 0;
    socket_data->index_end = 0;
    
    res = safe_recv(recv_socket, socket_data->trans_buf, BUF_SIZE * 2, &socket_data->trans_length);
    if (ENOTCONN == res)
        return res;
    socket_data->trans_buf[socket_data->trans_length] = '\0';
    return res;
}

int ftp_send_data(struct socket_data* socket_data, int send_socket)
{
    int res = -1;
    res = safe_send(send_socket, socket_data->trans_buf, socket_data->trans_length, &socket_data->trans_length);
    
    return res;
}

int safe_send(int fd, char* buf, int buf_size, int* send_length)
{
    int res = -1;
    int send_bytes = 0;
    do {
        res = send(fd, buf, buf_size, MSG_NOSIGNAL);
        if (res < 0) {
            //系统调用
            if ((errno == EINTR) || (errno == EAGAIN)) {
                usleep(1000);
                continue;
            }

            //其余情况
            return ENOTCONN;
        }
        else if (res == 0) {
            return ENOTCONN;
        }

        send_bytes += res;
        if (send_bytes < buf_size) {
            usleep(1000);
            continue;
        }
            
        break;
    } while (1);
    if (send_length != NULL) {
        *send_length = send_bytes;
    }
    return 0;
}

int safe_recv(int fd, char* buf, int buf_size, int* recv_length)
{
    int recv_bytes = 0;
    do {
        recv_bytes = 0;
        recv_bytes = recv(fd, buf, buf_size, 0);
        if (recv_bytes > 0)
            break;
        else if (recv_bytes == 0)
            return ENOTCONN;
        else {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN)
                return EAGAIN;
        }
    } while (1);
    if (recv_length != NULL) {
        *recv_length = recv_bytes;
    }
    return 0;
}

