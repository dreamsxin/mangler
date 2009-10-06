/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 */

#include "mangler.h"
#include "mangleraudio.h"
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>

ManglerAudio::ManglerAudio() {
}

void
ManglerAudio::startOutputStream()
{
    uint16_t *qbuf;
    uint32_t qlen = 0;
    static pa_sample_spec ss;
    ss.format = PA_SAMPLE_S16LE;
    ss.rate = 32000;
    ss.channels = 1;
    pa_simple *s = NULL;
    int error;
    struct timeval tv;

    if (!(s = pa_simple_new(NULL, "mangler", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        return;
    }
    if (!(s = pa_simple_new(NULL, "mangler", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        return;
    }
    while ((qbuf = v3_get_soundq(&qlen)) != NULL) {
        gettimeofday(&tv, NULL); fprintf(stderr, "%d.%d: ", (int)tv.tv_sec, (int)tv.tv_usec);
        fprintf(stderr, "received %d bytes of audio data\n", qlen);
        if (pa_simple_write(s, qbuf, (size_t) qlen, &error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
            free(qbuf);
            return;
        }
        gettimeofday(&tv, NULL); fprintf(stderr, "%d.%d: ", (int)tv.tv_sec, (int)tv.tv_usec);
        fprintf(stderr, __FILE__": pa_simple_new() finished\n");
        free(qbuf);
    }
}
