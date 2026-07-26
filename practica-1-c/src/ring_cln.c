// FUNCIONALIDAD DE LA PARTE CLIENTE
#include <sys/mman.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/stat.h>

#include "ring.h"
#include "common.h"

static int is_initialized(void);
static int initialize(void);

extern void *server_thread(void *arg);

unsigned int my_ip;
unsigned short my_port;
unsigned int suc_ip;
unsigned short suc_port;
char my_dir[256];

// inicia el nodo añadiéndolo a la red P2P si ya está creada;
// los puertos e IPs deben estar en formato de red;
// debe devolver en el último parámetro el puerto reservado en formato red;
// retorna 0 si OK y -1 si error
int ring_init(const char *shrd_dir, unsigned int local_ip, unsigned int remote_ip, unsigned short remote_port, unsigned short *alloc_port) {
    if (initialize()) return -1; // ya está inicializada

    my_ip = local_ip;
    strcpy(my_dir, shrd_dir);

    int srv_sock = create_socket_srv(&my_port);
    if (srv_sock < 0) return -1;

    *alloc_port = my_port;

    int *sock_ptr = malloc(sizeof(int));
    *sock_ptr = srv_sock;
    create_thread(server_thread, sock_ptr);

    if (remote_ip != 0 || remote_port != 0) {
        int sock = create_socket_cln(remote_ip, remote_port);
        if (sock < 0) return -1;
        
        char op = 'J';
        struct iovec iov[2];
        iov[0].iov_base = &op;
        iov[0].iov_len = sizeof(char);
        iov[1].iov_base = &my_port;
        iov[1].iov_len = sizeof(unsigned short);
        writev(sock, iov, 2);

        recv(sock, &suc_ip, sizeof(unsigned int), MSG_WAITALL);
        recv(sock, &suc_port, sizeof(unsigned short), MSG_WAITALL);
        
        close(sock);
    } else {
        suc_ip = my_ip;
        suc_port = my_port;
    }

    return 0;
}

// función local que devuelve la IP y el puerto del nodo;
// retorna 0 si OK y -1 si error
int ring_self(unsigned int *ip, unsigned short *port) {
    if (!is_initialized()) return -1; // no está inicializada
    
    *ip = my_ip;
    *port = my_port;
    
    return 0;
}

// devuelve el PID del nodo remoto especificado o -1 si error
int ring_remote_pid(unsigned int remote_ip, unsigned short remote_port) {
    if (!is_initialized()) return -1; // no está inicializada
    
    int sock = create_socket_cln(remote_ip, remote_port);
    if (sock < 0) return -1;

    char op = 'P';
    send(sock, &op, sizeof(char), 0);

    int pid;
    recv(sock, &pid, sizeof(int), MSG_WAITALL);
    
    close(sock);
    return ntohl(pid);
}

// función local que devuelve la IP y el puerto del nodo sucesor;
// retorna 0 si OK y -1 si error
int ring_successor(unsigned int *ip, unsigned short *port) {
    if (!is_initialized()) return -1; // no está inicializada
    
    *ip = suc_ip;
    *port = suc_port;
    
    return 0;
}

// devuelve la IP y el puerto del nodo sucesor del especificado;
// retorna 0 si OK y -1 si error
int ring_remote_successor(unsigned int remote_ip, unsigned short remote_port, unsigned int *suc_ip_out, unsigned short *suc_port_out) {
    if (!is_initialized()) return -1; // no está inicializada
    
    int sock = create_socket_cln(remote_ip, remote_port);
    if (sock < 0) return -1;

    char op = 'S';
    send(sock, &op, sizeof(char), 0);

    recv(sock, suc_ip_out, sizeof(unsigned int), MSG_WAITALL);
    recv(sock, suc_port_out, sizeof(unsigned short), MSG_WAITALL);
    
    close(sock);
    return 0;
}

// devuelve la IP y el puerto del nodo sucesor del sucesor del especificado;
// retorna 0 si OK y -1 si error
int ring_remote_successor_successor(unsigned int remote_ip, unsigned short remote_port, unsigned int *suc_suc_ip, unsigned short *suc_suc_port) {
    if (!is_initialized()) return -1; // no está inicializada
    
    int sock = create_socket_cln(remote_ip, remote_port);
    if (sock < 0) return -1;

    char op = 'U';
    send(sock, &op, sizeof(char), 0);

    recv(sock, suc_suc_ip, sizeof(unsigned int), MSG_WAITALL);
    recv(sock, suc_suc_port, sizeof(unsigned short), MSG_WAITALL);
    
    close(sock);
    return 0;
}

// descarga el fichero del nodo especificado;
// retorna el tamaño del fichero si OK y -1 en caso de error
int ring_download(unsigned int remote_ip, unsigned short remote_port, const char *filename) {
    if (!is_initialized()) return -1; // no está inicializada
    
    int sock = create_socket_cln(remote_ip, remote_port);
    if (sock < 0) return -1;

    char op = 'D';
    int len = strlen(filename) + 1;
    int net_len = htonl(len);

    struct iovec iov[3];
    iov[0].iov_base = &op;
    iov[0].iov_len = sizeof(char);
    iov[1].iov_base = &net_len;
    iov[1].iov_len = sizeof(int);
    iov[2].iov_base = (void *)filename;
    iov[2].iov_len = len;
    
    writev(sock, iov, 3);

    int filesize;
    recv(sock, &filesize, sizeof(int), MSG_WAITALL);
    filesize = ntohl(filesize);

    if (filesize < 0) {
        close(sock);
        return -1;
    }

    extern char my_dir[256];
    char path[512];
    sprintf(path, "%s/%s", my_dir, filename);
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);

    if (fd < 0) {
        close(sock);
        return -1;
    }

    ftruncate(fd, filesize);
    void *map = mmap(NULL, filesize, PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        close(sock);
        return -1;
    }

    int received = 0;
    while (received < filesize) {
        int r = recv(sock, map + received, filesize - received, 0);
        if (r <= 0) break;
        received += r;
    }

    munmap(map, filesize);
    close(fd);
    close(sock);

    if (received != filesize) return -1;
    
    return filesize;
}

// busca el fichero en el anillo dando un número máximo de saltos y devolviendo
// la IP y el puerto del nodo que lo contiene;
// retorna 0 si OK y -1 si error
int ring_lookup(const char *filename, int hops, unsigned int *ip, unsigned short *port) {
    if (!is_initialized()) return -1; // no está inicializada
    
    int sock = create_socket_cln(my_ip, my_port);
    if (sock < 0) return -1;

    char op = 'L';
    int net_hops = htonl(hops);
    int len = strlen(filename) + 1;
    int net_len = htonl(len);

    struct iovec iov[4];
    iov[0].iov_base = &op;
    iov[0].iov_len = sizeof(char);
    iov[1].iov_base = &net_hops;
    iov[1].iov_len = sizeof(int);
    iov[2].iov_base = &net_len;
    iov[2].iov_len = sizeof(int);
    iov[3].iov_base = (void *)filename;
    iov[3].iov_len = len;
    
    writev(sock, iov, 4);

    int res;
    recv(sock, &res, sizeof(int), MSG_WAITALL);
    res = ntohl(res);
    
    if (res == -1) {
        close(sock);
        return -1;
    }

    recv(sock, ip, sizeof(unsigned int), MSG_WAITALL);
    recv(sock, port, sizeof(unsigned short), MSG_WAITALL);
    
    close(sock);
    return 0;
}

// busca y descarga el fichero del nodo encontrado en el anillo que lo contiene;
// retorna el tamaño del fichero si OK y -1 en caso de error;
// ESTA FUNCIÓN YA ESTA COMPLETADA
int ring_get_file(const char *filename, int hops) {
    if (!is_initialized()) return -1; // no está inicializada
    unsigned int ip, ip_local;
    unsigned short port, port_local;
    int res = ring_lookup(filename, hops, &ip, &port);
    ring_self(&ip_local, &port_local);
    // realiza la descarga si encontrado en un nodo que no es el local
    if ((res!=-1) && ((ip!=ip_local) || (port!=port_local)))
        res = ring_download(ip, port, filename);
    return res;
}

// funciones auxiliares
static int initialized;

static int initialize(void) {
    return initialized?1:(initialized=1,0);
}
static int is_initialized(void) {
    return initialized;
}
