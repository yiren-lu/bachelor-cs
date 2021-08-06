#include <stdio.h>

int main() {
    int a = 123,                    // dec
        b = 0123U,                  // oct unsigned
        c = 0xABCL,                 // hex long,
        d = 3000000000LL;           // dec long long
    double x = +1e3,                // correct 10^(-3)
        y = .5e-3L,                 // correct 0.5*10^(-3)
        z = +1.2.3.4,               // incorrect
        w = 1A;                     // incorrect
    char str[] = "test\n\\string",  // correct,str with two
        str_[] = "test;             // incorrect, unmatched quotation
        char ch ='',                // incorrect, empty char
        ch2 = '\n';                 // correct
    for (int i = 1; i <= d || i <= c; ++i) {
        a += i;
        b = a ^ i + 1;
        /* multiple line comment

        // ********************
        */
    }


    /* unclosed multi-line comment
    return 0;
}