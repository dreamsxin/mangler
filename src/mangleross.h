

class ManglerOSS : public ManglerBackend {
    int             oss_fd;
public:
    virtual bool            open(int type, Glib::ustring device, int rate, int channels);
    virtual void            close(bool drain = false);
    virtual bool            write(uint8_t *sample, uint32_t length, int channels);
    virtual bool            read(uint8_t* buf, uint32_t pcm_framesize, int channels);
    virtual Glib::ustring   getAudioSubsystem(void);
    ManglerOSS();
    virtual ~ManglerOSS();

    static void            getDeviceList(std::vector<ManglerAudioDevice*>& inputDevices, std::vector<ManglerAudioDevice*>& outputDevices);
};
