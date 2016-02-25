/**
    Home Net Control - Monitorizes devices connected to net and notify
    according with its type. Also monitorizes the bandwidth.
    Copyright (C) 2016  bercianor  (https://github.com/bercianor)
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
#include <Python.h>
#include <pthread.h>

#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_RAW, INET_ADDRSTRLEN
#include <netinet/ip.h>       // IP_MAXPACKET (which is 65535)
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_ARP = 0x0806, ETH_P_ALL = 0x0003
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>
#include <ifaddrs.h>

#include <errno.h>            // errno, perror()

// Allocate memory for an array of chars.
char *allocate_strmem(int len) {
    void *tmp;
    if (len <= 0) {
        //fprintf(stderr, "ERROR: Cannot allocate memory because len = %i in allocate_strmem().\n", len);
        return(0);
    }
    tmp = (char *) malloc(len * sizeof(char));
    if (tmp != NULL) {
        memset(tmp, 0, len * sizeof(char));
        return(tmp);
    } else {
        //fprintf(stderr, "ERROR: Cannot allocate memory for array allocate_strmem().\n");
        return(0);
    }
}

// Allocate memory for an array of unsigned chars.
uint8_t *allocate_ustrmem(int len) {
    void *tmp;
    if (len <= 0) {
        //fprintf(stderr, "ERROR: Cannot allocate memory because len = %i in allocate_ustrmem().\n", len);
        return(0);
    }
    tmp = (uint8_t *) malloc(len * sizeof(uint8_t));
    if (tmp != NULL) {
        memset(tmp, 0, len * sizeof(uint8_t));
        return(tmp);
    } else {
        //fprintf(stderr, "ERROR: Cannot allocate memory for array allocate_ustrmem().\n");
        return(0);
    }
}

// Define a struct for ARP header
typedef struct _arp_hdr arp_hdr;
struct _arp_hdr {
  uint16_t htype;
  uint16_t ptype;
  uint8_t hlen;
  uint8_t plen;
  uint16_t opcode;
  uint8_t sender_mac[6];
  uint8_t sender_ip[4];
  uint8_t target_mac[6];
  uint8_t target_ip[4];
};

// Define some constants.
#define ETH_HDRLEN 14      // Ethernet header length
#define IP4_HDRLEN 20      // IPv4 header length
#define ARP_HDRLEN 28      // ARP header length
#define ARPOP_REQUEST 1    // Taken from <linux/if_arp.h>
#define ARPOP_REPLY 2      // Taken from <linux/if_arp.h>

static PyObject *ArPingError;
PyObject *ans;
char *ip;

void *receive_packets(void *arg) {
    char *target;
    int sd,status,l,m;
    char *ip_str, mac_str[18], err_msg[256];
    arp_hdr *recv_arphdr;
    uint8_t *recv_ether_frame;
    if (!(
        (recv_ether_frame = allocate_ustrmem(IP_MAXPACKET)) &&
        (ip_str = allocate_strmem(INET_ADDRSTRLEN)) &&
        (target = allocate_strmem(40))
    )) pthread_exit("ERROR: Cannot allocate memory.");
    int *argu;
    argu = (int *)arg;
    int timeout = argu[0];
    // Submit request for a raw socket descriptor.
    if ((sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
        sprintf(err_msg, "socket() failed %s", strerror(errno));
        pthread_exit(err_msg);
    }
    // Listen for incoming ethernet frame from socket sd.
    // We expect an ARP ethernet frame of the form:
    //     MAC (6 bytes) + MAC (6 bytes) + ethernet type (2 bytes)
    //     + ethernet data (ARP header) (28 bytes)
    // Keep at it until we get an ARP reply.
    recv_arphdr = (arp_hdr *) (recv_ether_frame + 6 + 6 + 2);
    time_t start = time(NULL);
    while (1) {
        char ok = 1;
        while ((((((recv_ether_frame[12]) << 8) + recv_ether_frame[13]) != ETH_P_ARP) || (ntohs(recv_arphdr->opcode) != ARPOP_REPLY)) || ok) {
            if ((status = recv(sd, recv_ether_frame, IP_MAXPACKET, 0)) < 0) {
                if (errno == EINTR) {
                    memset(recv_ether_frame, 0, IP_MAXPACKET * sizeof(uint8_t));
                    continue;  // Something weird happened, but let's try again.
                } else {
                    sprintf(err_msg, "recv() failed: %s", strerror(errno));
                    pthread_exit(err_msg);
                }
            }
            if (inet_ntop(AF_INET, &recv_arphdr->target_ip, target, INET_ADDRSTRLEN) == NULL) {
                sprintf(err_msg, "inet_ntop() failed for source IP address. Error message: %s", strerror(errno));
                pthread_exit(err_msg);
            }
            ok = strcmp(target,ip);
            if (time(NULL) > start + timeout)
                break;
        }
        if (!strcmp(target,ip)) {
            if (inet_ntop(AF_INET, &recv_arphdr->sender_ip, ip_str, INET_ADDRSTRLEN) == NULL) {
                sprintf(err_msg, "inet_ntop() failed for source IP address. Error message: %s", strerror(errno));
                pthread_exit(err_msg);
            }
            int resp=0;
            PyObject *Py_ip_str = Py_BuildValue("s", ip_str);
            for (int k=0;k<PyList_Size(ans);k++)
                resp += PyObject_RichCompareBool(PyDict_GetItemString(PyList_GetItem(ans,k), "IP"), Py_ip_str, Py_EQ);
            Py_XDECREF(Py_ip_str);
            if (!resp) {
                for (l=0,m=0; l<5; l++,m=m+3) {
                    sprintf(&mac_str[m], "%02x:", recv_arphdr->sender_mac[l]);
                }
                sprintf(&mac_str[m], "%02x", recv_arphdr->sender_mac[l]);
                PyObject *ansdevice = Py_BuildValue("{s:s,s:s}", "MAC", mac_str, "IP", ip_str);
                if (PyList_Append(ans, ansdevice))
                    pthread_exit("error al introducir el dispositivo en la lista");
                Py_XDECREF(ansdevice);
            }
        }
        if (time(NULL) > start + timeout)
            break;
    }
    close (sd);
    // Free allocated memory.
    free(recv_ether_frame);
    free(ip_str);
    free(target);
    pthread_exit(NULL);
}

static PyObject* arping_arping(PyObject *self, PyObject *args) {
    const char *net;
    const int retry, timeout;
    if (!PyArg_ParseTuple(args, "sii", &net, &retry, &timeout))
        /**Raise error**/
        return(NULL);
    const int netlen = pow(2,32-atoi(&net[strlen(net)-2]));
    const uint32_t netmask = htonl(0xFFFFFFFF << (32-atoi(&net[strlen(net)-2])));
    struct addrinfo hints, *res;
    struct sockaddr_in *ipv4;
    struct sockaddr_ll device;
    struct ifreq ifr;
    struct ifaddrs *ifaddr, *ifa;
    int arguments[1], resp, status, frame_length, sd, bytes;
    uint8_t *src_mac, *dst_mac, *ether_frame;
    uint32_t net_addr, dev_addr;
    char net_str[INET_ADDRSTRLEN]="", *target, err_msg[256], tret[256], interface[IFNAMSIZ];
    pthread_t tid;
    arp_hdr arphdr;
    // Allocate memory for various arrays.
    ans = PyList_New(0);
    if (!(
        (src_mac = allocate_ustrmem(6)) && 
        (dst_mac = allocate_ustrmem(6)) && 
        (ether_frame = allocate_ustrmem(IP_MAXPACKET)) && 
        (target = allocate_strmem(INET_ADDRSTRLEN)) && 
        (ip = allocate_strmem(INET_ADDRSTRLEN))
       )) {
        while (Py_REFCNT(ans)>0)
            Py_XDECREF(ans);
        PyErr_SetString(ArPingError, "ERROR: Cannot allocate memory.");
        return(NULL);
    }
    strncpy(net_str, &net[0],1);
    for (int i=1;net[i]!='/';i++)
        strncat(net_str, &net[i],1);
    if ((status = inet_pton(AF_INET, net_str, &net_addr)) != 1) {
        while (Py_REFCNT(ans)>0)
            Py_XDECREF(ans);
        sprintf(err_msg, "inet_pton() failed for source IP address. Error message: %s", strerror(status));
        PyErr_SetString(ArPingError, err_msg);
        return(NULL);
    }
/************************************************************GET IFACE INFO***********************************************************/
    if (getifaddrs(&ifaddr) == -1) {
        while (Py_REFCNT(ans)>0)
            Py_XDECREF(ans);
        sprintf(err_msg, "getifaddrs error: %s", strerror(errno));
        PyErr_SetString(ArPingError, err_msg);
        return(NULL);
    }
    ifa = ifaddr;
    do {
        if (ifa->ifa_addr == NULL)
            continue;
        strcpy(interface, ifa->ifa_name);
        if ((status = inet_pton(AF_INET, inet_ntoa((*(struct sockaddr_in *)ifa->ifa_addr).sin_addr), &arphdr.sender_ip)) != 1) {
            while (Py_REFCNT(ans)>0)
                Py_XDECREF(ans);
            sprintf(err_msg, "inet_pton() failed for source IP address. Error message: %s", strerror(status));
            PyErr_SetString(ArPingError, err_msg);
            return(NULL);
        }
        if ((status = inet_pton(AF_INET, inet_ntoa(*(struct in_addr *)&arphdr.sender_ip), &dev_addr)) != 1) {
            while (Py_REFCNT(ans)>0)
                Py_XDECREF(ans);
            sprintf(err_msg, "inet_pton() failed for source IP address. Error message: %s", strerror(status));
            PyErr_SetString(ArPingError, err_msg);
            return(NULL);
        }
        ifa = ifa->ifa_next;
    } while (((dev_addr & netmask) != net_addr) && ifa != NULL);
    freeifaddrs(ifaddr);
    if ((dev_addr & netmask) != net_addr) {
        while (Py_REFCNT(ans)>0)
            Py_XDECREF(ans);
        sprintf(err_msg, "No Network interface on that net. %s", strerror(errno));
        PyErr_SetString(ArPingError, err_msg);
        return(NULL);
    }
    if (inet_ntop(AF_INET, &arphdr.sender_ip, ip, INET_ADDRSTRLEN) == NULL) {
        while (Py_REFCNT(ans)>0)
            Py_XDECREF(ans);
        sprintf(err_msg, "inet_ntop() failed for source IP address. Error message: %s", strerror(status));
        PyErr_SetString(ArPingError, err_msg);
    }
    
    // Submit request for a socket descriptor to look up interface.
    if ((sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        while (Py_REFCNT(ans)>0)
            Py_XDECREF(ans);
        sprintf(err_msg, "socket() failed to get socket descriptor for using ioctl() %s", strerror(errno));
        PyErr_SetString(ArPingError, err_msg);
        return(NULL);
    }
    // Use ioctl() to look up interface name and get its MAC address.
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", interface);
    if(ioctl(sd, SIOCGIFHWADDR, &ifr) < 0) {
        while (Py_REFCNT(ans)>0)
            Py_XDECREF(ans);
        sprintf(err_msg, "ioctl() failed to get source MAC address %s", strerror(errno));
        PyErr_SetString(ArPingError, err_msg);
        return(NULL);
    }
    close(sd);
    // Copy source MAC address.
    memcpy(src_mac, ifr.ifr_hwaddr.sa_data, 6 * sizeof(uint8_t));
    
    // Find interface index from interface name and store index in
    // struct sockaddr_ll device, which will be used as an argument of sendto().
    memset(&device, 0, sizeof(device));
    if ((device.sll_ifindex = if_nametoindex(interface)) == 0) {
        while (Py_REFCNT(ans)>0)
            Py_XDECREF(ans);
        sprintf(err_msg, "if_nametoindex() failed to obtain interface index %s", strerror(errno));
        PyErr_SetString(ArPingError, err_msg);
        return(NULL);
    }
    // Fill out sockaddr_ll.
    device.sll_family = AF_PACKET;
    memcpy(device.sll_addr, src_mac, 6 * sizeof(uint8_t));
    device.sll_halen = 6;
/************************************************************GET IFACE INFO***********************************************************/
    // Set destination MAC address: broadcast address
    memset(dst_mac, 0xff, 6 * sizeof(uint8_t));
/***************************************************************HEADERS***************************************************************/
    // ARP header
    // Hardware type (16 bits): 1 for ethernet
    arphdr.htype = htons(1);
    // Protocol type (16 bits): 2048 for IP
    arphdr.ptype = htons(ETH_P_IP);
    // Hardware address length (8 bits): 6 bytes for MAC address
    arphdr.hlen = 6;
    // Protocol address length (8 bits): 4 bytes for IPv4 address
    arphdr.plen = 4;
    // OpCode: 1 for ARP request
    arphdr.opcode = htons(ARPOP_REQUEST);
    // Sender hardware address (48 bits): MAC address
    memcpy(&arphdr.sender_mac, src_mac, 6 * sizeof(uint8_t));
    // Sender protocol address (32 bits)
    // See Source IPv4 address above
    // Target hardware address (48 bits): zero, since we don't know it yet.
    memset(&arphdr.target_mac, 0xff, 6 * sizeof(uint8_t));
    // Target protocol address (32 bits)
    // See getaddrinfo() resolution of target into the loop
    // Fill out ethernet frame header.
    // Ethernet frame length = ethernet header (MAC + MAC + ethernet type) + ethernet data (ARP header)
    frame_length = 6 + 6 + 2 + ARP_HDRLEN;
    // Destination and Source MAC addresses
    memcpy(ether_frame, dst_mac, 6 * sizeof(uint8_t));
    memcpy(ether_frame + 6, src_mac, 6 * sizeof(uint8_t));
    // Next is ethernet type code (ETH_P_ARP for ARP).
    // http://www.iana.org/assignments/ethernet-numbers
    ether_frame[12] = ETH_P_ARP / 256;
    ether_frame[13] = ETH_P_ARP % 256;
/***************************************************************HEADERS***************************************************************/
    arguments[0] = timeout;
    for (int intento=0;intento<retry;intento++) {
        if ((status = pthread_create(&tid, NULL, receive_packets, (void *) arguments)) != 0) {
            while (Py_REFCNT(ans)>0)
                Py_XDECREF(ans);
            sprintf(err_msg, "Error creating thread. Error message: %s", strerror(status));
            PyErr_SetString(ArPingError, err_msg);
            return(NULL);
        }
/***********************************************************SEND ARP PACKET***********************************************************/
        for (int disp=1;disp<netlen-1;disp++) {
/**************************************************************TARGET IP**************************************************************/
            dev_addr = htonl(ntohl(net_addr) + disp);
            if (inet_ntop(AF_INET, &dev_addr, target, INET_ADDRSTRLEN) != NULL) {
                resp=0;
                PyObject *Py_target = Py_BuildValue("s", target);
                for (int k=0;k<PyList_Size(ans);k++)
                    resp += PyObject_RichCompareBool(PyDict_GetItemString(PyList_GetItem(ans,k), "IP"), Py_target, Py_EQ);
                Py_XDECREF(Py_target);
                if ((!resp) && (strcmp(target,ip))) {
                    // Fill out hints for getaddrinfo().
                    memset(&hints, 0, sizeof(struct addrinfo));
                    hints.ai_family = AF_INET;
                    hints.ai_socktype = SOCK_STREAM;
                    hints.ai_flags = hints.ai_flags | AI_CANONNAME;
                    // Resolve target using getaddrinfo().
                    if ((status = getaddrinfo(target, NULL, &hints, &res)) != 0) {
                        while (Py_REFCNT(ans)>0)
                            Py_XDECREF(ans);
                        sprintf(err_msg, "getaddrinfo() failed: %s", gai_strerror(status));
                        PyErr_SetString(ArPingError, err_msg);
                        return(NULL);
                    }
                    ipv4 = (struct sockaddr_in *) res->ai_addr;
                    memcpy(&arphdr.target_ip, &ipv4->sin_addr, 4 * sizeof(uint8_t));
                    freeaddrinfo(res);
/**************************************************************TARGET IP**************************************************************/
                    // Next is ethernet frame data (ARP header).
                    // ARP header
                    memcpy(ether_frame + ETH_HDRLEN, &arphdr, ARP_HDRLEN * sizeof(uint8_t));
                    // Submit request for a raw socket descriptor.
                    if ((sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
                        while (Py_REFCNT(ans)>0)
                            Py_XDECREF(ans);
                        sprintf(err_msg, "socket() failed %s", strerror(errno));
                        PyErr_SetString(ArPingError, err_msg);
                        return(NULL);
                    }
                    // Send ethernet frame to socket.
                    if ((bytes = sendto(sd, ether_frame, frame_length, 0, (struct sockaddr *) &device, sizeof(device))) <= 0) {
                        while (Py_REFCNT(ans)>0)
                            Py_XDECREF(ans);
                        sprintf(err_msg, "sendto() failed %s", strerror(errno));
                        PyErr_SetString(ArPingError, err_msg);
                        return(NULL);
                    }
                    // Close socket descriptor.
                    close(sd);
                }
            } else {
                while (Py_REFCNT(ans)>0)
                    Py_XDECREF(ans);
                sprintf(err_msg, "inet_ntop() failed for source IP address. Error message: %s", strerror(errno));
                PyErr_SetString(ArPingError, err_msg);
                return(NULL);
            }
        }
        pthread_join(tid, (void *)tret);
        if (*tret) {
            while (Py_REFCNT(ans)>0)
                Py_XDECREF(ans);
            sprintf(err_msg, "Error in thread: %s", tret);
            PyErr_SetString(ArPingError, err_msg);
            return(NULL);
        }
/***********************************************************SEND ARP PACKET***********************************************************/
    }
    // Free allocated memory.
    free(src_mac);
    free(dst_mac);
    free(ether_frame);
    free(target);
    free(ip);
    return(ans);
}

static PyMethodDef arpingMethods[] = {
    {"arping",  arping_arping, METH_VARARGS, "Gets both mac and ip addresses from devices connected to net."},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initarping(void) {
    PyObject *m;
    m = Py_InitModule("arping", arpingMethods);
    if (m == NULL)
        return;
    ArPingError = PyErr_NewException("arping.arpingerror", NULL, NULL);
    Py_INCREF(ArPingError);
    PyModule_AddObject(m, "arpingerror", ArPingError);
}