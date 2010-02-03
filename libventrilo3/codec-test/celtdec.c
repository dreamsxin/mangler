// celtdec.c

// gcc -Wall celtdec.c -lcelt0 -o celtdec

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <celt/celt.h>

#define MAX_FRAME_SIZE  2048
#define MAX_FRAME_BYTES 1024

int
main(int argc, char **argv) {
    uint8_t bits[MAX_FRAME_BYTES];
    int     fmt  = 16;    // signed 16 bit
    int32_t rate = 44100; // samples per second
    int     chan = 2;     // mono or stereo
    int     bytes_per_packet = 60;

    int32_t frame_size = 320; // pcm samples as one frame
    void   *mode    = NULL;
    void   *decoder = NULL;
    int16_t output[MAX_FRAME_SIZE];
    int     nbRead = 0;
    int     nbDecd = 0;

    if (!(mode = celt_mode_create(rate, frame_size, NULL))) {
        fprintf(stderr, "celt_mode_create(): failed to create mode\n");
        exit(EXIT_FAILURE);
    }
    if (!(decoder = celt_decoder_create(mode, chan, NULL))) {
        fprintf(stderr, "celt_decoder_create(): failed to create decoder\n");
        exit(EXIT_FAILURE);
    }

    int ctr = 0;
    while (!feof(stdin)) {
        memset(&bits, 0, sizeof(bits));
        memset(&output, 0, sizeof(output));

        nbRead = fread((uint8_t *)&bits, 1, bytes_per_packet, stdin);
        if (!ctr) {
            fprintf(stderr, "nbRead: %i\n", nbRead);
        }
        if (!nbRead) {
            break;
        }
        if (celt_decode(decoder, (void *)&bits, bytes_per_packet, (void *)&output) != 0) {
            fprintf(stderr, "celt_decode(): error while decoding packet %i\n", ctr);
        }
        if (!ctr) {
            nbDecd = (fmt/8)*chan*frame_size;
            fprintf(stderr, "decoding %i bytes as %i bytes per frame\n", bytes_per_packet, nbDecd);
        }
        fwrite(&output, nbDecd, 1, stdout);

        ctr++;
    }

    celt_decoder_destroy(decoder);
    celt_mode_destroy(mode);

    fprintf(stderr, "done with %i packets\n", ctr);
    exit(EXIT_SUCCESS);
    return 0;
}
