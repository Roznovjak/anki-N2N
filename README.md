# anki-N2N
Anki Notes To Notes - The command line tool for creating Anki notes from a txt file.

Requires installed [ICU4c](http://site.icu-project.org/home)( *uc* component, version 67) and [curl](https://curl.haxx.se/libcurl/) libraries. The program also requires running desktop [Anki](https://apps.ankiweb.net/) application with installed [AnkiConnect](https://ankiweb.net/shared/info/2055492159) plugin. <br />
The program expects at least three program parameters:
```
$ anki-N2N path_to_file deck_name note_type [tag ...]
```
> `path_to_file` - a full path to a file containing data for creating *Anki* cards. Please see example [file](https://github.com/Roznovjak/anki-N2N/blob/master/example_note_file.txt). The file have to contain hyphen separated words representing *Front* and *Back* note fields. Empty lines, lines with duplicate *Frond* fields and lines or lines not following the `text - text` pattern (preceding and following spaces are ignored) will be ignored. <br />
> `deck_name` - The name of a deck. The deck has to already exist. <br />
> `note_type` - The name of a note type. The note has to already exist and have two fields: *Front* and *Back*. <br />
> `tag` - Optional. May be specified multiple times. <br /> <br />

#### Building on Linux or MacOs
Obtain the sources:
```
$ git clone https://github.com/Roznovjak/anki-N2N.git
```
Build:
```
$mkdir build
cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
```
Mac users can generate *Xcode* project instead of *make* files:
```
$mkdir build_xcode
cd build
cmake -G "Xcode" -DCMAKE_BUILD_TYPE=Release ..
```

#### Example
```
$ anki-N2N /Users/my_name/Documents/New_vocabulary.txt "Mi Español" "Spanish-Basic (and reversed card)" Español Vocabulario
$ XY notes( XY cards) have been created.
```
#### Warning
Don't forget to backup your *Anki* data.

#### Mac Users
Set `export CMAKE_PREFIX_PATH=/usr/local/opt/icu4c` if *Cmake* can't find *ICU* libraries.
