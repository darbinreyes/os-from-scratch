/* Testing C lib. implementation behavior for getting a single character of
input with getchar(). */
#include <stdio.h>

/*

Is EOF defined by ASCII?
Is <RETURN> mapped to \n by ASCII? No, it is mapped to 0x0A <LINE FEED>

ASCII:

All visible characters are in the range [0x20, 0x7E]


The range of non-visible characters is [0x00, 0x1F]+0x7F.
0x7F = "Del"
In this range the characters I am familiar with are:

0x00 NULL

0x08 Backspace

0x0A Line Feed

0x0D Carriage Return

0x1B Escape

*/

int a[1024];

int main(void) {
    int c;

    // while (1) {
        // c = getchar();
        /*
            Notes behavior of getchar():
            ===

            It does indeed only return 1 character at a time.
            However during the prompt, you may enter as many as many characters as you like.
            After the first character, the remaining characters are placed in some internal buffer and fetched 1 at a time, as if you typed them in 1 at a time.
            getchar() returns if you press <RETURN> but also if you press <CTRL+D>, which I believe is EOF.
            Entering <CTRL+C> terminates the program.

            Entering <ESC> prints "^[" at the prompt. No character is returned by getchar() corresponding to <ESC>.
            Entering <F5> prints "^[[15~" at the prompt and getchar() returns "[15~". Similarly for other single purpose F keys.

            <TAB> mapped to 0x09.

            No effect at prompt: <CAPSLOCK>,<LR-SHIFT>,<LR-CTRL>,<LR-ALT/OPTION>,<BACKSPACE/DELETE>,<Fn>,<END>,<PG-UP>,<PG-DOWN>,<NUMPAD-CLEAR>,<>,<>,<>

            <RETURN>/<NUMPAD-ENTER> mapped to 0x0A.

            <HOME> mapped to "[33~<0x0A>`<0x0A><0x0A><0x0A>"


            <CUR-UP> mapped to "^[[A"/returns "[A".
            <CUR-DOWN> mapped to "^[[B"/returns "[B".
            <CUR-RIGHT> mapped to "^[[C"/returns "[C".
            <CUR-LEFT> mapped to "^[[D"/returns "[D".
        */
        // printf("char = %X = %c\n", c, c);
    // }
    for (int i = 0; i < 1024; i++)
        printf("%d\n", a[i]);

    return 0;
}