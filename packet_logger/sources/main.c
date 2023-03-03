#include <stdio.h>
#include <pcap/pcap.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/udp.h>

char errbuf[PCAP_ERRBUF_SIZE];

char *get_first_device()
{
    pcap_if_t *devs;
    if (pcap_findalldevs(&devs, errbuf) != 0)
    {
        fprintf(stderr, "Error - pcap_findalldevs(): %s", errbuf);
        return NULL;
    }

    if (devs == NULL)
    {
        return NULL;
    }

    char *dev = malloc(strlen(devs->name) + 1);
    strcpy(dev, devs->name);

    return dev;
}

char *get_mac_addr_str(struct ether_addr *addr)
{
    char *str = malloc(ETHER_ADDR_LEN * 3 + 1);
    sprintf(str, "%02x", addr->ether_addr_octet[0]);
    for (int i = 1; i < ETHER_ADDR_LEN; i++)
    {
        sprintf(str + strlen(str), ":%02x", addr->ether_addr_octet[i]);
    }
    return str;
}

char *get_ip_addr_str(struct in_addr *addr)
{
    char *str = malloc(INET_ADDRSTRLEN + 1);
    if (inet_ntop(AF_INET, addr, str, INET_ADDRSTRLEN + 1) == NULL)
    {
        perror("Error - inet_ntop()");
        return NULL;
    }
    return str;
}

char *get_tcp(const u_char *bytes)
{
    struct ip *ip_header = (struct ip *)(bytes + sizeof(struct ether_header));
    struct tcphdr *tcp_header = (struct tcphdr *)(bytes + sizeof(struct ether_header) + sizeof(struct ip));

    char *src_ip_addr_str = get_ip_addr_str(&ip_header->ip_src);
    struct sockaddr_in src_addr;
    memset(&src_addr, 0, sizeof(struct sockaddr_in));
    src_addr.sin_family = AF_INET;
    src_addr.sin_port = tcp_header->th_sport;
    char service[NI_MAXSERV];
    if (getnameinfo((struct sockaddr *)&src_addr, sizeof(struct sockaddr_in), NULL, 0, service, NI_MAXSERV, 0) != 0)
    {
        perror("Error - getnameinfo()");
        return NULL;
    }
    char src[INET_ADDRSTRLEN + NI_MAXSERV + 2];
    sprintf(src, "%s.%s", src_ip_addr_str, service);

    char *dst_ip_addr_str = get_ip_addr_str(&ip_header->ip_dst);
    struct sockaddr_in dst_addr;
    memset(&dst_addr, 0, sizeof(struct sockaddr_in));
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = tcp_header->th_dport;
    if (getnameinfo((struct sockaddr *)&dst_addr, sizeof(struct sockaddr_in), NULL, 0, service, NI_MAXSERV, 0) != 0)
    {
        perror("Error - getnameinfo()");
        return NULL;
    }
    char dst[INET_ADDRSTRLEN + NI_MAXSERV + 2];
    sprintf(dst, "%s.%s", dst_ip_addr_str, service);

    char *tcp = malloc(strlen(src) + strlen(dst) + 4);
    sprintf(tcp, "%s > %s", src, dst);
    return tcp;
}

char *get_udp(const u_char *bytes)
{
    struct ip *ip_header = (struct ip *)(bytes + sizeof(struct ether_header));
    struct udphdr *udp_header = (struct udphdr *)(bytes + sizeof(struct ether_header) + sizeof(struct ip));

    char *src_ip_addr_str = get_ip_addr_str(&ip_header->ip_src);
    struct sockaddr_in src_addr;
    memset(&src_addr, 0, sizeof(struct sockaddr_in));
    src_addr.sin_family = AF_INET;
    src_addr.sin_port = udp_header->uh_sport;
    char service[NI_MAXSERV];
    if (getnameinfo((struct sockaddr *)&src_addr, sizeof(struct sockaddr_in), NULL, 0, service, NI_MAXSERV, 0) != 0)
    {
        perror("Error - getnameinfo()");
        return NULL;
    }
    char src[INET_ADDRSTRLEN + NI_MAXSERV + 2];
    sprintf(src, "%s.%s", src_ip_addr_str, service);

    char *dst_ip_addr_str = get_ip_addr_str(&ip_header->ip_dst);
    struct sockaddr_in dst_addr;
    memset(&dst_addr, 0, sizeof(struct sockaddr_in));
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = udp_header->uh_dport;
    if (getnameinfo((struct sockaddr *)&dst_addr, sizeof(struct sockaddr_in), NULL, 0, service, NI_MAXSERV, 0) != 0)
    {
        perror("Error - getnameinfo()");
        return NULL;
    }
    char dst[INET_ADDRSTRLEN + NI_MAXSERV + 2];
    sprintf(dst, "%s.%s", dst_ip_addr_str, service);

    char *udp = malloc(strlen(src) + strlen(dst) + 4);
    sprintf(udp, "%s > %s", src, dst);
    return udp;
}

char *get_icmp(const u_char *bytes)
{
    struct ip *ip_header = (struct ip *)(bytes + sizeof(struct ether_header));

    char *src_ip_addr_str = get_ip_addr_str(&ip_header->ip_src);
    char *dst_ip_addr_str = get_ip_addr_str(&ip_header->ip_dst);

    char *icmp = malloc(strlen(src_ip_addr_str) + strlen(dst_ip_addr_str) + 4);
    sprintf(icmp, "%s > %s", src_ip_addr_str, dst_ip_addr_str);
    return icmp;
}

void pcap_handle(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes)
{
    pcap_dump(user, h, bytes);

    struct tm *time = localtime(&h->ts.tv_sec);
    printf("%d:%d:%d.%ld ", time->tm_hour, time->tm_min, time->tm_sec, h->ts.tv_usec);

    struct ether_header *ethernet_header = (struct ether_header *)bytes;
    char *src_mac_addr_str = get_mac_addr_str((struct ether_addr *)&ethernet_header->ether_shost);
    printf("%s > ", src_mac_addr_str);

    char *dst_mac_addr_str = get_mac_addr_str((struct ether_addr *)&ethernet_header->ether_dhost);
    printf("%s ", dst_mac_addr_str);

    struct ip *ip_header = (struct ip *)(bytes + sizeof(struct ether_header));

    if (ip_header->ip_p == IPPROTO_TCP)
    {
        printf("%s", get_tcp(bytes));
    }
    else if (ip_header->ip_p == IPPROTO_UDP)
    {
        printf("%s", get_udp(bytes));
    }
    else if (ip_header->ip_p == IPPROTO_ICMP)
    {
        printf("%s", get_icmp(bytes));
    }

    printf("\n");
}

int main(int argc, char *argv[])
{
    if (pcap_init(PCAP_CHAR_ENC_UTF_8, errbuf) != 0)
    {
        fprintf(stderr, "Error - pcap_init(): %s", errbuf);
        return -1;
    }

    char *dev = NULL;
    if (argc == 1)
    {
        dev = get_first_device();
        if (dev == NULL)
        {
            printf("No devices found!\n");
            return 0;
        }
    }
    else if (argc == 2)
    {
        dev = argv[1];
    }
    else
    {
        printf("Invalid command!");
        return 0;
    }
    
    printf("Device: %s\n", dev);

    pcap_t *cap = pcap_create(dev, errbuf);
    if (cap == NULL)
    {
        fprintf(stderr, "Error - pcap_create(): %s", errbuf);
        return -1;
    }

    if (pcap_set_timeout(cap, 500) != 0)
    {
        pcap_perror(cap, "Error - pcap_set_timeout()");
        return -1;
    }

    int activated = pcap_activate(cap);
    if (activated > 0)
    {
        pcap_perror(cap, "Warning - pcap_activate()");
    }
    else if (activated < 0)
    {
        pcap_perror(cap, "Error - pcap_activate()");
        return -1;
    }

    pcap_dumper_t *dumper = pcap_dump_open(cap, "dump.pcap");
    if (dumper == NULL)
    {
        pcap_perror(cap, "Error - pcap_dump_open()");
        return -1;
    }

    if (pcap_loop(cap, -1, pcap_handle, (u_char *)dumper) != 0)
    {
        pcap_perror(cap, "Error - pcap_loop()");
        return -1;
    }

    pcap_dump_close(dumper);
    pcap_close(cap);
}