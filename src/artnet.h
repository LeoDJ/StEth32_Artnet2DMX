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

// bitfields are LSB first
typedef union ArtPollStatus1 {
    struct {
        uint8_t ubeaSupported : 1;  // This field is used to report wether an UEBA is supported
        uint8_t rdmCapable : 1;     // Remote Device Management capable
        uint8_t bootMode : 1;       // Denote uploaded firmware after boot
        uint8_t unimplemented : 1;      
        uint8_t portAddrAuth : 2;   // Port-Address Programming Authority (00 = unknown, 01 = front panel, 10 = network, 11 = illegal)
        uint8_t indicatorCtrl : 2;  // Front panel indicator control (00 = unknown, 01 = Locate, 10 = Mute, 11 = Normal)
    };
    uint8_t raw;
} apSt1_t;

typedef union ArtPollStatus2 {
    struct {
        uint8_t browserCfg : 1;     // Device supports configuration using web browser
        uint8_t ipFromDHCP : 1;     // Device has been allocated an IP address via DHCP (only relevant if dhcpCapable = 1)
        uint8_t dhcpCapable : 1;    // Device capable of responding to DHCP
        uint8_t artNet3Capable : 1; // Device capable of 15 bit addresses (Art-Net 3)
    };
    uint8_t raw;
} apSt2_t;

struct PortType {
    enum ArtPollPortTypeData : uint8_t {
        DMX512 = 0,
        MIDI = 1,
        Avab = 2,
        ColortranCMX = 3,
        ADB62_5 = 4,
        Art_Net = 5
    };
};

typedef union ArtPollPortType {
    struct {
        uint8_t type : 6; // see PortType
        uint8_t inputImplemented : 1;
        uint8_t outputImplemented : 1;
    };
    uint8_t raw;
} apPT_t;

typedef union ArtPollGoodInput {
    struct {
        uint8_t unused : 2;
        uint8_t dataErrors : 1;         // Data errors flag, reset via ArtAddress->Command->AcResetRxFlags
        uint8_t inpuptEnable : 1;       // Input enabled, can be set via ArtInput->Input[]
        uint8_t textPktsReceived : 1;   // Text packets received in DMX512 mode, reset via ArtAddress->Command->AcResetRxFlags
        uint8_t sipPktsReceived : 1;    // SIP packets received in DMX512 mode, reset via ArtAddress->Command->AcResetRxFlags
        uint8_t testPktsReceived : 1;   // Test packets received in DMX512 mode, reset via ArtAddress->Command->AcResetRxFlags
        uint8_t dataReceived : 1;       // Data received (not necessarily good), default timeout after 3s
    };
    uint8_t raw;
} apGI_t;

typedef union ArtPollGoodOutput {
    struct {
        uint8_t unused : 1;
        uint8_t mergeMode : 1;              // Only relevant if merging = 1 (0 = HTP, 1 = LTP), set via ArtAddress->Command
        uint8_t outputShort : 1;            // Report hardware short on output port, reset via ArtAddress->Command->AcResetRxFlags
        uint8_t merging : 1;                // Indicates wether Port is currently merging, cancel via ArtAddress->Command
        uint8_t textPktsTransmitted : 1;    // Text packets transmitted in DMX512 mode, controlled by Node
        uint8_t sipPktsTransmitted : 1;     // SIP packets transmitted in DMX512 mode, controlled by Node
        uint8_t testPktsTransmitted : 1;    // Test packets transmitted in DMX512 mode, controlled by Node
        uint8_t dataTransmitted : 1;        // Data transmitted (not necessarily changing)
    };
    uint8_t raw;
} apGO_t;

// ArtPollReply Packet from https://art-net.org.uk/structure/discovery-packets/artpollreply/
// uint16_t = low byte then high byte (little endian)
// TODO: big endian u16 will be handled wrong, fix
typedef struct __attribute__((packed)) {
    uint8_t ID[8];          // Identifies this as Art-Net
    uint16_t OpCode;        // Defines this packet type
    uint8_t IpAddress[4];   // IPv4 address of this device (most significant byte first)
    uint16_t PortNumber;    // Ethernet Port 0x1936
    be_uint16_t VersInfo;   // Firmware Revision (big endian)
    uint8_t NetSwitch;      // Bits 14-8 of the 15-bit Port-Address
    uint8_t SubSwitch;      // Bits 7-4 of the 15-bit Port-Address
    be_uint16_t Oem;        // OEM Code
    uint8_t UbeaVersion;    // UBEA version number
    apSt1_t Status1;        // Configuration bits
    uint16_t EstaMan;       // The ESTA Manufacturer code
    char ShortName[18];     // Short text name of device
    char LongName[64];      // Long text name of device
    char NodeReport[64];    // Text report of device status
    be_uint16_t NumPorts;   // Number of input or output ports (high byte is reserved)
    apPT_t PortTypes[4];    // Array defining Input and Output-Ports
    apGI_t GoodInput[4];    // Array showing Input-Port status
    apGO_t GoodOutput[4];   // Array showing Output-Port status
    uint8_t SwIn[4];        // Array contains low 4 bits of Input Port-Address
    uint8_t SwOut[4];       // Array contains low 4 bits of Output Port-Address
    uint8_t SwVideo;        // Video switch. Deprecated
    uint8_t SwMacro;        // Macro switch. Deprecated
    uint8_t SwRemote;       // Remote switch. Deprecated
    uint8_t Spare1;
    uint8_t Spare2;
    uint8_t Spare3;
    uint8_t Style;          // Style of device 
    uint8_t Mac[6];         // MAC Address of device (big endian)
    uint8_t BindIp[4];      // IP address to which device is bound (big endian)
    uint8_t BindIndex;      // Distance from BindIp
    apSt2_t Status2;        // Configuration bits
    uint8_t Filler[26];     // For expansion
} artPollReply_t;