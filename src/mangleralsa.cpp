#include "mangler.h"

#ifdef HAVE_ALSA
#include "mangleraudio.h"
#include "mangleralsa.h"

ManglerAlsa::ManglerAlsa() {
    alsa_stream = NULL;
}

bool
ManglerAlsa::open(int type, Glib::ustring device, int rate, int channels) {
    if ((alsa_error = snd_pcm_open(
                    &alsa_stream,
                    (device == "") ? "default" : device.c_str(),
                    (type >= AUDIO_OUTPUT) ? SND_PCM_STREAM_PLAYBACK : SND_PCM_STREAM_CAPTURE,
                    0)) < 0) {
        fprintf(stderr, "alsa: snd_pcm_open() failed: %s\n", snd_strerror(alsa_error));
        alsa_stream = NULL;
        return false;
    }
    if ((alsa_error = snd_pcm_set_params(
                    alsa_stream,                      // pcm handle
                    SND_PCM_FORMAT_S16_LE,            // format
                    SND_PCM_ACCESS_RW_INTERLEAVED,    // access
                    channels,                         // channels
                    rate,                             // rate
                    true,                             // soft_resample
                    150000)) < 0) {                   // latency in usec (0.15 sec)
        fprintf(stderr, "alsa: snd_pcm_set_params() failed: %s\n", snd_strerror(alsa_error));
        close();
        return false;
    }
    if ((alsa_error = snd_pcm_prepare(alsa_stream)) < 0) {
        fprintf(stderr, "alsa: snd_pcm_prepare() failed: %s\n", snd_strerror(alsa_error));
        close();
        return false;
    }
    if (type == AUDIO_INPUT && (alsa_error = snd_pcm_start(alsa_stream)) < 0) {
        fprintf(stderr, "alsa: snd_pcm_start() failed: %s\n", snd_strerror(alsa_error));
        close();
        return false;
    }
    return true;
}

void
ManglerAlsa::close(bool drain) {
    if (alsa_stream) {
        if (drain) {
            snd_pcm_drain(alsa_stream);
        }
        snd_pcm_close(alsa_stream);
        alsa_stream = NULL;
    }
}

bool
ManglerAlsa::write(uint8_t *sample, uint32_t length, int channels) {
    uint32_t buflen;
    uint32_t pcmlen = length;
    uint8_t *pcmptr = sample;
    while ((buflen = pcmlen >= ALSA_BUF ? ALSA_BUF : pcmlen)) {
        if ((alsa_frames = snd_pcm_writei(alsa_stream, pcmptr, buflen / (sizeof(int16_t) * channels))) < 0) {
            if (alsa_frames == -EPIPE) {
                snd_pcm_prepare(alsa_stream);
            } else if ((alsa_error = snd_pcm_recover(alsa_stream, alsa_frames, 0)) < 0) {
                fprintf(stderr, "alsa: snd_pcm_writei() failed: %s\n", snd_strerror(alsa_error));
                return false;
            }
        }
        pcmlen -= buflen;
        pcmptr += buflen;
    }
    return true;
}

bool
ManglerAlsa::read(uint8_t *buf, uint32_t pcm_framesize, int channels)
{
    if ((alsa_frames = snd_pcm_readi(alsa_stream, buf, pcm_framesize / (sizeof(int16_t) * channels))) < 0) {
        if (alsa_frames == -EPIPE) {
            snd_pcm_prepare(alsa_stream);
        } else if ((alsa_error = snd_pcm_recover(alsa_stream, alsa_frames, 0)) < 0) {
          fprintf(stderr, "alsa: snd_pcm_readi() failed: %s\n", snd_strerror(alsa_error));
          return false;
        }
    }
    return true;
}
Glib::ustring 
ManglerAlsa::getAudioSubsystem(void) {
    return Glib::ustring("alsa");
}

void
ManglerAlsa::getDeviceList(std::vector<ManglerAudioDevice*>& inputDevices, std::vector<ManglerAudioDevice*>& outputDevices) {
    snd_pcm_stream_t stream[2] = { SND_PCM_STREAM_PLAYBACK, SND_PCM_STREAM_CAPTURE };
    int ctr;
    
    for (ctr = 0; ctr < 2; ctr++) { // the rest is just copypasta, with bad code from alsa
        snd_ctl_t *handle;
        int card, err, dev, idx_p = 0, idx_c = 0;
        snd_ctl_card_info_t *info;
        snd_pcm_info_t *pcminfo;
        
        card = -1;
        snd_ctl_card_info_alloca(&info);
        snd_pcm_info_alloca(&pcminfo);
        if (snd_card_next(&card) < 0 || card < 0) {
            fputs("alsa: no sound cards found!\n", stderr);
            return;
        }
        while (card >= 0) {
            char hw[256] = "";
            snprintf(hw, 255, "hw:%i", card);
            if ((err = snd_ctl_open(&handle, hw, 0)) < 0) {
                fprintf(stderr, "alsa: control open (%i): %s\n", card, snd_strerror(err));
                if (snd_card_next(&card) < 0) {
                    fprintf(stderr, "alsa: snd_ctl_open: snd_card_next\n");
                    break;
                }
                continue;
            }
            if ((err = snd_ctl_card_info(handle, info)) < 0) {
                fprintf(stderr, "alsa: control hardware info (%i): %s\n", card, snd_strerror(err));
                snd_ctl_close(handle);
                if (snd_card_next(&card) < 0) {
                    fprintf(stderr, "alsa: snd_ctl_card_info: snd_card_next\n");
                    break;
                }
                continue;
            }
            dev = -1;
            for (;;) {
                if (snd_ctl_pcm_next_device(handle, &dev) < 0) {
                    fprintf(stderr, "alsa: snd_ctl_pcm_next_device\n");
                }
                if (dev < 0) {
                    break;
                }
                snd_pcm_info_set_device(pcminfo, dev);
                snd_pcm_info_set_subdevice(pcminfo, 0);
                snd_pcm_info_set_stream(pcminfo, stream[ctr]);
                if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                    if (err != -ENOENT) {
                        fprintf(stderr, "alsa: control digital audio info (%i): %s\n", card, snd_strerror(err));
                    }
                    continue;
                }
                char name[256] = "", desc[512] = "";
                snprintf(name, 255, "hw:%i,%i", card, dev);
                snprintf(desc, 511, "%s: %s (%s)",
                         snd_ctl_card_info_get_name(info),
                         snd_pcm_info_get_name(pcminfo),
                         name
                         );
                switch (stream[ctr]) {
                case SND_PCM_STREAM_PLAYBACK:
                    outputDevices.push_back(
                            new ManglerAudioDevice(
                                idx_p++,
                                name,
                                desc)
                            );
                    break;
                case SND_PCM_STREAM_CAPTURE:
                    inputDevices.push_back(
                            new ManglerAudioDevice(
                                idx_c++,
                                name,
                                desc)
                            );
                    break;
                }
            }
            snd_ctl_close(handle);
            if (snd_card_next(&card) < 0) {
                fprintf(stderr, "alsa: snd_card_next\n");
                break;
            }
        }
    }
}
#endif
