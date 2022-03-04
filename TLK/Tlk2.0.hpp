/*********************************************************************************************
** Copyright 2011 hikami, aka longod                                                        **
** Copyright 2021 André Guilherme, aka Wolf3s                                               **                                                                 
**																						    **
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR			    **
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                 **
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE              **
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                   **
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,            **
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  **
** https://social.bioware.com/                                                              **
** http://www.datoolset.net/wiki/Main_Page                                                  **
** https://hnnewgamesofficial.blogspot.com/                                                 **
** https://discord.gg/yVWTAmGVuE                                                            **
**********************************************************************************************/

#include "types.hpp"

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