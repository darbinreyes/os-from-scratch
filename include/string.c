/*!
    @header Standard C Header
*/

/*!
    @function    memcpy

memcpy -- copy memory area
DESCRIPTION
     The memcpy() function copies n bytes from memory area src to memory area
     dst.  If dst and src overlap, behavior is undefined.  Applications in
     which dst and src might overlap should use memmove(3) instead.

RETURN VALUES
     The memcpy() function returns the original value of dst.



*/
void *memcpy(void *restrict dst, const void *restrict src, size_t n) {
    return NULL;
}


/*!
    @function    memset

DESCRIPTION
     The memset() function writes len bytes of value c (converted to an
     unsigned char) to the string b.

RETURN VALUES
     The memset() function returns its first argument.

*/
void *memset(void *b, int c, size_t len) {
    return NULL;
}