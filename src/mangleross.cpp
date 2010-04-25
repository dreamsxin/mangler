#include "mangler.h"
#ifdef HAVE_OSS
#include "mangleraudio.h"

#include "mangleross.h"

ManglerOSS::ManglerOSS(uint32_t rate, uint8_t channels, uint32_t pcm_framesize) { /*{{{*/
    oss_fd = -1;
    this->pcm_framesize = pcm_framesize;
} /*}}}*/

ManglerOSS::~ManglerOSS() { /*{{{*/
    if(oss_fd >= 0) {
        close();
    }
} /*}}}*/

bool
ManglerOSS::open(int type, Glib::ustring device, int rate, int channels) { /*{{{*/
    if ((oss_fd = ::open((device == "") ? "/dev/dsp" : device.c_str(), (type >= AUDIO_OUTPUT) ? O_WRONLY : O_RDONLY)) < 0) {
        fprintf(stderr, "oss: open() %s failed: %s\n", (device == "") ? "/dev/dsp" : device.c_str(), strerror(errno));
        return false;
    }
    int opt;
    opt = AFMT_S16_NE;
    if ((::ioctl(oss_fd, SNDCTL_DSP_SETFMT, &opt) < 0) || opt != AFMT_S16_NE) {
        fprintf(stderr, "oss: ioctl() SNDCTL_DSP_SETFMT failed: %s\n", strerror(errno));
        close();
        return false;
    }
    opt = channels;
    if ((::ioctl(oss_fd, SNDCTL_DSP_CHANNELS, &opt) < 0) || opt != (int)channels) {
        fprintf(stderr, "oss: ioctl() SNDCTL_DSP_CHANNELS failed: %s\n", strerror(errno));
        close();
        return false;
    }
    opt = rate;
    if ((::ioctl(oss_fd, SNDCTL_DSP_SPEED, &opt) < 0) || opt != (int)rate) {
        fprintf(stderr, "oss: ioctl() SNDCTL_DSP_SPEED failed: %s\n", strerror(errno));
        close();
        return false;
    }
    return true;
} /*}}}*/

void
ManglerOSS::close(bool drain) { /*{{{*/
    if (oss_fd >= 0) {
        ::close(oss_fd);
        oss_fd = -1;
    }
} /*}}}*/

bool
ManglerOSS::write(uint8_t *sample, uint32_t length, int channels) { /*{{{*/
    if(oss_fd < 0) {
        return false;
    }
    if (::write(oss_fd, sample, length) < 0) {
        fprintf(stderr, "oss: write() failed: %s\n", strerror(errno));
        return false;
    }
    return true;
} /*}}}*/

bool
ManglerOSS::read(uint8_t *buf) { /*{{{*/
    if(oss_fd < 0) {
        return false;
    }
    if (::read(oss_fd, buf, pcm_framesize) < 0) {
        fprintf(stderr, "oss: read() failed: %s\n", strerror(errno));
        return false;
    }
    return true;
} /*}}}*/

Glib::ustring
ManglerOSS::getAudioSubsystem(void) { /*{{{*/
    return Glib::ustring("oss");
} /*}}}*/

void
ManglerOSS::getDeviceList(std::vector<ManglerAudioDevice*>& inputDevices, std::vector<ManglerAudioDevice*>& outputDevices) { /*{{{*/
    int idx_p = 0, idx_c = 0;
#if SOUND_VERSION >= 0x040000
    bool ossv3 = false;
    int fd, dev, version = 0;
    oss_sysinfo sysinfo;
    oss_audioinfo ainfo;
    
    if ((fd = ::open("/dev/mixer", O_RDONLY)) < 0) {
        fprintf(stderr, "oss: open() /dev/mixer failed: %s\n", strerror(errno));
        ossv3 = true;
    }
    if (!ossv3 && (::ioctl(fd, OSS_GETVERSION, &version) < 0 || version < 0x040000)) {
        fprintf(stderr, "oss: ioctl() failed: version too old for device enumeration\n");
        ::close(fd);
        ossv3 = true;
    }
    if (!ossv3 && ::ioctl(fd, SNDCTL_SYSINFO, &sysinfo) < 0) {
        fprintf(stderr, "oss: ioctl() failed: %s\n", strerror(errno));
        ::close(fd);
        ossv3 = true;
    }
    if (!ossv3) {
        if (!sysinfo.numaudios) {
            fprintf(stderr, "oss: no sound cards found!\n");
            ::close(fd);
            return;
        }
        for (dev = 0; dev < sysinfo.numaudios; dev++) {
            ainfo.dev = dev;
            if (::ioctl(fd, SNDCTL_AUDIOINFO, &ainfo) < 0 || !ainfo.enabled) {
                continue;
            }
            if (ainfo.caps & PCM_CAP_OUTPUT) {
                outputDevices.push_back(
                    new ManglerAudioDevice(
                        idx_p++,
                        ainfo.devnode,
                        ainfo.name)
                    );
            }
            if (ainfo.caps & PCM_CAP_INPUT) {
                inputDevices.push_back(
                   new ManglerAudioDevice(
                        idx_c++,
                        ainfo.devnode,
                        ainfo.name)
                   );
            }
        }
        ::close(fd);
        return;
    }
    fprintf(stderr, "oss: falling back to ossv3 device listing\n");
#endif
    Glib::PatternSpec dsp("dsp*");
    Glib::Dir dir("/dev");
    Glib::ustring path;
    
    for (Glib::DirIterator iter = dir.begin(); iter != dir.end(); iter++) {
        if (dsp.match(*iter)) {
            path = "/dev/" + *iter;
            outputDevices.push_back(
               new ManglerAudioDevice(
                   idx_p++,
                   path,
                   path)
               );
            inputDevices.push_back(
               new ManglerAudioDevice(
                   idx_c++,
                   path,
                   path)
               );
        }
    }
} /*}}}*/
#endif
