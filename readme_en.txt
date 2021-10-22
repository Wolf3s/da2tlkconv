--------------------------------------------------------------------------------
Name   : Dragon Age 2 TLK Converter
Auther : Hikami (Longod)
Web    : http://hotstudio.net/serika/, http://social.bioware.com/project/4206/
Version: 0.6.1
Date   : 2011/03/13
--------------------------------------------------------------------------------

Sorry, my english is not good.

--------------------------------------------------------------------------------
Description:
Dragon Age 2 TLK Converter is Unofficial Tool that converting TLK file into
text file (or converting txt into tlk).
This TLK file is in "Dragon Age 2" that developed by BioWare.
This text file encode is UTF16-LE.

--------------------------------------------------------------------------------
Usage:
da2tlkconv [option]... <input path> <output path>
    Convert selection (required & exclusion):
        -d  Convert TLK into Text(UTF-16LE).
        -c  Convert Text(UTF-16LE) into TLK.
    Format change:
        -x  using XML format.
        -t  using Nesting text format.
    Output control:
        -i  Ignore IDs that have an empty string (combine -d).
        -a  Add an ID to beginning of the string (combine -c).

--------------------------------------------------------------------------------
Formats:
*Default
    Writing a single line.
    Not available "{", "}" and linefeed in the string.
    Linefeed as alternated "\r\n".
*Nesting -t
    String nested by "{","}".
    Not available "{", "}" in the string.
*XML -x (recommend)
    Not available parsing Tags.
    Other Tags are available.

--------------------------------------------------------------------------------
Todo:
*It support UTF-8 encoding, if I need.

--------------------------------------------------------------------------------
Known issues:
see readme_ja.txt for details (japanese).
*Different coding offset address. But it work in game.
*Different tree serializing. But it work in game.

--------------------------------------------------------------------------------
History:
see readme_ja.txt for details (japanese).
2011/03/06 v.0.6.1
*Using OS-default locale.

2011/03/06 v.0.6
*Added option -x.
*Added option -t.
*Fixed a memory leak.

2011/03/01 v.0.4
*Added option -i.
*Added option -a.
*Changed buffering (Prepare outputting XML).
*Fixed processing same strings.
*Fixed right error messeage output from error code.

2011/02/27 v.0.1
Initial release.
