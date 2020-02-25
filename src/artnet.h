#include <stdint.h>

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define htons(n) (n)
#define ntohs(n) (n)
#else
#define htons(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#define ntohs(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#endif

// big endian uint16_t handling
class be_uint16_t {
public:
    be_uint16_t() : be_val_(0) {
    }
    // Transparently cast from uint16_t
    be_uint16_t(const uint16_t &val) : be_val_(htons(val)) {
    }
    // Transparently cast to uint16_t
    operator uint16_t() const {
            return ntohs(be_val_);
    }
private:
    uint16_t be_val_;
} __attribute__((packed));



#define ARTNET_PORT     0x1936
#define ARTNET_HEADER   "Art-Net"
#define PROTOCOL_VER    14

typedef struct {
    char headerStr[8];
    uint16_t opcode;
    be_uint16_t protocolVersion;
    uint8_t sequence;
    uint8_t physical;
    uint16_t universe;
    be_uint16_t length;
    uint8_t data[];
} artnetPacket_t;

void initArtnet();
void loopArtnet();


struct OpCode {
    enum ArtNetOpCodes {
        // Device Discovery
        Poll = 0x2000,
        PollReply = 0x2100,
        // Device Configuration
        Address = 0x6000,
        Input = 0x7000,
        IpProg = 0xF800,
        IpProgReply = 0xF900,
        Command = 0x2400,
        // Streaming Control
        Dmx = 0x5000,
        Nzs = 0x5100,
        Sync = 0x5200,
        // RDM
        TodRequest = 0x8000,
        TodData = 0x8100,
        TodControl = 0x8200,
        Rdm = 0x8300,
        RdmSub = 0x8400,
        // Time-Keeping
        TimeCode = 0x9700,
        TimeSync = 0x9800,
        // Triggering
        Trigger = 0x9900,
        // Diagnostics
        DiagData = 0x2300,
        // File Transfer
        FirmwareMaster = 0xF200,
        FirmwareReply = 0xF300,
        Directory = 0x9A00,
        DirectoryReply = 0x9B00,
        FileTnMaster = 0xF400,
        FileFnMaster = 0xF500,
        FileFnReply = 0xF600
    };
};

        