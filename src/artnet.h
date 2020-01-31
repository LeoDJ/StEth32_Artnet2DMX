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
#define OPCODE_ARTDMX   0x5000

typedef struct {
    char headerStr[8];
    uint16_t opcode;
    be_uint16_t protocolVersion;
    uint8_t sequence;
    uint8_t physical;
    uint16_t universe;
    be_uint16_t length;
} artnetHeader_t;

void initArtnet();
void loopArtnet();