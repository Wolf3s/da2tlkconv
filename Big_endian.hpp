#include "uint.hpp"
template <class T>
class GFF_List;

typedef GFF_List<wchar_t> String;

bool g_x360 = false; // big endian

bool g_ignoreEmptyLine = false;
bool g_addIDPrefix = false;
bool g_usingXML = false;
bool g_usingTroika = false;

namespace bit {
    u32 swapU32(u32 d) {
        if (!g_x360) {
            return d;
        }
        return (((((d) & 0x000000ff) << 24) | (((d) & 0x0000ff00) << 8) | (((d) & 0x00ff0000) >> 8) | (((d) & 0xff000000) >> 24)));
    }
    u16 swapU16(u16 d) {
        if (!g_x360) {
            return d;
        }
        u8 temp[2];
        u8* p = reinterpret_cast<u8*>(&d);
        temp[0] = p[1];
        temp[1] = p[0];
        u16* pp = reinterpret_cast<u16*>(temp);
        return *pp;
    }
    wchar_t swapU16(wchar_t d) {
        if (!g_x360) {
            return d;
        }
        u8 temp[2];
        u8* p = reinterpret_cast<u8*>(&d);
        temp[0] = p[1];
        temp[1] = p[0];
        wchar_t* pp = reinterpret_cast<wchar_t*>(temp);
        return *pp;
    }
}
namespace file 
{
    bool read(const char* file, char*& buff, size_t* size) {
        assert(file);
        assert(size);
        assert(buff == NULL);

        std::ifstream infile(file, std::ifstream::binary);
        if (infile.fail()) {
            return false;
        }

        infile.seekg(0, std::ifstream::end);
        size_t s = static_cast<size_t>(infile.tellg());
        infile.seekg(0, std::ifstream::beg);
        buff = NEW char[s];
        infile.read(buff, static_cast<std::streamsize>(s));
        infile.close();
        if (size) {
            *size = s;
        }

        return true;
    }
    bool write(const char* file, const char* buff, size_t size) {
        assert(file);
        assert(buff);

        std::ofstream outfile(file, std::ostream::binary);
        if (outfile.fail()) {
            return false;
        }
        outfile.write(buff, static_cast<std::streamsize>(size));
        outfile.close();
        return true;
    }
    bool writeApp(const char* file, const char* buff, size_t size) {
        assert(file);
        assert(buff);

        std::ofstream outfile(file, std::ostream::binary | std::ostream::app);
        if (outfile.fail()) {
            return false;
        }
        outfile.write(buff, static_cast<std::streamsize>(size));
        outfile.close();
        return true;
    }
}

enum FieldDataTypes : unsigned short {
    UINT8 = 0,
    INT8 = 1,
    UINT16 = 2,
    INT16 = 3,
    UINT32 = 4,
    INT32 = 5,
    UINT64 = 6,
    INT64 = 7,
    FLOAT32 = 8,
    FLOAT64 = 9,
    Vector3f = 10,
    Vector4f = 12,
    Quaternionf = 13,
    ECString = 14,
    Color4f = 15,
    Matrix4x4f = 16,
    TlkString = 17,
    Generic = 0xFFFF,
};

static const char* g_fieldDataType[] =
{
    "UINT8       = 0",
    "INT8        = 1",
    "UINT16      = 2",
    "INT16       = 3",
    "UINT32      = 4",
    "INT32       = 5",
    "UINT64      = 6",
    "INT64       = 7",
    "FLOAT32     = 8",
    "FLOAT64     = 9",
    "Vector3f    = 10",
    "UNKNOWN     = 11",
    "Vector4f    = 12",
    "Quaternionf = 13",
    "ECString    = 14",
    "Color4f     = 15",
    "Matrix4x4f  = 16",
    "TlkString   = 17",
};

enum FieldFlag : unsigned short {
    LIST = 0x8000,
    STRUCT = 0x4000,
    REFERENCE = 0x2000,
};