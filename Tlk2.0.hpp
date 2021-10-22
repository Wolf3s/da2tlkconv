#include "uint.hpp"

class GFFv4_0
{
public:
    int Extractlk2_0(const char* input_path, const char* output_path);

    u32 GFFMagicNumber;
    u32 GFFVersion;
    u32 TargetPlatform;
    u32 FileType;
    u32 FileVersion;
    u32 StructCount;
    u32 DataOffset;
};

struct GFF4_0Struct
{
    union
    {
        u32 StructType;
        s8 Type[sizeof(u32)];
    };
    u32 FieldCount;
    u32 FieldOffset;
    u32 StructSize;
};