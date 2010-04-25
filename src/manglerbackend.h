#ifndef _MANGLER_BACKEND_H
#define _MANGLER_BACKEND_H

//circular dependencies in C(++) are fun!
class ManglerAudioDevice;

class ManglerBackend {
public:
    virtual bool            open(int type, Glib::ustring device, int rate, int channels) = 0;
    virtual void            close(bool drain = false) = 0;
    

    virtual bool            write(uint8_t* sample, uint32_t length, int channels) = 0;
    virtual bool            read(uint8_t* buf, uint32_t pcm_framesize, int channels) = 0;
    virtual Glib::ustring   getAudioSubsystem(void) = 0;
    static ManglerBackend* getBackend(Glib::ustring audioSubsystem); 
    static void            getDeviceList(Glib::ustring audioSubsystem, std::vector<ManglerAudioDevice*>& input, std::vector<ManglerAudioDevice*>& output);
    virtual ~ManglerBackend();
};
#endif
