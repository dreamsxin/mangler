// celtenc.c

// gcc -Wall celtenc.c -lcelt0 -o celtenc

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <celt/celt.h>

#define MAX_FRAME_SIZE  2048
#define MAX_FRAME_BYTES 1024

int
main(int argc, char **argv) {
    int16_t input[MAX_FRAME_SIZE];
    int     fmt  = 16;        // signed 16 bit
    int32_t rate = 44100;     // samples per second
    int     chan = 2;         // mono or stereo
    int32_t frame_size = 320; // pcm samples as one frame

    float   bitrate = 71.76; // kbits/sec
    int     bytes_per_packet = 48; // default is 48
    void   *mode    = NULL;
    void   *encoder = NULL;
    uint8_t bits[MAX_FRAME_BYTES];
    int     prediction = 2;
    int     complexity = 10;
    int     nbRead  = 0;
    int     nbBytes = 0;

    if (bitrate <= 0.005) {
        bitrate = (chan == 1) ? 64.0 : 128.0;
    }

    //bytes_per_packet = ((bitrate*1000*frame_size) / (rate+4)) / 8;
    bytes_per_packet = 60;
    fprintf(stderr, "bytes_per_packet: %i\n", bytes_per_packet);

    if (bytes_per_packet < 8) {
        bytes_per_packet = 8;
        fprintf(stderr, "requested bitrate (%0.3fkbit/sec) is too low. setting CELT to 8 bytes/frame.\n", bitrate);
    } else if (bytes_per_packet > MAX_FRAME_BYTES) {
        bytes_per_packet = MAX_FRAME_BYTES;
        fprintf(stderr, "requested bitrate (%0.3fkbit/sec) is too high. setting CELT to %i bytes/frame.\n", bitrate, MAX_FRAME_BYTES);
    }

    if (!(mode = celt_mode_create(rate, frame_size, NULL))) {
        fprintf(stderr, "celt_mode_create(): failed to create mode\n");
        exit(EXIT_FAILURE);
    }
    if (!(encoder = celt_encoder_create(mode, chan, NULL))) {
        fprintf(stderr, "celt_encoder_create(): failed to create encoder\n");
        exit(EXIT_FAILURE);
    }
    if (celt_encoder_ctl(encoder, CELT_SET_PREDICTION(prediction)) != CELT_OK) {
        fprintf(stderr, "celt_encoder_ctl(): prediction request failed\n");
        exit(EXIT_FAILURE);
    }
    if (celt_encoder_ctl(encoder, CELT_SET_COMPLEXITY(complexity)) != CELT_OK) {
        fprintf(stderr, "celt_encoder_ctl(): complexity 0 through 10 is only supported\n");
        exit(EXIT_FAILURE);
    }

    int ctr = 0;
    while (!feof(stdin)) {
        memset(&input, 0, sizeof(input));
        memset(&bits, 0, sizeof(bits));

        nbRead = fread((uint8_t *)&input, 1, (fmt/8)*chan*frame_size, stdin);
        if (!ctr) {
            fprintf(stderr, "nbRead: %i\n", nbRead);
        }
        if (!nbRead) {
            break;
        }
        if ((nbBytes = celt_encode(encoder, (void *)&input, NULL, (void *)&bits, bytes_per_packet)) < 0) {
            fprintf(stderr, "celt_encode(): %s\n", celt_strerror(nbBytes));
            exit(EXIT_FAILURE);
        }
        if (!ctr) {
            fprintf(stderr, "nbBytes: %i\n", nbBytes);
        }
        fwrite(&bits, nbBytes, 1, stdout);

        ctr++;
    }

    celt_encoder_destroy(encoder);
    celt_mode_destroy(mode);

    fprintf(stderr, "done with %i packets\n", ctr);
    exit(EXIT_SUCCESS);
    return 0;
}
