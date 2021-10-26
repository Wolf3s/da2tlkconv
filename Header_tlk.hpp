/*
** Copyright 2011 hikami, aka longod
** Copyright 2021 André Guilherme, aka Wolf3s
** Licensed on MIT License
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
** https://social.bioware.com/
** http://www.datoolset.net/wiki/Main_Page
** https://hnnewgamesofficial.blogspot.com/
** https://discord.gg/yVWTAmGVuE
*/


#include "uint.hpp"
using namespace std;
// header
template <class T>
class GFF_List;

// win32
typedef GFF_List<wchar_t> String;

struct GFF_Header 
{
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

struct GFF_Struct 
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

struct GFF_Field 
{
    u32 Label;
    union 
    {
        u32 FieldType;
        struct 
        {
            u16 TypeID;
            u16 Flags;
        } Type;
    };
    u32 Index;
};

struct HTLK 
{
    u32 tag; // unknown 0 but tlk�̒l�Ɠ����Ӗ������͂�
    u32 dictOffset; // unknown 1
    u32 bitOffset; // unknown 2
};
struct HSTR 
{
    u32 id;
    u32 ptr; // ���̃I�t�Z�b�g��͕�����ł͂Ȃ�
};
struct HSTRChunk 
{
    HSTRChunk() : id(0xFFFFFFFF), offset(0xFFFFFFFF)/*, length( 0 ), ptr( NULL ) */ {}
    u32 id;
    u32 offset;
    //u32 length; // u32�ł������ǂ����͕s���A�o�C�g�P�ʂ���
    //u32* ptr;
    bool operator < (const HSTRChunk& a) const 
    {
        return id < a.id;
    }
};

struct HNode 
{
    HNode() : left(0xFFFFFFFF), right(0xFFFFFFFF) {
    }
    s32 left;
    s32 right;
};

struct TLKEntry 
{
    TLKEntry() : offset(0) {
    }
    TLKEntry(const TLKEntry& copy) : offset(0) 
    {
        this->id = copy.id;
        this->str = copy.str;
    }
    u32 offset;
    std::wstring id; // copy���܂��肾��
    std::wstring str;
    std::vector<u8> bit;
    //wchar_t* ptr;
    //u32 length;
};

class HuffmanNode
{
  public:
    HuffmanNode() : data(0), count(0), ID(0), left(NULL), right(NULL) {}
    explicit HuffmanNode(wchar_t d, u32 c) : ID(0), left(NULL), right(NULL) { data = d; count = c; }
    explicit HuffmanNode(HuffmanNode* l, HuffmanNode* r) : data(0xFFFF), ID(0) 
    {
        count = l->count + r->count;
        left = l;
        right = r;
    }
    virtual ~HuffmanNode() 
    {
        if (left) {
            delete left;
            left = NULL;
        }
        if (right) {
            delete right;
            right = NULL;
        }
    }

    wchar_t data;
    u32 count;
    u32 ID;

    HuffmanNode* left;
    HuffmanNode* right;

    bool operator < (const HuffmanNode& a) const 
    {
        return count < a.count;
    }
    bool operator <= (const HuffmanNode& a) const 
    {
        return count <= a.count;
    }
};

class CompHuffmanNode 
{
public:
    bool operator () (const HuffmanNode* lhs, const HuffmanNode* rhs) const 
    {
        return ((*lhs) < (*rhs));
    }
};
