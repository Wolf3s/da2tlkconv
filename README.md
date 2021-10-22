# Dragon Age 2 TLK Converter

## Description
__Dragon Age 2 TLK Converter (da2tlkconv)__ is an unofficial tool for Dragon Age 2. It converts a TLK file into a text file or a text file into a TLK file.

See readme_ja.txt for more details (It's written in japanese).

### Websites
* http://blackmasqueradegames.com
* http://social.bioware.com/project/4206 (Archived)

### LICENSE
MIT  
Copyright (c) 2011 hikami, aka longod

## Usage
da2tlkconv [option]... \<input path> \<output path>

* Converting (required & exclusion):
    * -d  Convert TLK into Text.
    * -c  Convert Text into TLK.
* Text Formatting (optional, combine -d or -c)
    * -x  Use XML format.
    * -t  Use brace text format (see below).
* Output Controlling (optional)
    * -i  Ignore IDs that have an empty string (combine -d).
    * -a  Add an ID to beginning of the string (combine -c).

## Format
* Text file encoding must be UTF16-LE.

* Default
    * Writing a single line.
    * Brace '{', '}' and Linefeed are NOT available in the string.
    * Linefeed is expressed as "\r\n".
* -t: Brace
    * Strings is in brace '{', '}'.
    * Brace '{', '}' is NOT available in the string.
    * Linefeed is available.
* -x XML (recommend)
    * XML Tags are NOT available.
    * Brace and linefeed are available.

## TODO
* Support UTF-8 encoding, if I need.

## Known issues
* Internal coding offset addresses are different from the original TLK file. But it works in game.
* Internal tree serializing are are different from the original TLK file. But it works in game.

## CHANGELOG
* 2011/03/06 v.0.6.1
    * Fixed locale (OS default).
* 2011/03/06 v.0.6
    * Added -x option.
    * Added -t option.
    * Fixed a memory leak.
* 2011/03/01 v.0.4
    * Added -i option.
    * Added -a option.
    * Changed buffering (prepared XML).
    * Fixed processing same strings.
    * Fixed outputing error messeages from error codes.
* 2011/02/27 v.0.1
    * Initial release.

