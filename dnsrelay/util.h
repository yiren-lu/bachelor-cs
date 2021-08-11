#pragma once
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef linux
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
typedef int SOCKET;
typedef struct sockaddr SOCKADDR;
#else
#include <winsock.h>
#endif

#define TimeStamp() time(0)
#define DNS_HEADER_SIZE 12
#define DNS_QUESTION_SIZE 4
#define DNS_RECORD_SIZE 10
#define ANS_HEADER_SIZE 10

FILE *logFile;
#define DEBUG(...)                     \
    {                                  \
        fprintf(stderr, __VA_ARGS__);  \
        fprintf(logFile, __VA_ARGS__); \
    }
// Alibaba DNS 223.5.5.5
#define DNS_SERVER "223.5.5.5"

typedef struct __DNS_HEADER {  // 12 bytes
    uint16_t id, mask, qdcount, ancount, nscount, arcount;
} DNSHeader;

typedef struct __DNS_QUESTION {  // 4 bytes
    // QNAME
    uint16_t qtype, qclass;
} DNSQuestion;

typedef struct __DNS_RECORD {  // 10 bytes
    // NAME
    uint16_t type, class;
    uint32_t ttl;
    uint16_t rdlen;
    // RDARA
} DNSRecord;

typedef struct __HEX_STRING {
    const uint8_t *s;
    int len;
} HexString;

typedef struct __ANS_HEADER {
    uint32_t arrive;
    uint16_t ancount, nscount, arcount;
} AnsHeader;

uint64_t HexStringHash(HexString s) {
    uint64_t h = 0;
    for (int i = 0; i < s.len; ++i) h = h * 257 + s.s[i];
    return h;
}

HexString HexStringCopy(HexString s) {
    uint8_t *p = (uint8_t *)malloc(s.len);
    memcpy(p, s.s, s.len);
    return (HexString){p, s.len};
}

int HexStringEqual(HexString a, HexString b) {
    return a.len == b.len && memcmp(a.s, b.s, a.len) == 0;
}

#define Swap(a, b)         \
    {                      \
        __typeof(a) t = a; \
        a = b;             \
        b = t;             \
    }

#define Min(a, b) ((a) < (b) ? (a) : (b))
