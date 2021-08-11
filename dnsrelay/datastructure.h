#pragma once
#include "util.h"
#define CACHE_CAP ((int)10)

typedef struct __LIST_NODE ListNode;
typedef struct __HEAP_NODE HeapNode;
typedef struct __HASH_NODE HashNode;

struct __LIST_NODE {
    ListNode *prev, *next;
    HexString s;
};

ListNode listNodePool[CACHE_CAP], *freeListNodes[CACHE_CAP],
    **freeListNode = freeListNodes, *listHead = NULL, *listTail = NULL;

void ListInsert(ListNode *n) {
    if (!listHead)
        listHead = listTail = n;
    else {
        listHead->prev = n;
        n->next = listHead;
        listHead = n;
    }
}

void ListExtract(ListNode *p) {
    if (listHead == listTail)
        listHead = listTail = NULL;
    else if (p == listHead) {
        listHead = p->next;
        p->next->prev = NULL;
    } else if (p == listTail) {
        listTail = p->prev;
        p->prev->next = NULL;
    } else {
        p->prev->next = p->next;
        p->next->prev = p->prev;
    }
    p->prev = p->next = NULL;
}

struct __HASH_NODE {
    HashNode *next;
    uint64_t h;
    HexString s, v;
    ListNode *lisp;
    HeapNode *hep;
};

#define HASH_MODULE 99929
#define HASH_CAP (CACHE_CAP + 1000)
HashNode hashNodePool[HASH_CAP], *hashList[HASH_MODULE],
    *freeHashNodes[HASH_CAP], **freeHashNode = freeHashNodes;

void HashInsert(HexString s, HexString v, ListNode *lisp, HeapNode *hep) {
    uint64_t h = HexStringHash(s);
    **--freeHashNode =
        (HashNode){hashList[h % HASH_MODULE], h, s, v, lisp, hep};
    hashList[h % HASH_MODULE] = *freeHashNode;
}

void HashDelete(HexString s, ListNode **lisp, HeapNode **hep) {
    uint64_t h = HexStringHash(s);
    for (HashNode *c = hashList[h % HASH_MODULE], *p = NULL; c;
         p = c, c = c->next)
        if (c->h == h && HexStringEqual(c->s, s)) {
            if (!p)
                hashList[h % HASH_MODULE] = c->next;
            else
                p->next = c->next;  // fix
            *freeHashNode++ = c;
            free((void *)c->s.s);
            free((void *)c->v.s);
            *lisp = c->lisp;
            *hep = c->hep;
            return;
        }
    assert(0);
}

HexString HashQuery(HexString s, ListNode **lisp) {
    uint64_t h = HexStringHash(s);
    for (HashNode *c = hashList[h % HASH_MODULE]; c; c = c->next)
        if (c->h == h && HexStringEqual(c->s, s)) {
            *lisp = c->lisp;
            return c->v;
        }
    return (HexString){NULL, -1};
}

struct __HEAP_NODE {
    HeapNode *son[2], *fa;
    int dis;
    uint32_t expr;
    HashNode *hasp;
} heapNodePool[CACHE_CAP], *freeHeapNodes[CACHE_CAP],
    **freeHeapNode = freeHeapNodes, *heapRoot = NULL;
HeapNode *HeapMerge(HeapNode *a, HeapNode *b) {
    if (!a || !b)
        return a ? a : b;
    else {
        if (a->expr > b->expr) Swap(a, b);
        a->son[1] = HeapMerge(a->son[1], b);
        a->son[1]->fa = a;
        if ((a->son[0] ? a->son[0]->dis : 0) < (a->son[1] ? a->son[1]->dis : 0))
            Swap(a->son[0], a->son[1]);
        a->dis = a->son[1] ? a->son[1]->dis + 1 : 0;
        return a;
    }
}

void HeapPush(uint32_t expr, HashNode *hasp) {
    HeapNode *n = *--freeHeapNode;
    *n = (HeapNode){{NULL, NULL}, NULL, 0, expr, hasp};

    heapRoot = HeapMerge(heapRoot, n);
}

void HeapPop(HeapNode *p) {
    if (p == heapRoot)
        heapRoot = HeapMerge(p->son[0], p->son[1]);
    else {
        int d = p == p->fa->son[1];
        if ((p->fa->son[d] = HeapMerge(p->son[0], p->son[1])))
            p->fa->son[d]->fa = p->fa;
    }
    *freeHeapNode++ = p;
}
