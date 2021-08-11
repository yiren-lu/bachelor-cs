#include "cache.h"
#include "datastructure.h"
#include "packet.h"
#include "util.h"

void Initialize() {
    logFile = fopen("prog.log", "w");
    for (int i = 0; i < CACHE_CAP; ++i) {
        *freeListNode++ = listNodePool + i;
        *freeHeapNode++ = heapNodePool + i;
    }
    for (int i = 0; i < HASH_CAP; ++i) *freeHashNode++ = hashNodePool + i;
    FILE *fp = fopen("dnsrelay.txt", "r");
    for (char ip[1024], dom[1024]; ~fscanf(fp, "%s %s", ip, dom);) {
        ConvertDomain(dom);
        HexString hdom =
            HexStringCopy((HexString){(uint8_t *)dom, strlen(dom)});
        if (strcmp(ip, "0.0.0.0") == 0)
            HashInsert(hdom, (HexString){NULL, 0}, NULL, NULL);
        else {
            int alen = GenAnswer(ip);
            HexString hip = HexStringCopy((HexString){(uint8_t *)ip, alen});
            HashInsert(hdom, hip, NULL, NULL);
        }
    }
    fclose(fp);
    for (int i = 0; i < (1 << ID_BITS); ++i) *freeQueID++ = i;
#ifndef linux
    WSADATA wsa;
    assert(WSAStartup(MAKEWORD(2, 2), &wsa) == 0);
#endif
    struct sockaddr_in localAddr;
    localSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    localAddr.sin_port = htons(53);
#ifndef linux
    u_long mode = 1;
    ioctlsocket(localSocket, FIONBIO, &mode);
#else
    fcntl(localSocket, F_SETFL, O_NONBLOCK);
#endif

    if (bind(localSocket, (SOCKADDR *)&localAddr, sizeof(SOCKADDR)) != 0) {
#ifdef linux
        fprintf(stderr, "errno is: %s\n", strerror(errno));
#endif
    }
    DNSAddr.sin_family = AF_INET;
    DNSAddr.sin_addr.s_addr = inet_addr(DNS_SERVER);
    DNSAddr.sin_port = htons(53);
}

void Round() {
    struct sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);
    int len = recvfrom(localSocket, (char *)buffer, sizeof(buffer), 0,
                       (SOCKADDR *)&senderAddr, &senderAddrSize);
    if (len != -1) {
        DEBUG("%u %d\n", TimeStamp(), len);
        for (int i = 0; i < len; ++i) DEBUG("%02hx ", buffer[i]);
        DEBUG("\n");

        DNSHeader *h = (DNSHeader *)buffer;
        if (h->qdcount == htons(0x1)) {
            uint16_t real = ntohs(h->id);
            uint32_t tstamp = TimeStamp();
            uint32_t ttl = Depacket(buffer + 12, tstamp, ntohs(h->ancount),
                                    ntohs(h->nscount), ntohs(h->arcount));
            HexString qs = (HexString){qsBuffer, qslen},
                      as = (HexString){asBuffer, aslen};
            if (senderAddr.sin_addr.s_addr ==
                DNSAddr.sin_addr.s_addr) {  // response from upper server
                uint16_t cid = clientID[real];
                h->id = htons(cid);

                DEBUG("response from upper DNS server\n");
                for (int i = 0; i < len; ++i) DEBUG("%02hx ", buffer[i]);
                DEBUG("\n");

                sendto(localSocket, (char *)buffer, len, 0,
                       (SOCKADDR *)&clientAddr[real], sizeof(clientAddr[real]));
                *freeQueID++ = real;
                CacheDiscard(1);
                assert(freeListNode != freeListNodes);
                CacheInsert(qs, as, tstamp + ttl);
            } else {  // query from lower server
                CacheDiscard(0);
                ListNode *lisp;
                HexString rs = HashQuery(qs, &lisp);
                if (rs.len >= 0) {
                    int blen = GenResponse(buffer, rs);
                    ListExtract(lisp);
                    ListInsert(lisp);

                    DEBUG("send cached answer\n");
                    for (int i = 0; i < blen; ++i) DEBUG("%02hx ", buffer[i]);
                    DEBUG("\n");

                    sendto(localSocket, (char *)buffer, blen, 0,
                           (SOCKADDR *)&senderAddr, sizeof(senderAddr));
                } else {
                    if (freeQueID != freeQueIDs) {
                        uint16_t idtoUpper = *--freeQueID;
                        clientAddr[idtoUpper] = senderAddr;
                        clientID[idtoUpper] = real;
                        h->id = htons(idtoUpper);

                        DEBUG("send query to upper DNS server\n");
                        for (int i = 0; i < len; ++i)
                            DEBUG("%02hx ", buffer[i]);
                        DEBUG("\n");

                        sendto(localSocket, (char *)buffer, len, 0,
                               (SOCKADDR *)&DNSAddr, sizeof(DNSAddr));
                    } else
                        DEBUG("%u full client poll\n", TimeStamp());
                }
            }
        } else
            DEBUG("# trash on port 53, from %u\n", senderAddr.sin_addr.s_addr);
    }
}

int main() {
    for (Initialize();; Round())
        ;
    return 0;
}
