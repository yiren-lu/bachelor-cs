#include "datastructure.h"
#include "util.h"

void CacheInsert(HexString s, HexString v, uint32_t expr) {
    s = HexStringCopy(s);  // all from here
    v = HexStringCopy(v);
    **--freeListNode = (ListNode){NULL, NULL, s};
    ListInsert(*freeListNode);
    HeapPush(expr, NULL);
    HashInsert(s, v, *freeListNode, *freeHeapNode);
    (*freeHeapNode)->hasp = *freeHashNode;
}

void CacheRemove(HexString s) {
    ListNode *lisp;
    HeapNode *hep;
    HashDelete(s, &lisp, &hep);
    ListExtract(lisp);
    *freeListNode++ = lisp;
    HeapPop(hep);
}

void CacheDiscard(int force) {
    uint32_t curTime = TimeStamp();
    while (heapRoot && heapRoot->expr <= curTime)
        CacheRemove(heapRoot->hasp->s);
    if (force && freeListNode == freeListNodes) CacheRemove(listTail->s);
}
