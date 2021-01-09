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

    while (1) {
        c = getchar();
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

        /*
            A very helpful snippet from The UNIX Programming Environment - by K&P

            The UNIX system is unusual in its approach to representing control
            information, particularly its use of newlines(0x0A) to terminate
            lines. Many systems instead provide “records,” one per line, each of
            which contains not only your data but also a count of the number of
            characters in the line (and no newline(0x0A)) Other systems
            terminate each line with a carriage return(0x0D) and a
            newline(0x0A), because that sequence is necessary for output on most
            terminals. (The word “linefeed” is a synonym for newline(0x0A), so
            this sequence is often called “CRLF,” which is nearly
            pronounceable.)


            The UNIX system does neither there are no records, no record counts,
            and no bytes in any file that you or your programs did not put
            there.

            **A new-line is expanded into a carriage return(0x0D) and a
            newline(0x0A) when sent to a terminal**,

            but programs need only deal with the single newline character(0x0A),
            because that is all they see. For most purposes, this simple scheme
            is exactly what is wanted. When a more complicated structure is
            needed, it can easily be built on top of this; the converse,
            creating simplicity from complexity, is harder to achieve.

            Since the end of a line is marked by a newline character(0x0A), you might
            expect a file to be terminated by another special character, say \e
            for “end of file.” Looking at the output of od, though,

            **you will see no special character at the end of the file it just
            stops.**

            **Rather than using a special code, the system signifies the end of a file by
            simply saying there is no more data in the file.**

            The kernel keeps track of file lengths, so a program encounters
            end-of-file when it has processed all the bytes in a file.

            Programs retrieve the data in a file by a system call (a subroutine
            in the kernel) called read. Each time read is called, it returns the
            next part of a file ---

            **the next line(0x0A)** of text typed on the terminal, for example.

            read also says how many bytes of the file were returned, so
            end of file is assumed when a read says “zero bytes are being
            returned.” If there were any bytes left, read would have returned
            some of them. Actually, it makes sense not to represent end of file
            by a special byte value, because, as we said earlier, the meaning of
            the bytes depends on the interpretation of the file. But all files
            must end, and since all files must be accessed through read,
            returning zero is an interpretation-independent way to represent the
            end of a file without introducing a new special character.

            When a program reads from your **terminal**, each input line is
            given to the program by the kernel only when you type its
            newline(0x0A) (i.e, press RETURN).

            Therefore if you make a typing **mistake**, you can back up and
            correct it if you realize the mistake before you type newline. If
            you type newline before realizing the error, the line has been read
            by the system and you cannot correct it.

        */
        printf("char = %X = %c \n", c, c);
    }

    // for (int i = 0; i < 1024; i++)
        // printf("%d\n", a[i]);

    return 0;
}