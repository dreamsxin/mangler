#ifndef _MANGLER_ALSA_H
#define _MANGLER_ALSA_H
#ifdef HAVE_ALSA

#include <alsa/asoundlib.h>
#define ALSA_BUF 640


class ManglerAlsa : public ManglerBackend {
    snd_pcm_t       *alsa_stream;
    snd_pcm_sframes_t alsa_frames;
    int             alsa_error;

public:
    bool            open(int type, Glib::ustring device, int rate,int channels);
    void            close(bool drain = false);
    bool            write(uint8_t *sample, uint32_t length, int channels);
    bool            read(uint8_t* buf, uint32_t pcm_framesize, int channels);
    void            queue(uint32_t length, uint8_t *sample);
    void            finish(void);
    void            output(void);
    void            input(void);
    void            getDeviceList(void);
    void            playNotification(Glib::ustring name);
    virtual Glib::ustring   getAudioSubsystem(void);
    ManglerAlsa();
    static void            getDeviceList(std::vector<ManglerAudioDevice*>& inputDevices, std::vector<ManglerAudioDevice*>& outputDevices);
};

#endif
#endif
