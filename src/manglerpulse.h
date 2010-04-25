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


typedef struct pa_devicelist {
    uint8_t initialized;
    char name[512];
    uint32_t index;
    char description[256];
} pa_devicelist_t;

void pa_state_cb(pa_context *c, void *userdata);
void pa_sinklist_cb(pa_context *c, const pa_sink_info *l, int eol, void *userdata);
void pa_sourcelist_cb(pa_context *c, const pa_source_info *l, int eol, void *userdata);
int pa_get_devicelist(pa_devicelist_t *input, pa_devicelist_t *output);
#endif

#endif
#endif
