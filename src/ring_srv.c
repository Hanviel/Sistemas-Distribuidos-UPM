#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <pthread.h>
#include "ring.h"
#include "common.h"
#include <sys/socket.h>

extern unsigned int my_ip;
extern unsigned short my_port;
extern unsigned int suc_ip;
extern unsigned short suc_port;
extern char my_dir[256];

void *worker_thread(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    char op;
    if (recv(client_sock, &op, sizeof(char), MSG_WAITALL) != sizeof(char)) {
        close(client_sock);
        return NULL;
    }

    if (op == 'P') {
        int pid = htonl(getpid());
        send(client_sock, &pid, sizeof(int), 0);
    }
    else if (op == 'S') {
        send(client_sock, &suc_ip, sizeof(unsigned int), MSG_MORE);
        send(client_sock, &suc_port, sizeof(unsigned short), 0);
    }
    else if (op == 'J') {
        unsigned short new_port;
        recv(client_sock, &new_port, sizeof(unsigned short), MSG_WAITALL);

        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        getpeername(client_sock, (struct sockaddr *)&addr, &addr_len);

        send(client_sock, &suc_ip, sizeof(unsigned int), MSG_MORE);
        send(client_sock, &suc_port, sizeof(unsigned short), 0);

        suc_ip = addr.sin_addr.s_addr;
        suc_port = new_port;
    }
    else if (op == 'U') {
        unsigned int ss_ip;
        unsigned short ss_port;
        ring_remote_successor(suc_ip, suc_port, &ss_ip, &ss_port);
        send(client_sock, &ss_ip, sizeof(unsigned int), MSG_MORE);
        send(client_sock, &ss_port, sizeof(unsigned short), 0);
    }
    else if (op == 'L') {
        int net_hops, net_len;
        recv(client_sock, &net_hops, sizeof(int), MSG_WAITALL);
        recv(client_sock, &net_len, sizeof(int), MSG_WAITALL);
        int hops = ntohl(net_hops);
        int len = ntohl(net_len);

        char *filename = malloc(len);
        recv(client_sock, filename, len, MSG_WAITALL);

        char path[512];
        sprintf(path, "%s/%s", my_dir, filename);

        if (access(path, F_OK) != -1) {
            int res = htonl(0);
            struct iovec iov[3];
            iov[0].iov_base = &res; iov[0].iov_len = sizeof(int);
            iov[1].iov_base = &my_ip; iov[1].iov_len = sizeof(unsigned int);
            iov[2].iov_base = &my_port; iov[2].iov_len = sizeof(unsigned short);
            writev(client_sock, iov, 3);
        } else if (hops == 0) {
            int res = htonl(-1);
            send(client_sock, &res, sizeof(int), 0);
        } else {
            int fwd_sock = create_socket_cln(suc_ip, suc_port);
            if (fwd_sock >= 0) {
                char fwd_op = 'L';
                int fwd_hops = htonl(hops - 1);
                struct iovec iov[4];
                iov[0].iov_base = &fwd_op; iov[0].iov_len = sizeof(char);
                iov[1].iov_base = &fwd_hops; iov[1].iov_len = sizeof(int);
                iov[2].iov_base = &net_len; iov[2].iov_len = sizeof(int);
                iov[3].iov_base = filename; iov[3].iov_len = len;
                writev(fwd_sock, iov, 4);

                int fwd_res;
                recv(fwd_sock, &fwd_res, sizeof(int), MSG_WAITALL);
                if (ntohl(fwd_res) == 0) {
                    unsigned int ans_ip;
                    unsigned short ans_port;
                    recv(fwd_sock, &ans_ip, sizeof(unsigned int), MSG_WAITALL);
                    recv(fwd_sock, &ans_port, sizeof(unsigned short), MSG_WAITALL);

                    struct iovec iov_in[3];
                    iov_in[0].iov_base = &fwd_res; iov_in[0].iov_len = sizeof(int);
                    iov_in[1].iov_base = &ans_ip; iov_in[1].iov_len = sizeof(unsigned int);
                    iov_in[2].iov_base = &ans_port; iov_in[2].iov_len = sizeof(unsigned short);
                    writev(client_sock, iov_in, 3);
                } else {
                    int res = htonl(-1);
                    send(client_sock, &res, sizeof(int), 0);
                }
                close(fwd_sock);
            } else {
                int res = htonl(-1);
                send(client_sock, &res, sizeof(int), 0);
            }
        }
        free(filename);
    }
    else if (op == 'D') {
        int net_len;
        recv(client_sock, &net_len, sizeof(int), MSG_WAITALL);
        int len = ntohl(net_len);

        char *filename = malloc(len);
        recv(client_sock, filename, len, MSG_WAITALL);

        char path[512];
        sprintf(path, "%s/%s", my_dir, filename);

        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            int res = htonl(-1);
            send(client_sock, &res, sizeof(int), 0);
        } else {
            struct stat st;
            fstat(fd, &st);
            int filesize = st.st_size;
            int net_filesize = htonl(filesize);
            send(client_sock, &net_filesize, sizeof(int), MSG_MORE);
            sendfile(client_sock, fd, NULL, filesize);
            close(fd);
        }
        free(filename);
    }

    close(client_sock);
    return NULL;
}

void *server_thread(void *arg){
    int srv_sock = *(int *)arg;
    free(arg);

    while (1) {
        int client_sock = accept(srv_sock, NULL, NULL);
        if (client_sock < 0) continue;

        int *sock_ptr = malloc(sizeof(int));
        *sock_ptr = client_sock;
        create_thread(worker_thread, sock_ptr);
    }
    return NULL;
}

