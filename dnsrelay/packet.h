#include "util.h"
#define BUF_SIZE (1 << 20)
uint8_t buffer[BUF_SIZE];
SOCKET localSocket;

void DecreaseTTL(uint8_t *p, int rc, uint32_t d) {
    while (rc--) {
        p += p[0] & 0xc0 ? 2 : strlen((char *)p) + 1;
        DNSRecord *r = (DNSRecord *)p;
        DEBUG("ttl=%d d=%d\n", ntohl(r->ttl), d);
        if (ntohl(r->ttl) > d)
            r->ttl = htonl(ntohl(r->ttl) - d);
        else {
            DEBUG("shouldn't be!!!\n");
            r->ttl = htonl(0);
        }
        p += DNS_RECORD_SIZE + ntohs(r->rdlen);
    }
}
int GenResponse(uint8_t *buffer, HexString rs) {
    // only modifies answer
    DNSHeader *header = (DNSHeader *)buffer;
    // header->id stay still
    uint16_t mask = ntohs(header->mask);
    mask &= ~((1 << 4) - 1);  // reply code 0
    mask &= ~(1 << 7);        // recursion unavailable
    mask &= ~(1 << 10);       // not authoritive
    mask |= (1 << 15);        // response message
    int len = DNS_HEADER_SIZE;
    buffer += DNS_HEADER_SIZE;
    len += strlen((char *)buffer) + 1 + DNS_QUESTION_SIZE;
    buffer += len - DNS_HEADER_SIZE;
    if (!rs.s) {
        mask |= 3;
        header->ancount = header->nscount = header->arcount = 0;
    } else {
        AnsHeader *ah = (AnsHeader *)rs.s;
        header->ancount = htons(ah->ancount);
        header->nscount = htons(ah->nscount);
        header->arcount = htons(ah->arcount);
        len += rs.len - ANS_HEADER_SIZE;
        memcpy(buffer, rs.s + ANS_HEADER_SIZE, rs.len - ANS_HEADER_SIZE);
        DecreaseTTL(buffer, ah->ancount + ah->nscount + ah->arcount,
                    TimeStamp() - ah->arrive);
    }
    header->mask = htons(mask);
    return len;
}

#define ID_BITS 16
struct sockaddr_in DNSAddr, clientAddr[1 << ID_BITS];
int freeQueIDs[1 << ID_BITS], *freeQueID = freeQueIDs, clientID[1 << ID_BITS];

uint8_t qsBuffer[BUF_SIZE], asBuffer[BUF_SIZE];
int qslen, aslen;

void ReadRecord(uint8_t *p, int *len, uint32_t *ttl) {
    *len = p[0] & 0xc0 ? 2 : strlen((char *)p) + 1;
    p += *len;
    *ttl = ntohl(((DNSRecord *)p)->ttl);
    *len += DNS_RECORD_SIZE + ntohs(((DNSRecord *)p)->rdlen);
}

int ReadRecords(uint8_t *buffer, int rc, uint32_t *ttl) {
    *ttl = -1;
    int len = 0;
    while (rc--) {
        int clen;
        uint32_t cttl;
        ReadRecord(buffer, &clen, &cttl);
        *ttl = Min(*ttl, cttl);
        buffer += clen;
        len += clen;
    }
    return len;
}

uint32_t Depacket(uint8_t *buffer, uint32_t tstamp, uint16_t ancount,
                  uint16_t nscount, uint16_t arcount) {
    memset(qsBuffer, 0, sizeof(qsBuffer));
    int l = strlen((char *)buffer);
    *((uint16_t *)qsBuffer) = *((uint16_t *)(buffer + l + 1));
    memcpy(qsBuffer + 2, buffer, l);
    qslen = l + 2;
    buffer += l + 1 + DNS_QUESTION_SIZE;
    memset(asBuffer, 0, sizeof(asBuffer));
    uint32_t ttl;
    aslen = ReadRecords(buffer, ancount + nscount + arcount, &ttl);
    AnsHeader *ah = (AnsHeader *)asBuffer;
    ah->arrive = tstamp;
    ah->ancount = ancount;
    ah->nscount = nscount;
    ah->arcount = arcount;
    memcpy(asBuffer + ANS_HEADER_SIZE, buffer, aslen);
    aslen += ANS_HEADER_SIZE;
    return ttl;
}

void ConvertDomain(char *buffer) {
    uint8_t cnt = 0;
    int len = strlen(buffer);
    for (int i = len - 1; i >= 0; --i) {
        if (buffer[i] == '.') {
            buffer[i + 3] = cnt;
            cnt = 0;
        } else {
            buffer[i + 3] = buffer[i];
            cnt++;
        }
    }
    buffer[2] = cnt;
    *((uint16_t *)buffer) = htons(0x1);
}

int GenAnswer(char *buffer) {
    uint32_t ip = inet_addr(buffer);
    uint16_t *cnts = (uint16_t *)buffer;
    cnts[0] = 1;
    cnts[1] = cnts[2] = 0;
    int len = 6;
    buffer += len;
    uint8_t *rp = (uint8_t *)buffer;
    *((uint16_t *)rp) = htons(0xc00c);
    *((DNSRecord *)(rp + 2)) =
        (DNSRecord){htons(0x1), htons(0x1), htonl(2 * 24 * 60), htons(0x4)};
    *((uint32_t *)rp + 2 + DNS_RECORD_SIZE) = ip;
    len += 16;
    return len;
}
