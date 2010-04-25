#ifndef _MANGLER_PULSE_H
#define _MANGLER_PULSE_H
#ifdef HAVE_PULSE

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>


class ManglerPulse : public ManglerBackend {
    pa_sample_spec  pulse_samplespec;
    pa_buffer_attr  buffer_attr;
    pa_simple       *pulse_stream;
    int             pulse_error;
public:
    virtual bool            open(int type, Glib::ustring device, int rate, int channels);
    virtual void            close(bool drain = false);
    virtual bool            write(uint8_t *sample, uint32_t length, int channels);
    virtual bool            read(uint8_t* buf);
    virtual Glib::ustring   getAudioSubsystem(void);
    ManglerPulse(uint32_t rate, uint8_t channels, uint32_t pcm_framesize);
    virtual ~ManglerPulse();

    static void            getDeviceList(std::vector<ManglerAudioDevice*>& inputDevices, std::vector<ManglerAudioDevice*>& outputDevices);
};




#endif
#endif
