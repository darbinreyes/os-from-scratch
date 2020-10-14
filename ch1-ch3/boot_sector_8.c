/*

; Scratch work.

; Let's consider printing a single nibble. We must convert this nibble to ASCII. If the nibble is [0-9] then
char = nibble + ‘0’
; otherwise if the nibble is [A-F] == [10-15]
nibble = nibble - 10 ; or possibly nibble = nibble + -10
char = nibble + ‘a’

;;;
; ‘0x0000'
;  0    5 ; character offset
;;;

; Getting 1 nibble at a time from dx
nibble = dx & 0x000f
; we can use dx if we pusha/popa.
dx = dx >> 4

;;;

*/

static char s[] = "0x0000";

void print_nibble(unsigned char n, int i) {
    unsigned char c;
    if (n >= 0 && n < 10)
        c = n + 'a';
    else if (n >= 10 && n < 16)
        c = n - 10 + 'a';
    else
        return; // n out of range

    // putchar(c);
    s[i] = c;
}

void print_hex(unsigned short d) {
    unsigned short t;
    int i = 5;

    while (d != 0) {
        t = d & 0x000f;
        print_nibble((char) t, i);
        d = d >> 4;
        i = i - 1;
    }
}