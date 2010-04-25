#include "mangler.h"
#include "mangleraudio.h"
#include "manglerbackend.h"
#include "manglerpulse.h"
#include "mangleralsa.h"
#include "mangleross.h"

ManglerBackend* 
ManglerBackend::getBackend(Glib::ustring audioSubsystem, uint32_t rate, uint8_t channels, uint32_t pcm_framesize) {
#ifdef HAVE_PULSE
    if(audioSubsystem == "pulse") {
        return new ManglerPulse(rate, channels, pcm_framesize);
    }
#endif
#ifdef HAVE_ALSA
    if(audioSubsystem == "alsa") {
        return new ManglerAlsa(rate, channels, pcm_framesize);
    }
#endif
#ifdef HAVE_OSS
    if(audioSubsystem == "oss") {
        return new ManglerOSS(rate, channels, pcm_framesize);
    }
#endif
    if(audioSubsystem == "openal"){
      fprintf(stderr, "no mac users\n");
      return NULL;
    }
    fprintf(stderr, "unrecognized audio subsystem \"%s\"\n", audioSubsystem.c_str());
    return NULL;
}

void
ManglerBackend::getDeviceList(Glib::ustring audioSubsystem, std::vector<ManglerAudioDevice*>& input, std::vector<ManglerAudioDevice*>& output) {
#ifdef HAVE_PULSE
    if(audioSubsystem == "pulse") {
        ManglerPulse::getDeviceList(input, output);
    }
#endif
#ifdef HAVE_ALSA
    if(audioSubsystem == "alsa") {
        ManglerAlsa::getDeviceList(input, output);
    }
#endif
#ifdef HAVE_OSS
    if(audioSubsystem == "oss") {
        ManglerOSS::getDeviceList(input, output);
    }
#endif

}

ManglerBackend::~ManglerBackend() {

}
