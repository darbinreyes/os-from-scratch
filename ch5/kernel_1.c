// Print a single character to via direct write to video memory.
void main(void) {
    unsigned char *video_memory = (unsigned char *) 0x000b8000;
    *video_memory = 'D';
    *(video_memory + 1) = 0x0f; // White on black.
    return;
}