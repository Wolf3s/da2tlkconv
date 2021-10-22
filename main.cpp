// (c) hikami, aka longod
// http://social.bioware.com/project/4206/
// http://social.bioware.com/wiki/datoolset/index.php/GFF

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#include <tchar.h>

#ifdef _DEBUG
#define NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW new
#endif


#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>

#include "hash_map.hpp"

using namespace std;


template <class T>
class GFF_List;

// win32
typedef unsigned char u8;
typedef char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef unsigned long long u64;
typedef long long s64;
typedef float f32;
typedef double f64;
typedef GFF_List<wchar_t> String;

bool g_x360 = false; // big endian

bool g_ignoreEmptyLine = false;
bool g_addIDPrefix = false;
bool g_usingXML = false;
bool g_usingTroika = false;

namespace bit {
    u32 swapU32( u32 d ) {
        if ( !g_x360 ) {
            return d;
        }
        return (((((d) & 0x000000ff) << 24) | (((d) & 0x0000ff00) <<  8) | (((d) & 0x00ff0000) >>  8) | (((d) & 0xff000000) >> 24) ));
    }
    u16 swapU16( u16 d ) {
        if ( !g_x360 ) {
            return d;
        }
        u8 temp[2];
        u8* p = reinterpret_cast<u8*>( &d );
        temp[0] = p[1];
        temp[1] = p[0];
        u16* pp = reinterpret_cast<u16*>(temp);
        return *pp;
    }
    wchar_t swapU16( wchar_t d ) {
        if ( !g_x360 ) {
            return d;
        }
        u8 temp[2];
        u8* p = reinterpret_cast<u8*>( &d );
        temp[0] = p[1];
        temp[1] = p[0];
        wchar_t* pp = reinterpret_cast<wchar_t*>(temp);
        return *pp;
    }
}

namespace file {
    bool read( const char *file, char *&buff, size_t *size ) {
        assert(file);
        assert(size);
        assert(buff == NULL);

        ifstream infile( file, ifstream::binary );
        if ( infile.fail() ) {
            return false;
        }

        infile.seekg( 0, ifstream::end );
        size_t s = static_cast< size_t >(infile.tellg());
        infile.seekg( 0, ifstream::beg );
        buff = NEW char[s];
        infile.read( buff, static_cast<streamsize>(s) );
        infile.close();
        if ( size ) {
            *size = s;
        }

        return true;
    }
    bool write( const char *file, const char *buff, size_t size ) {
        assert(file);
        assert(buff);

        ofstream outfile( file, ostream::binary );
        if ( outfile.fail() ) {
            return false;
        }
        outfile.write( buff, static_cast<streamsize>(size) );
        outfile.close();
        return true;
    }
    bool writeApp( const char *file, const char *buff, size_t size ) {
        assert(file);
        assert(buff);

        ofstream outfile( file, ostream::binary | ostream::app );
        if ( outfile.fail() ) {
            return false;
        }
        outfile.write( buff, static_cast<streamsize>(size) );
        outfile.close();
        return true;
    }
}


enum FieldDataTypes : unsigned short {
    UINT8       = 0,
    INT8        = 1,
    UINT16      = 2,
    INT16       = 3,
    UINT32      = 4,
    INT32       = 5,
    UINT64      = 6,
    INT64       = 7,
    FLOAT32     = 8,
    FLOAT64     = 9,
    Vector3f    = 10,
    Vector4f    = 12,
    Quaternionf = 13,
    ECString    = 14,
    Color4f     = 15,
    Matrix4x4f  = 16,
    TlkString   = 17,
    Generic     = 0xFFFF,
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
    REFERENCE  = 0x2000,
};


// header
struct GFF_Header {
    u32 GFFMagicNumber;
    u32 GFFVersion;
    u32 TargetPlatform;
    u32 FileType;
    u32 FileVersion;
    u32 StructCount;
    u32 DataOffset;
    //The first five fields are always in big endian and never byteswapped.
    //This keeps those fields human readable on any machine.
};

struct GFF_Struct {
    union{
        u32 StructType;
        s8 Type[ sizeof(u32) ];
    };
    u32 FieldCount;
    u32 FieldOffset;
    u32 StructSize;
};

struct GFF_Field {
    u32 Label;
    union {
        u32 FieldType;
        struct {
            u16 TypeID;
            u16 Flags;
        } Type;
    };
    u32 Index;
};

struct HTLK {
    u32 tag; // unknown 0 but tlk�̒l�Ɠ����Ӗ������͂�
    u32 dictOffset; // unknown 1
    u32 bitOffset; // unknown 2
};
struct HSTR {
    u32 id;
    u32 ptr; // ���̃I�t�Z�b�g��͕�����ł͂Ȃ�
};
struct HSTRChunk {
    HSTRChunk () : id( 0xFFFFFFFF ), offset( 0xFFFFFFFF )/*, length( 0 ), ptr( NULL ) */{}
    u32 id;
    u32 offset;
    //u32 length; // u32�ł������ǂ����͕s���A�o�C�g�P�ʂ���
    //u32* ptr;
    bool operator < ( const HSTRChunk &a ) const {
        return id < a.id;
    }
};

struct HNode {
    HNode() : left(0xFFFFFFFF), right(0xFFFFFFFF) {
    }
    s32 left;
    s32 right;
};
struct TLKEntry {
    TLKEntry() : offset( 0 ) {
    }
    TLKEntry( const TLKEntry& copy ) : offset( 0 ) {
        this->id = copy.id;
        this->str = copy.str;
    }
    u32 offset;
    wstring id; // copy���܂��肾��
    wstring str;
    std::vector<u8> bit;
    //wchar_t* ptr;
    //u32 length;
};

class HuffmanNode {
public:
    HuffmanNode() : data( 0 ), count( 0 ), ID(0), left( NULL ), right(NULL) {}
    explicit HuffmanNode( wchar_t d, u32 c ) : ID(0), left( NULL ), right(NULL)  { data = d; count = c; }
    explicit HuffmanNode( HuffmanNode* l, HuffmanNode* r ) : data( 0xFFFF ), ID(0) {
        count = l->count + r->count;
        left = l;
        right = r;
    }
    virtual ~HuffmanNode() {
        if ( left ) {
            delete left;
            left = NULL;
        }
        if ( right ) {
            delete right;
            right = NULL;
        }
    }

    wchar_t data;
    u32 count;
    u32 ID;

    HuffmanNode* left;
    HuffmanNode* right;

    bool operator < ( const HuffmanNode &a ) const {
        return count < a.count;
    }
    bool operator <= ( const HuffmanNode &a ) const {
        return count <= a.count;
    }
};
template<typename T>
bool lesser_ptr (T * lhs, T * rhs) {
    return ((*lhs) < (*rhs));
}

bool lesser_ptr (HuffmanNode * lhs, HuffmanNode * rhs) {
    return ((*lhs) < (*rhs));
}

class CompHuffmanNode {
public:
    bool operator () (const HuffmanNode * lhs, const HuffmanNode * rhs) const{
        return ((*lhs) < (*rhs));
    }
};

void traverseHuffmanTree( HuffmanNode* node, std::vector<u8>& code, stdext::hash_map<wchar_t, std::vector<u8> >& huffmanCodes ) {
    if ( node->left == node->right ) {
        huffmanCodes.insert( std::pair< wchar_t, std::vector<u8> >( node->data, code ) );
    } else {
        code.push_back( 0 );
        traverseHuffmanTree( node->left, code, huffmanCodes );
        code.pop_back();

        code.push_back( 1 );
        traverseHuffmanTree( node->right, code, huffmanCodes );
        code.pop_back();

    }
}

enum Mode {
    Mode_Compress,
    Mode_Decompress,
    Mode_None,
};

// xml
const wchar_t* xml_linefeed   = L"\r\n";
const wchar_t* xml_head       = L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>";
const wchar_t* xml_listbeg    = L"<tlkList>"; // �o�[�W��������H�ς��Ƃ��Ă��ʒu��񂭂炢���� ����Ƃ܂Ƃ��ȃp�[�T�i���ꂪ�ʓ|�j����Ȃ��Ɩʓ|
const wchar_t* xml_listend    = L"</tlkList>";
const wchar_t* xml_chunkbeg   = L"<tlkElement>";
const wchar_t* xml_chunkend   = L"</tlkElement>";
const wchar_t* xml_idbeg      = L"<tlkID>";
const wchar_t* xml_idend      = L"</tlkID>";
const wchar_t* xml_textbeg    = L"<tlkString>";
const wchar_t* xml_textend    = L"</tlkString>";
const wchar_t* xml_whitespace = L"    ";

bool parseXML( std::list<TLKEntry>& entry_list, const wchar_t* buff, u32 size  ) {
    std::wstring xml(buff); // �������ǂ��H�������������r���߂�ǂ�
    size_t pos = 0;
    size_t eol = 0;
    // find header
    {
        std::wstring::size_type index = xml.find( xml_head );
        if ( std::wstring::npos == index ) {
            // not found xml header
            return false;
        }
        pos = index + wcslen( xml_head );
    }
    // find list
    {
        std::wstring::size_type start = xml.find( xml_listbeg, pos );
        if ( std::wstring::npos == start ) {
            return false;
        }
        pos = start + wcslen( xml_listbeg );
        std::wstring::size_type end = xml.find( xml_listend, pos );
        if ( std::wstring::npos == end ) {
            return false;
        }
        eol = end;
    }
    
    size_t chunkendlen = wcslen( xml_chunkend );
    size_t idbeglen = wcslen( xml_idbeg );
    size_t textbeglen = wcslen( xml_textbeg );
    while( pos < eol ) {
        std::wstring::size_type start = xml.find( xml_chunkbeg, pos );
        if ( std::wstring::npos == start ) {
            break;
            //return false;
        }
        pos = start + wcslen( xml_listbeg );
        std::wstring::size_type end = xml.find( xml_chunkend, pos );
        if ( std::wstring::npos == end || end < pos ) {
            return false;
        }
        // find id
        TLKEntry temp;
        bool findID = false;
        {
            std::wstring::size_type s = xml.find( xml_idbeg, pos );
            if ( std::wstring::npos == s || end < s ) {
                //return false;
            } else {
                size_t beg = s + idbeglen;
                std::wstring::size_type e = xml.find( xml_idend, beg  );
                if ( std::wstring::npos == e || e < beg ) {
                    return false;
                }

                temp.id = xml.substr( beg, e - beg );
                findID = true;
            }
        }
        // find string
        bool findText= false;
        {
            std::wstring::size_type s = xml.find( xml_textbeg, pos );
            if ( std::wstring::npos == s || end < s ) {
                //return false;
            } else {
                size_t beg = s + textbeglen;
                std::wstring::size_type e = xml.find( xml_textend, beg  );
                if ( std::wstring::npos == e || e < beg ) {
                    return false;
                }
                temp.str = xml.substr( beg, e - beg );;
                findText = true;
            }
        }
        if ( findID ) {
            entry_list.push_back( temp );

        }

        pos = end + chunkendlen;
    }
    return true;
}

bool parseTroika( std::list<TLKEntry>& entry_list, const wchar_t* buff, u32 size  ) {
   
    TLKEntry temp;
    bool isID = false;
    bool isIDdone = false;
    bool isStr = false;

    for ( u32 i = 0; i < size; ++i ) {
        wchar_t c = buff[ i ];
        switch ( c )
        {
        case L'{':
            if ( !isID && !isIDdone ) {
                isID = true;
                temp.id.clear();
            } else if ( !isStr ) {
                isStr = true;
                temp.str.clear();
            } else {
                // error or ignore
            }
            break;
        case L'}':
            if ( isID ) {
                isID = false;
                isIDdone = true;
            } else if ( isStr ) {
                isStr = false;
                isIDdone = false;
                entry_list.push_back( temp );
            } else {
                // error or ignore
            }
            break;
        default:
            if ( isID ) {
                temp.id += c;
            } else if ( isStr ) {
                temp.str += c;
            }
            break;
        }
    }
    return true;
}

bool parseText( std::list<TLKEntry>& entry_list, const wchar_t* buff, u32 size  ) {
    
    bool isID = false;
    bool isStr = false;
    int cnt = 0; // return count;
    TLKEntry temp;

    for ( u32 i = 0; i < size; ++i ) {
        wchar_t c = buff[ i ];
        switch ( c ) {
                case L'{':
                    isID = true;
                    temp.id.clear();
                    break;
                case L'}':
                    isID = false;
                    isStr = true;
                    cnt = 0;
                    temp.str.clear();
                    break;
                case L'\r':
                    break;
                case L'\n':
                case L'\0':
                    if ( isStr ) {
                        if ( cnt > 0 ) {
                            // ������\r\n���]������
                            entry_list.push_back( temp ); //�R�s�[�����̂�
                            temp.id.clear();
                            temp.str.clear();
                            isStr = false;
                        }
                        ++cnt;
                    }
                    break;
                default:
                    if ( isID ) {
                        temp.id += c;
                    }
                    if ( isStr ) {
                        // \r\n�u��
                        if ( c == L'\\' && (i + 1) < size ) {
                            wchar_t next = buff[ i + 1 ];
                            if ( next == L'n' ) {
                                temp.str += L'\n';
                                ++i;
                            } else if ( next == L'r' ) {
                                temp.str += L'\r';
                                ++i;
                            } else {
                                temp.str += c;
                            }
                        } else {
                            temp.str += c;
                        }
                        // �����ŃA���t�@�x�b�g���J�E���g���������A\r\n��������Ȃ�
                    }
                    break;
        }
    }

    return true;
}

int convertTLKintoTXT( const char* input_path, const char* output_path ) {
    cout << "---- Converting TLK into TXT. --------------------------------" << endl;

    cout << "\treading input file." << endl;

    char* bin = NULL;
    size_t size = 0;

    bool isRead = file::read( input_path, bin, &size );
    if ( !isRead ) {
        // open error
        return -3;
    }

    // header chack
    GFF_Header* header = reinterpret_cast<GFF_Header*>(bin);
    {
        bool checkHeader = true;
        if ( header->GFFMagicNumber != ' FFG' ) {
            checkHeader = false;
        }
        if ( header->GFFVersion != '0.4V' ) {
            checkHeader = false;
        }
        if ( header->TargetPlatform == '  CP' ) {
        } else if ( header->TargetPlatform == '063X' ){
#if 0 // disalbe BE
            g_x360 = true;
#else
            checkHeader = false;
#endif
        } else {
            checkHeader = false;
        }

        if ( header->FileType != ' KLT' ) {
            checkHeader = false;
        }
        if ( header->FileVersion != '5.0V' ) {
            checkHeader = false;
        }
        if ( !checkHeader ) {
            // ignore file format
            if ( bin ) {
                delete[] bin;
                bin = NULL;
            }
            return -5;
        }
    }


    u32 structCount = bit::swapU32( header->StructCount );
    u32 dataOffset = bit::swapU32( header->DataOffset );

#if 0
    cout << endl;
    cout << L"StructCount: " << structCount << endl;
    cout << L"DataOffset: 0x" << hex << dataOffset << dec << endl;
    cout << endl;
#endif

    u8* struct_head = reinterpret_cast<u8*>(header + 1);
    GFF_Struct* struct_array = reinterpret_cast<GFF_Struct*>(header + 1);


    // field
#if 0
    GFF_Struct* sptr = struct_array;
    for( u32 i = 0; i < structCount; ++i ) {
        ++sptr;
    }
    GFF_Field* field_array = reinterpret_cast<GFF_Field*>(sptr);

    GFF_Field* fptr = field_array;

    GFF_Field* fp = fptr;

    for( u32 i = 0; i < structCount; ++i ) {
        GFF_Struct* sp = &(struct_array[ i ]);
        cout << "Struct: "<< i << endl;
        char type[5];
        strncpy_s( type, 5, sp->Type, 4 );
        cout << "\tType: " << type << endl;
        u32 fieldCount = bit::swapU32( sp->FieldCount );
        cout << "\tFieldCount: " << fieldCount << endl;
        u32 fieldOffset = bit::swapU32( sp->FieldOffset );
        cout << "\tFieldOffset: " << fieldOffset << endl;
        u32 structSize = bit::swapU32( sp->StructSize );
        cout << "\tStructSize: " << structSize << endl;
        cout << endl;

        GFF_Field* fp = reinterpret_cast<GFF_Field*>( bin + fieldOffset );
        for ( u32 j = 0; j < fieldCount; ++j ) {
            cout << "\tField: "<< j << endl;
            cout << "\t\tLabel: "<< bit::swapU32(fp->Label) << endl;
            GFF_Field type;
            type.FieldType = bit::swapU32(fp->FieldType);
            cout << "\t\tFieldType: "<< type.FieldType << endl;
            if ( type.Type.TypeID < 17 ) {
                cout << "\t\t\tTypeID: "<< g_fieldDataType[ type.Type.TypeID ] << endl;
            } else if ( type.Type.TypeID == 0xFFFF ) {
                cout << "\t\t\tTypeID: "<< L"Generic = 0xFFFF" << endl;
            } else {
                cout << "\t\t\tTypeID: "<< type.Type.TypeID << L" (UNKNOWN)" << endl;
            }

            cout << "\t\t\tFlags: "<< type.Type.Flags;
            if ( type.Type.Flags & LIST ) {
                cout << " (List)";
            }
            if ( type.Type.Flags & STRUCT ) {
                cout << " (Struct)";
            }
            if ( type.Type.Flags & REFERENCE ) {
                cout << " (Reference)";
            }
            cout << endl;

            cout << "\t\tIndex: "<< bit::swapU32(fp->Index) << endl;

            //fptr;
            ++fp;
        }
        cout << endl;
    }
    cout << endl;
#endif

    cout << "\tcreating tree." << endl;

    u8* raw = reinterpret_cast<u8*>( bin + dataOffset );
    //char* p = raw;
    u32* p32 = reinterpret_cast<u32*>(raw);


    HTLK* ptlk = reinterpret_cast<HTLK*>( raw );
    HTLK htlk;
    htlk.tag = bit::swapU32( ptlk->tag );
    htlk.dictOffset = bit::swapU32( ptlk->dictOffset );
    htlk.bitOffset = bit::swapU32( ptlk->bitOffset );

    // create dictionary
    u8* p0 = raw + htlk.dictOffset; // s32��list
    u32* pval0len = reinterpret_cast<u32*>( p0 );
    u32 val0len = bit::swapU32( *pval0len );
    s32* pval0 = reinterpret_cast<s32*>( p0 + 4 );

    u8* pDictStart = reinterpret_cast<u8*>(pval0);

    std::vector<HNode> nodes;
    u32 nodesize = val0len / 2;
    nodes.reserve( nodesize );
    nodes.resize( nodesize );
    for ( u32 i = 0; i < nodesize; ++i ) {
        u32 left = bit::swapU32( pval0[ i * 2 ] );
        u32 right = bit::swapU32( pval0[ i * 2 + 1 ] );
        nodes[ i ].left = left;
        nodes[ i ].right = right;
    }

    cout << "\tcreating bits." << endl;
    // create bits array
    u8* p1 = raw + htlk.bitOffset; // u32��list
    u32* pval1len = reinterpret_cast<u32*>( p1 );
    u32 val1len = bit::swapU32( *pval1len );
    u32* pval1 = reinterpret_cast<u32*>( p1 + 4 );
    u8* pDataStart = reinterpret_cast<u8*>(pval1);

    std::vector<u8> bit_array;
    bit_array.reserve( val1len * 4 * 8 ); // 1unsigned -> 4byte -> 32bit
    bit_array.resize( val1len * 4 * 8 );
    for ( u32 i = 0; i < val1len; ++i ) {
        // ���Ԃ�4�o�C�g�P�ʂŕ��בւ��Ȃ��Ƒʖ�
        u32 data = bit::swapU32( pval1[ i ] );
        u8* p = reinterpret_cast<u8*>(&data);
        for ( u32 j = 0; j < 4; ++j ) {
            u8 d = p[ j ];

            // for�Ń}�X�N���������蓮�ł�낤
            u8 bits[8]; 
            bits[ 0 ] = (d & 0x01) ? 1 : 0;
            bits[ 1 ] = (d & 0x02) ? 1 : 0;
            bits[ 2 ] = (d & 0x04) ? 1 : 0;
            bits[ 3 ] = (d & 0x08) ? 1 : 0;
            bits[ 4 ] = (d & 0x10) ? 1 : 0;
            bits[ 5 ] = (d & 0x20) ? 1 : 0;
            bits[ 6 ] = (d & 0x40) ? 1 : 0;
            bits[ 7 ] = (d & 0x80) ? 1 : 0;


            u32 index = i * 4 + j;
            index *= 8; //  to bit
            for ( u32 k = 0; k < 8; ++k ) {
                bit_array[ index + k ] = bits[ k ];
            }

        }
    }


    cout << "\tcreating ID & offset pairs." << endl;

    // create id:string bits offset
    u32* pstrlen = reinterpret_cast<u32*>(ptlk + 1);
    u32 strlen = bit::swapU32( *pstrlen );
    HSTR* pstr = reinterpret_cast<HSTR*>(pstrlen + 1);


    std::vector<HSTRChunk> hstr_array;
    hstr_array.reserve( strlen );

    u32 discardCount = 0;

    for ( u32 i = 0; i < strlen; ++i ){
        HSTR& hs = pstr[i];
        HSTRChunk chunk;
        // 0.5�Ŗ����Ȃ����Hhs.ptr == 0x0�����ɂȂ���

        // TODO:��̃e�L�X�g��e��
        // 0x0����Ƃ͌���Ȃ�
        // 0.5��0xFFFFFFFF��NULL�������Ȃ����H

#if 01 // �e���Ȃ��Ɨ�O���߂�ǂ�
        //if ( hs.id == 0xFFFFFFFF || hs.ptr == 0xFFFFFFFF || hs.ptr == 0x0 ) {
        if ( hs.id == 0xFFFFFFFF || hs.ptr == 0xFFFFFFFF ) {
            ++discardCount;
            continue;
        }
#endif
        chunk.id = bit::swapU32(hs.id);
        chunk.offset = bit::swapU32( hs.ptr );
        hstr_array.push_back( chunk );

        //cout << L"0x" << hex << bit::swapU32(hs.id)  << dec << L" (" <<  bit::swapU32(hs.id) << L") : " <<  L"0x" << hex << bit::swapU32(hs.ptr) << dec << L" (" << bit::swapU32(hs.ptr) << L")" << endl;
    }
    cout << "\t\tcount: " << hstr_array.size() << endl;
    cout << "\t\tignore null ID or null offset: " << discardCount << endl;


#if 0 // ID�\�[�g:v0.5�Ń\�[�g�ς݂ɂȂ����C������񂾂��
    std::sort( hstr_array.begin(), hstr_array.end() );
#endif

    // TODO:xml����t�H�[�}�b�g�ύX���l�����Ē��Ƀo�b�t�@�ɓ����̂ł͂Ȃ��f�[�^�\�z�݂̂ɂ���

    //cout << "\tdecompressing text." << endl;
    cout << "\tdecompressing strings." << endl;

#if 0
    std::vector<wchar_t> line_buffer; //omoi
    line_buffer.reserve( 4 * 1024 * 1024 ); // 8MB
    line_buffer.push_back( 0xFEFF ); // BOM
#endif
    std::vector<TLKEntry> entry_array;
    entry_array.reserve( hstr_array.size() );
    entry_array.resize( hstr_array.size() );

    for ( u32 i = 0; i < hstr_array.size(); ++i ) {
        HSTRChunk& chunk = hstr_array[ i ];
        s32 key = chunk.offset;
        TLKEntry& entry = entry_array[ i ];

        HNode root = nodes[ nodes.size() - 1 ]; // �t�Ȃ�т̃n�t�}���c���[
        HNode curNode = root;

        // ������UTF16�ɕϊ�
#if 0
        line_buffer.push_back( L'{' );
#endif

        const int nummax = 128;
        wchar_t number[ nummax ];
        int stored = swprintf_s( number, nummax - 1, L"%d", chunk.id );
        if ( stored  < 0 ) {
            // error length over
            return -6;
        }

        entry.id = number;
#if 0

        for( s32 i = 0; i < stored; ++i ) {
            line_buffer.push_back( number[i] ); 
        }

        line_buffer.push_back( L'}' );
        line_buffer.push_back( L'\r' );
        line_buffer.push_back( L'\n' );
#endif

        for ( s32 i = key; i < static_cast<s32>(bit_array.size()); ++i ) {
            u8 bit = bit_array[ i ];
            s32 next = 0;
            if ( bit ) {
                next = curNode.right;
            } else {
                next = curNode.left;
            }
            if( next & 0x80000000 ) {
                u32 c = 0xFFFFFFFF - next;
                wchar_t wc = c & 0xFFFF;
                //u16 wc = c & 0xFFFF;
                if ( wc != 0 ) {

                    // v0.5�ŉ��s�R�[�h��\r\n�ɂȂ��Ă�Ȃ�
                    // ��������邵�ʃt�H�[�}�b�g�ł�肽���Ƃ��낾���Axml��<>���g���Ă���̂ŃG�X�P�[�v�������ʓ|��
                    // ��b��{}�ň͂��Ă��܂��̂��������������
                    // :���s�R�[�h�̏ꍇ��daotlkeditor�p��"\n"�ɕϊ�����
                    
                    // ��������Ȃ��ăo�b�t�@�����O���ɂ�肽�����A�����񑀍�Ɏア�̂�
                    if ( g_usingTroika || g_usingXML ) {
                        entry.str += wc;
                    } else {
                        // ��text�t�H�[�}�b�g���͉��s��u������
                        if ( wc == 0x000A ) {
                            entry.str += L"\\n";
                        } else if ( wc == 0x000D ) {
                            entry.str += L"\\r";
                        } else {
                            entry.str += wc;
                        }
                    }

                    curNode = root;

                } else {
                    key = i + 1;
                    break;
                }

            } else {
                //assert( (s32)nodes.size() > next );
                curNode = nodes[ next ];
            }
        }
    }


    if ( bin ) {
        delete[] bin;
        bin = NULL;
    }

    // �����Ńo�b�t�@��
    cout << "\tformating text." << endl;
    // wstring�ɓ���Ă݂�H
    std::wstring output; // bom
    output = static_cast<wchar_t>(0xFEFF); // BOM
    u32 ignoreCount = 0;

    if ( g_usingXML ) {
        // xml format

        output += xml_head;
        output += xml_linefeed;
        output += xml_listbeg;
        output += xml_linefeed;


        for ( u32 i = 0; i < entry_array.size(); ++i ) {
            TLKEntry& entry = entry_array[ i ];

            if ( g_ignoreEmptyLine && entry.str.size() == 0 ) {
                ++ignoreCount;
                continue;
            }

            // number
            output += xml_whitespace;
            output += xml_chunkbeg;
            output += xml_linefeed;
            output += xml_whitespace;
            output += xml_whitespace;
            output += xml_idbeg;
            output += entry.id;
            output += xml_idend;
            output += xml_linefeed;

            output += xml_whitespace;
            output += xml_whitespace;
            output += xml_textbeg;
            output += entry.str;
            output += xml_textend;
            output += xml_linefeed;
            output += xml_whitespace;
            output += xml_chunkend;
            output += xml_linefeed;
        }
        
        output += xml_listend;
        output += xml_linefeed;

    } else if ( g_usingTroika ) {
        // troika format
        for ( u32 i = 0; i < entry_array.size(); ++i ) {
            TLKEntry& entry = entry_array[ i ];

            if ( g_ignoreEmptyLine && entry.str.size() == 0 ) {
                ++ignoreCount;
                continue;
            }

            // number
            output += L'{';
            output += entry.id;
            output += L'}';
            output += L"\r\n";
            output += L'{';
            output += entry.str;
            output += L'}';
            output += L"\r\n";
            output += L"\r\n";
        }

    } else {
        // dao format
        for ( u32 i = 0; i < entry_array.size(); ++i ) {
            TLKEntry& entry = entry_array[ i ];

            if ( g_ignoreEmptyLine && entry.str.size() == 0 ) {
                ++ignoreCount;
                continue;
            }

            // number
            output += L'{';
            output += entry.id;
            output += L'}';
            output += L"\r\n";
            output += entry.str;
            output += L"\r\n";
            output += L"\r\n";
        }
    }

    if ( g_ignoreEmptyLine ) {
        cout << "\t\tignore IDs: " << ignoreCount << endl;
    }


    // write
    cout << "\twriting output file." << endl;
    //bool isWrite = file::write( output_path, reinterpret_cast<const char*>(&line_buffer.front()), line_buffer.size() * sizeof(wchar_t) );
    bool isWrite = file::write( output_path, reinterpret_cast<const char*>(output.c_str()), output.length() * sizeof(wchar_t) );
    if ( !isWrite ) {
        // error
        return -4;
    }

    return 0;
}

int convertTXTintoTLK( const char* input_path, const char* output_path ) {
    cout << "---- Converting TXT into TLK. --------------------------------" << endl;

    cout << "\treading input file." << endl;

    wchar_t* bin = NULL;
    size_t size = 0;

    bool isRead = file::read( input_path, (char*&)bin, &size );
    if ( !isRead ) {
        // open error
        return -3;
    }
    u32 linesize = static_cast<u32>(size) / sizeof(wchar_t);

    cout << "\tparsing input file." << endl;

    std::list<TLKEntry> entry_list; // list better than vector

    bool ret = false;
    if ( g_usingXML ) {
        ret = parseXML( entry_list, bin, linesize );
    } else if ( g_usingTroika ) {
        ret = parseTroika( entry_list, bin, linesize );
    } else {
        ret = parseText( entry_list, bin, linesize);
    }

    if ( bin ) {
        delete[] bin;
        bin = NULL;
    }

    if ( !ret ) {
        return -7;
    }

#if 0
    // xml�Ƃ��̕����y���񂾂�
    bool isID = false;
    bool isStr = false;
    int cnt = 0; // return count;
    TLKEntry temp;

    for ( u32 i = 0; i < linesize; ++i ) {
        wchar_t c = bin[ i ];
        switch ( c ) {
                case L'{':
                    isID = true;
                    temp.id.clear();
                    break;
                case L'}':
                    isID = false;
                    isStr = true;
                    cnt = 0;
                    temp.str.clear();
                    break;
                case L'\r':
                    break;
                case L'\n':
                case L'\0':
                    if ( isStr ) {
                        if ( cnt > 0 ) {
                            // ������\r\n���]������
                            entry_list.push_back( temp ); //�R�s�[�����̂�
                            temp.id.clear();
                            temp.str.clear();
                            isStr = false;
                        }
                        ++cnt;
                    }
                    break;
                default:
                    if ( isID ) {
                        temp.id += c;
                    }
                    if ( isStr ) {
                        // \r\n�u��
                        if ( c == L'\\' && (i + 1) < linesize ) {
                            wchar_t next = bin[ i + 1 ];
                            if ( next == L'n' ) {
                                temp.str += L'\n';
                                ++i;
                            } else if ( next == L'r' ) {
                                temp.str += L'\r';
                                ++i;
                            } else {
                                temp.str += c;
                            }
                        } else {
                            temp.str += c;
                        }
                        // �����ŃA���t�@�x�b�g���J�E���g���������A\r\n��������Ȃ�
                    }
                    break;
        }
    }
#endif

    cout << "\t\tcount: " <<  entry_list.size() << endl;

    // 0.4���Ƌ�̏ꍇ��ffffff,ffffff�l�߂ďI��点�Ă������d�l���c���ł��Ă��Ȃ��̂ł�߂�
    if ( entry_list.size() == 0 ) {
        return -7;
    }


    std::list<TLKEntry>::iterator entry_end = entry_list.end();


    // add an ID prefix�͂����ł��
    if ( g_addIDPrefix ) {
        cout << "\tadding IDs." << endl;
        // ��������̍s�ɂ͕t���Ȃ�
        u32 addcount = 0;
        for ( std::list<TLKEntry>::iterator it = entry_list.begin(); it != entry_end; ++it ) {
            TLKEntry& entry = *it;
            u32 len = static_cast<u32>(entry.str.length());
            if ( len > 0 ) {
                entry.str = entry.id + L":" + entry.str;
                ++addcount;
            }
        }
        cout << "\t\tcount: " << addcount << endl;
    }

    cout << "\tcollecting characters." << endl;



    // �d��
    // �����J�E���g
    stdext::hash_map<wchar_t, u32> dictionary;

    // �������L���b�V�����g���Ēe���Ă݂�
    stdext::hash_map< std::wstring, u32 > cacheString; // string, offset

    for ( std::list<TLKEntry>::iterator it = entry_list.begin(); it != entry_end; ++it ) {
        TLKEntry& entry = *it;
        u32 len = static_cast<u32>(entry.str.length());
        if ( cacheString.count( entry.str ) > 0 ) {
        } else {
            for( u32 i = 0; i < len; ++i ) {
                // \r\n���łĂ�����u�������� // \���J�n�L�[���H����\n�Ƃ������\�L���Ȃ����Ƃ��肤��
                wchar_t c = entry.str.c_str()[ i ];
                if ( dictionary.count( c ) == 0 ) {
                    dictionary.insert( std::pair<wchar_t, u32>( c, 0 ) );
                }
                stdext::hash_map<wchar_t, u32>::iterator cn = dictionary.find( c );
                ++(*cn).second;
            }
            // ������\0���J�E���g���Ȃ��Ƒʖ�
            if ( dictionary.count( L'\0' ) == 0 ) {
                dictionary.insert( std::pair<wchar_t, u32>( L'\0', 0 ) );
            }
            stdext::hash_map<wchar_t, u32>::iterator cn = dictionary.find( L'\0' );
            ++(*cn).second;

            cacheString.insert( std::pair<std::wstring, u32>(entry.str, entry.offset) );

        }
    }

    cout << "\t\tused characters: " <<  dictionary.size() << endl;


    cout << "\tcreating tree." << endl;


    // �����c���[�̍쐬
    stdext::hash_map<wchar_t, u32>::iterator dictionary_end = dictionary.end();
#if 01
    std::list<HuffmanNode*> tree;
#else
    std::vector<HuffmanNode*> tree;
#endif
    for ( stdext::hash_map<wchar_t, u32>::iterator it = dictionary.begin(); it != dictionary_end; ++it ) {
        HuffmanNode* node = NEW HuffmanNode( (*it).first, (*it).second );
        tree.push_back( node );
    }
    while ( tree.size() > 1 ) {
#if 01
        std::stable_sort( tree.begin(), tree.end(), std::ptr_fun(&lesser_ptr) ); // �A�h���X�Ń\�[�g����Ȃ��悤��
        std::list<HuffmanNode*>::iterator first = tree.begin();
        std::list<HuffmanNode*>::iterator second = tree.begin();
#else
        // quick sort�͋t�������Ă�[�����Ȃ��Ƃ���
        // quick�g���Ă�tree�̕��т͕ς��Ȃ�����
        std::sort( tree.begin(), tree.end(),  std::ptr_fun(&lesser_ptr) ); // �A�h���X�Ń\�[�g����Ȃ��悤��
        std::vector<HuffmanNode*>::iterator first = tree.begin();
        std::vector<HuffmanNode*>::iterator second = tree.begin();
#endif
        ++second;
        HuffmanNode* parent = NEW HuffmanNode( *first, *second );
        ++second; // ��ɐi�߂Ȃ��Ɠ�Ԗڂ������Ȃ�
        tree.erase( first, second );
        tree.push_back( parent );
    };

    cout << "\tcreating bits." << endl;
    // �����ɑ�������r�b�g����Y�o����
    std::vector<u8> bitcode;
    stdext::hash_map<wchar_t, std::vector<u8> > huffmanCodes;
    bitcode.reserve( dictionary.size() * 2 );
    traverseHuffmanTree( tree.front(), bitcode, huffmanCodes ); // saiki

    cout << "\tcompressing text." << endl;
    // �����F�r�b�g������ɕ�����𕄍�������
    // �I�t�Z�b�g���o��
    // 8bit�Ȃǂ̃A���C�����g�l�߂͂���Ȃ� 4byte�l�߂���΂������炠�����Ńp�t�H�[�}���X�オ�肻���ł͂��邪

    u32 offset = 0;
    //offset = 32;

    // \0�͐�ɂƂ��ė܂��܂���������������
    stdext::hash_map<wchar_t, std::vector<u8> >::iterator nullfind = huffmanCodes.find( L'\0' );
    std::vector<u8>& nullcode =  nullfind->second;
    u32 nullsize = static_cast<u32>(nullcode .size());

    // TODO
    // ���ꕪ�̏ꍇ��offset��i�߂��Abit����ꂸ�ɁA�����̃I�t�Z�b�g���w�肵�Ă���ăT�C�Y�����炵�Ă���悤���c
    // 0�ȊO������Ă�炵�� ���̏ꍇ�����J�E���g�ɂ��e�����Ă���H�v����
    // map�ŃL���b�V�����Ă������d������release�Ȃ瑽���]�T

    // cache���Ă݂��B���ꂪ�ǂ߂�΍����Ă���悤�����A�A�h���X���傢�ɈقȂ�͎̂d�����Ȃ�
    // decompress����bits��ɕ���ł��镶����̏������L�^���Ă�������
    // TODO:�����Ȃ񂩔����ɃT�C�Y���傫���Ȃ��Ă���̂��� �c���[�̕��т�������ƈႤ�̂Ɗ֌W����̂��H
    // ����Ƃ��A���̂������̌����͂��������J�E���g�̎��_�ő��Ⴊ�������Ă���̂��낤���H
    // tlk����ǂݍ��񂾂����Ă݂Č��؂��Ȃ��Ƃ�
    
    // wstring����Ȃ��ƃA�h���X��r����đʖڂ��Ǝv�����R�s�[����������cfind��func��ύX�ł��Ȃ��̂���
    stdext::hash_map< std::wstring, u32 > cacheOffset; // string, offset

    for ( std::list<TLKEntry>::iterator it = entry_list.begin(); it != entry_end; ++it ) {
        TLKEntry& entry = *it;
        if ( cacheOffset.count( entry.str ) > 0 ) {
            stdext::hash_map<std::wstring, u32>::iterator cn = cacheOffset.find( entry.str );

            entry.bit.clear();
            entry.offset = cn->second;
        } else {
            entry.offset = offset;
            u32 len = static_cast<u32>(entry.str.length());
            for( u32 i = 0; i < len; ++i ) {
                wchar_t c = entry.str.c_str()[ i ];
                stdext::hash_map<wchar_t, std::vector<u8> >::iterator find = huffmanCodes.find( c );
                std::vector<u8>& code =  find->second;
                u32 codesize = static_cast<u32>(code.size());
                for( u32 i = 0; i < codesize; ++i ) {
                    entry.bit.push_back( code[ i ] );
                }
                offset += codesize;
            }
            // \0���t�^���Ȃ��Ƒʖ�
            for( u32 i = 0; i < nullsize; ++i ) {
                entry.bit.push_back( nullcode[ i ] );
            }
            offset += nullsize;

            cacheOffset.insert( std::pair<std::wstring, u32>(entry.str, entry.offset) );
        }
    }

    cout << "\tcreating binary." << endl;

    // create HSTRChunk & bit array
    std::vector<HSTRChunk> hstr_raw;
    hstr_raw.reserve( static_cast<u32>(entry_list.size()) );

    std::vector<u32> bits_raw; // 4byte alignment
    bits_raw.reserve( static_cast<u32>( entry_list.size()) * 1024 ); // tekitou

    //bits_raw.push_back( 0 );

    u32 bits = 0;
    u32 bitshift = 0;
    for ( std::list<TLKEntry>::iterator it = entry_list.begin(); it != entry_end; ++it ) {
        TLKEntry& entry = *it;
        HSTRChunk chunk;
        // �����ɒ���
        chunk.id = static_cast<u32>( _wtoi( entry.id.c_str() ) );
        chunk.offset = entry.offset;
        hstr_raw.push_back( chunk );

        // bit��u32�ɋl�߂Ă���
        u32 size = static_cast<u32>(entry.bit.size());
        for ( u32 i = 0; i < size; ++i ) {
            u8 bit = entry.bit[ i ];
            bits += bit << bitshift;
            ++bitshift;
            if ( bitshift % 32 == 0 ) {
                bits_raw.push_back( bits );
                bitshift = 0;
                bits = 0;
            }
        }
    }
    // 0�ł��t���������悢�H
    if ( bitshift != 0 ){
        bits_raw.push_back( bits ); // �̂���
    }

    // �c���[�̃o�C�i����
    // �E���D��

    // TODO:�Ȃ񂩎��X�c���[�̍��E���t�ɂȂ��Ă��邪�A���v���ۂ��H
    // �\�[�g�A���S���Y���̍���ɂ��͗l
    // stl��quick sort marge sort�ǂ���������ɍ���Ȃ��ӂ莩�O�֐�����

    cout << "\tserializing tree." << endl;

    std::list<HuffmanNode*> queue;
    std::list<HuffmanNode*> indices;
    u32 index = 0;
    queue.push_back( tree.front() );
    while( queue.size() > 0 ) {
        HuffmanNode* node = queue.front();
        queue.pop_front();
        if (node->left == node->right) {
            node->ID = 0xFFFFFFFF - node->data;
        } else {
            node->ID = index++;
            indices.push_front( node );
        }
        if ( node->right ) {
            queue.push_back(node->right);
        }
        if ( node->left ) {
            queue.push_back(node->left);
        }

    }
    // ������t�ɂ���΁c
    u32 dsize = static_cast<u32>(indices.size());
    std::vector<HNode> dict_raw;
    dict_raw.reserve( dsize );
    for ( std::list<HuffmanNode*>::iterator it = indices.begin(); it != indices.end(); ++it ) {
        HuffmanNode* node = (*it);
        u32 l = node->left->ID;
        u32 r = node->right->ID;
        // �����ȊO�͔��]������
        if ( (l & 0x80000000) == false ) {
            node->left->ID = (dsize - 1) - l;
        }
        if ( (r & 0x80000000) == false ) {
            node->right->ID = (dsize - 1) - r;
        }
        HNode hnode;
        hnode.left = node->left->ID;
        hnode.right = node->right->ID;
        dict_raw.push_back( hnode );
    }

    // �����o��
    {

        GFF_Header head;
        head.GFFMagicNumber = ' FFG';
        head.GFFVersion = '0.4V';
        head.TargetPlatform = '  CP';
        head.FileType = ' KLT';
        head.FileVersion = '5.0V';
        head.StructCount = 2; // HTLK, HSTR
        head.DataOffset = 0x78; // // header + struct array + field array

        GFF_Struct htlk_struct;
        htlk_struct.StructType = 'KLTH';
        htlk_struct.FieldCount = 3;
        htlk_struct.FieldOffset = sizeof(GFF_Header) + head.StructCount * sizeof(GFF_Struct) + 0; // header + struct + offset
        htlk_struct.StructSize = sizeof(u32) * htlk_struct.FieldCount;
        GFF_Field htlk_field[ 3 ]; // FieldCount
        htlk_field[ 0 ].Label = 19006;
        htlk_field[ 0 ].FieldType = 0xc0000001;
        htlk_field[ 0 ].Index = 0;
        htlk_field[ 1 ].Label = 19007;
        htlk_field[ 1 ].FieldType = 0x80000005;
        htlk_field[ 1 ].Index = 4;
        htlk_field[ 2 ].Label = 19008;
        htlk_field[ 2 ].FieldType = 0x80000004;
        htlk_field[ 2 ].Index = 8;

        GFF_Struct hstr_struct;
        hstr_struct.StructType = 'RTSH';
        hstr_struct.FieldCount = 2;
        hstr_struct.FieldOffset = sizeof(GFF_Header) + head.StructCount * sizeof(GFF_Struct) + sizeof(htlk_field); // header + struct + field
        hstr_struct.StructSize = sizeof(u32) * hstr_struct.FieldCount;

        GFF_Field hstr_field[ 2 ]; // FieldCount
        hstr_field[ 0 ].Label = 19004;
        hstr_field[ 0 ].FieldType = 0x00000004;
        hstr_field[ 0 ].Index = 0;
        hstr_field[ 1 ].Label = 19005;
        hstr_field[ 1 ].FieldType = 0x00000004;
        hstr_field[ 1 ].Index = 4;

        head.DataOffset = sizeof(GFF_Header) + head.StructCount * sizeof(GFF_Struct) + sizeof(htlk_field) + sizeof(hstr_field);

        u32 hstr_length = static_cast<u32>( hstr_raw.size() );
        u32 dict_length = static_cast<u32>( dict_raw.size() ) * (sizeof(HNode) / sizeof(u32)); // HNode�P�ʂł͂Ȃ�u32�P�ʂ̃T�C�Y
        u32 bits_length = static_cast<u32>( bits_raw.size() );

        HTLK htlk_raw;
        htlk_raw.tag = 0x0000000c; // unknown magicnumber? 12byte?
        htlk_raw.dictOffset = sizeof(htlk_raw) + sizeof(hstr_length) + hstr_length * sizeof(HSTRChunk);
        htlk_raw.bitOffset = htlk_raw.dictOffset + sizeof(dict_length) + static_cast<u32>( dict_raw.size() ) * sizeof(HNode);

        cout << "\twriting output file." << endl;

        bool isWrite = false;

        isWrite = file::write( output_path, reinterpret_cast<const char*>(&head), sizeof( head ) );
        if ( !isWrite ) { delete tree.front(); tree.clear(); return -4; }
        isWrite = file::writeApp( output_path, reinterpret_cast<const char*>(&htlk_struct), sizeof( htlk_struct ) );
        if ( !isWrite ) { delete tree.front(); tree.clear(); return -4; }
        isWrite = file::writeApp( output_path, reinterpret_cast<const char*>(&hstr_struct), sizeof( hstr_struct ) );
        if ( !isWrite ) { delete tree.front(); tree.clear(); return -4; }
        isWrite = file::writeApp( output_path, reinterpret_cast<const char*>(htlk_field), sizeof( htlk_field ) );
        if ( !isWrite ) { delete tree.front(); tree.clear(); return -4; }
        isWrite = file::writeApp( output_path, reinterpret_cast<const char*>(hstr_field), sizeof( hstr_field ) );
        if ( !isWrite ) { delete tree.front(); tree.clear(); return -4; }
        // htlk
        isWrite = file::writeApp( output_path, reinterpret_cast<const char*>(&htlk_raw), sizeof( htlk_raw ) );
        if ( !isWrite ) { delete tree.front(); tree.clear(); return -4; }
        // ID�e�[�u���T�C�Y
        isWrite = file::writeApp( output_path, reinterpret_cast<const char*>(&hstr_length), sizeof( hstr_length ) );
        if ( !isWrite ) { delete tree.front(); tree.clear(); return -4; }
        // ID�e�[�u��
        isWrite = file::writeApp( output_path, reinterpret_cast<const char*>(&hstr_raw.front()), hstr_length * sizeof(HSTRChunk) );
        if ( !isWrite ) { delete tree.front(); tree.clear(); return -4; }
        // �����T�C�Y
        isWrite = file::writeApp( output_path, reinterpret_cast<const char*>(&dict_length), sizeof( dict_length ) );
        if ( !isWrite ) { delete tree.front(); tree.clear(); return -4; }
        // �����f�[�^
        isWrite = file::writeApp( output_path, reinterpret_cast<const char*>(&dict_raw.front()), static_cast<u32>( dict_raw.size() ) * sizeof(HNode) ); // HNode�P�ʂ̃T�C�Y
        if ( !isWrite ) { delete tree.front(); tree.clear(); return -4; }
        // bit�T�C�Y
        isWrite = file::writeApp( output_path, reinterpret_cast<const char*>(&bits_length), sizeof( bits_length ) );
        if ( !isWrite ) { delete tree.front(); tree.clear(); return -4; }
        // bit�f�[�^
        isWrite = file::writeApp( output_path, reinterpret_cast<const char*>(&bits_raw.front()), bits_length * sizeof(u32) );
        if ( !isWrite ) { delete tree.front(); tree.clear(); return -4; }

    }

    delete tree.front();
    tree.clear();

    return 0;
}

void printUsage() {
    cout << "Usage:" << endl;
    cout << "da2tlkconv [option]... <input path> <output path>" << endl;
    cout << "\tConvert selection (required & exclusion):" << endl;
    cout << "\t\t-d\tConvert TLK into Text(UTF-16LE)." << endl;
    cout << "\t\t-c\tConvert Text(UTF-16LE) into TLK." << endl;
    cout << "\tFormat change:" << endl;
    cout << "\t\t-x\tusing XML format." << endl;
    cout << "\t\t-t\tusing Nesting text format." << endl;
    cout << "\tOutput control:" << endl;
    cout << "\t\t-i\tIgnore IDs that have an empty string (combine -d)." << endl;
    cout << "\t\t-a\tAdd an ID to beginning of the string (combine -c)." << endl;
    cout << endl;
}

int main( int argc, const char* argv[] ) {
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    const char *charLocale = "";
    const wchar_t *wcharLocale = L"";

    // widechar��locale�������Ȃ��o�O����
    //http://social.msdn.microsoft.com/Forums/ja-JP/vcexpressja/thread/8e5b23b4-c8e1-489a-ae45-b8c64f80df1a/
    const char* locatebale = setlocale(LC_ALL, charLocale );
    std::locale::global(std::locale( charLocale ));
    _wsetlocale(LC_ALL, wcharLocale ); // commandline�őւ�����悤�ɂ��邩�ˁH



    // �J���}��؂肪����Ȃ��悤�ɐ�������C���P�[���ɐ؂�ւ���
    std::locale loc( std::locale( charLocale ), "C",  std::locale::numeric );
    std::locale::global( loc );
    //std::locale::global( std::locale("", std::locale::ctype) );

    //std::locale jp("japanese");
    //std::cout.imbue(jp);
    //locale(locale("japanese"), "C", locale::numeric);

    cout << "Dragon Age 2 TLK Converter" << endl;
    cout << "\tAuther: Hikami" << endl;
    cout << "\tModified by Wolf3s";
    cout << "\tVersion: 0.6.1\tBuild Date: " << __DATE__ << " " << __TIME__ << endl;
    cout << endl;

    if ( argc < 4 ) {
        printUsage();
        return -1;
    }

    Mode mode = Mode_None;
    const char* input_path = NULL;
    const char* output_path = NULL;

    // �\�[�g�͂���񂪁A
    // ��s�폜�I�v�V����
    // ID�t�^�I�v�V����
    for ( int i = 1; i < argc; ++i ) {
        const char* arg = argv[ i ];
        u32 len = static_cast<u32>(strlen( arg ));
        if ( len > 1 && arg[0] == '-' ) {
            for ( u32 s = 1; s < len; ++s ) {
                switch ( arg[s] )
                {
                case 'd': // decompress
                    if ( mode != Mode_Decompress ) {
                        mode = Mode_Decompress;
                    } else {
                        // �r��
                        // mukou na switch
                        cout << "ERROR: Exclusive option. Choose from -d or -c." << endl;
                        cout << endl;
                        printUsage();
                        return -2;
                    }
                    break;
                case 'c': // compress
                    if ( mode != Mode_Compress ) {
                        mode = Mode_Compress;
                    } else {
                        // �r��
                        // mukou na switch
                        cout << "ERROR: Exclusive option. Choose from -d or -c." << endl;
                        cout << endl;
                        printUsage();
                        return -2;
                    }
                    break;
                case 'a': // id
                    g_addIDPrefix = true;
                    break;
                case 'i': // empty delete
                    g_ignoreEmptyLine = true;
                    break;
                case 'x':
                    g_usingXML = true;
                    break;
                case 't':
                    g_usingTroika = true;
                    break;
                default:
                    cout << "ERROR: Wrong option. " << arg << endl;
                    cout << endl;
                    printUsage();
                    return -2;
                }
            }
        } else {
            if ( !input_path ) {
                input_path = arg;
            } else if ( !output_path ) {
                output_path = arg;
            }
        }
    }

    // arg check
    if ( mode == Mode_None ) {
        // �w�肪�Ȃ�
        cout << "ERROR: Required option nothing. Choose from -d or -c." << endl;
        cout << endl;
        printUsage();
        return -2;
    }
    if ( !input_path ) {
        cout << "ERROR: Input path nothing." << endl;
        cout << endl;
        printUsage();
        return -2;
    }
    if ( !output_path ) {
        cout << "ERROR: Output path nothing." << endl;
        cout << endl;
        printUsage();
        return -2;
    }


    int ret = -1;
    switch ( mode ) {
        case Mode_Compress:
            ret = convertTXTintoTLK( input_path, output_path );
            cout << endl;
            break;
        case Mode_Decompress:
            ret = convertTLKintoTXT( input_path, output_path );
            cout << endl;
        default:
            break;
    }

    switch ( ret ) {
        case 0:
            cout << "Complete." << endl;
            break;
        case -3:
            cout << "ERROR: Reading failed. " << input_path << endl;
            break;
        case -4:
            cout << "ERROR: Writing failed. " << output_path << endl;
            break;
        case -5:
            cout << "ERROR: Input file is NOT TLK v0.5. " << input_path << endl;
            break;
        case -6:
            cout << "ERROR: Converting ID failed." << endl;
            break;
        case -7:
            cout << "ERROR: Parsing failed." << input_path << endl;
            break;
        default:
            cout << "ERROR: " << ret << endl;
            break;
    }

    return ret;
}