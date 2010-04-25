#ifndef _MANGLER_ALSA_H
#define _MANGLER_ALSA_H
#ifdef HAVE_ALSA

#include <alsa/asoundlib.h>
#define ALSA_BUF 640


class ManglerAlsa : public ManglerBackend {
    snd_pcm_t       *alsa_stream;
    snd_pcm_sframes_t alsa_frames;
    int             alsa_error;
    uint32_t        pcm_framesize;
    uint8_t         channels;

public:
    virtual bool            open(int type, Glib::ustring device, int rate,int channels);
    virtual void            close(bool drain = false);
    virtual bool            write(uint8_t *sample, uint32_t length, int channels);
    virtual bool            read(uint8_t* buf);
    virtual Glib::ustring   getAudioSubsystem(void);
    ManglerAlsa(uint32_t rate, uint8_t channels, uint32_t pcm_framesize);
    virtual ~ManglerAlsa();
    static void            getDeviceList(std::vector<ManglerAudioDevice*>& inputDevices, std::vector<ManglerAudioDevice*>& outputDevices);
};

#endif
#endif
