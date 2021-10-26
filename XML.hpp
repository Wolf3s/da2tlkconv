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

#define _CRTDBG_MAP_ALLOC
#ifdef _DEBUG
#define NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW new
#endif
#include <iostream>

class ParseXML
{
	bool UTF16(const wchar_t* buff, u32 size);
	bool UTF8(const wchar_t* buff, u32 size);

	struct UTF16
	{
		const wchar_t* xml16_linefeed = L"\r\n";
		const wchar_t* xml16_head = L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>";
		const wchar_t* xml16_listbeg = L"<tlkList>"; // �o�[�W��������H�ς��Ƃ��Ă��ʒu��񂭂炢���� ����Ƃ܂Ƃ��ȃp�[�T�i���ꂪ�ʓ|�j����Ȃ��Ɩʓ|
		const wchar_t* xml16_listend = L"</tlkList>";
		const wchar_t* xml16_chunkbeg = L"<tlkElement>";
		const wchar_t* xml16_chunkend = L"</tlkElement>";
		const wchar_t* xml16_idbeg = L"<tlkID>";
		const wchar_t* xml16_idend = L"</tlkID>";
		const wchar_t* xml16_textbeg = L"<tlkString>";
		const wchar_t* xml16_textend = L"</tlkString>";
		const wchar_t* xml16_whitespace = L"    ";
	};

	struct UTF8
	{
		const wchar_t* xml8_linefeed = L"\r\n";
		const wchar_t* xml8_head = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
		const wchar_t* xml8_listbeg = L"<tlkList>"; // �o�[�W��������H�ς��Ƃ��Ă��ʒu��񂭂炢���� ����Ƃ܂Ƃ��ȃp�[�T�i���ꂪ�ʓ|�j����Ȃ��Ɩʓ|
		const wchar_t* xml8_listend = L"</tlkList>";
		const wchar_t* xml8_chunkbeg = L"<tlkElement>";
		const wchar_t* xml8_chunkend = L"</tlkElement>";
		const wchar_t* xml8_idbeg = L"<tlkID>";
		const wchar_t* xml8_idend = L"</tlkID>";
		const wchar_t* xml8_textbeg = L"<tlkString>";
		const wchar_t* xml8_textend = L"</tlkString>";
		const wchar_t* xml8_whitespace = L"    ";
	};
};

bool g_usingXML8 = false;
bool g_usingXML16 = false;

//LEGAÇY
const wchar_t* xml_linefeed = L"\r\n";
const wchar_t* xml_head = L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>";
const wchar_t* xml_listbeg = L"<tlkList>"; // �o�[�W��������H�ς��Ƃ��Ă��ʒu��񂭂炢���� ����Ƃ܂Ƃ��ȃp�[�T�i���ꂪ�ʓ|�j����Ȃ��Ɩʓ|
const wchar_t* xml_listend = L"</tlkList>";
const wchar_t* xml_chunkbeg = L"<tlkElement>";
const wchar_t* xml_chunkend = L"</tlkElement>";
const wchar_t* xml_idbeg = L"<tlkID>";
const wchar_t* xml_idend = L"</tlkID>";
const wchar_t* xml_textbeg = L"<tlkString>";
const wchar_t* xml_textend = L"</tlkString>";
const wchar_t* xml_whitespace = L"    ";

//XML UTF16
const wchar_t* xml16_linefeed = L"\r\n";
const wchar_t* xml16_head = L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>";
const wchar_t* xml16_listbeg = L"<tlkList>"; // �o�[�W��������H�ς��Ƃ��Ă��ʒu��񂭂炢���� ����Ƃ܂Ƃ��ȃp�[�T�i���ꂪ�ʓ|�j����Ȃ��Ɩʓ|
const wchar_t* xml16_listend = L"</tlkList>";
const wchar_t* xml16_chunkbeg = L"<tlkElement>";
const wchar_t* xml16_chunkend = L"</tlkElement>";
const wchar_t* xml16_idbeg = L"<tlkID>";
const wchar_t* xml16_idend = L"</tlkID>";
const wchar_t* xml16_textbeg = L"<tlkString>";
const wchar_t* xml16_textend = L"</tlkString>";
const wchar_t* xml16_whitespace = L"    ";

//XML UTF8
const wchar_t* xml8_linefeed = L"\r\n";
const wchar_t* xml8_head = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
const wchar_t* xml8_listbeg = L"<tlkList>"; // �o�[�W��������H�ς��Ƃ��Ă��ʒu��񂭂炢���� ����Ƃ܂Ƃ��ȃp�[�T�i���ꂪ�ʓ|�j����Ȃ��Ɩʓ|
const wchar_t* xml8_listend = L"</tlkList>";
const wchar_t* xml8_chunkbeg = L"<tlkElement>";
const wchar_t* xml8_chunkend = L"</tlkElement>";
const wchar_t* xml8_idbeg = L"<tlkID>";
const wchar_t* xml8_idend = L"</tlkID>";
const wchar_t* xml8_textbeg = L"<tlkString>";
const wchar_t* xml8_textend = L"</tlkString>";
const wchar_t* xml8_whitespace = L"    ";