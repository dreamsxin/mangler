#ifndef _MANGLER_PULSE_H
#define _MANGLER_PULSE_H
#ifdef HAVE_PULSE

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>


class ManglerPulse : ManglerBackend {
    pa_sample_spec  pulse_samplespec;
    pa_buffer_attr  buffer_attr;
    pa_simple       *pulse_stream;
    int             pulse_error;
public:
    bool            open(int type, Glib::ustring device, int rate, int channels);
    void            close(bool drain = false);
    bool            write(uint8_t *sample, uint32_t length, int channels);
    bool            read(uint8_t* buf, uint32_t pcm_framesize, int channels) = 0;
    void            queue(uint32_t length, uint8_t *sample);
    void            finish(void);
    void            output(void);
    void            input(void);
    void            getDeviceList(void);
    void            playNotification(Glib::ustring name);
    ManglerPulse();

    static void            getDeviceList(std::vector<ManglerAudioDevice*>& inputDevices, std::vector<ManglerAudioDevice*>& outputDevices);
};

#endif
#endif
