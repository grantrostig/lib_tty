*** lib_tty ***
(c) Copyright Grant Rostig 2018, all rights reserved.
TRADE SECRECT of Grant Rostig 2018, you agree not to share ideas herein, nor the source code, with others.

NOTE: Any code, comments, submissions, or assistance provided to the author by any party, becomes property of the owner of the code repository
(curently Grant Rostig), free of charge.

+ Subject:
 - Used to read keyboard / tty / kb input on a "character by character" basis as raw characters,
    even multibyte sequences such as those generated by functions keys such as F1.
 - Also is able to react to a single key press without waiting for CR or Enter.

+ Uses: mostly C++17 (or probaly even C++11), except for one test function that demonstrates C++ Concepts and can be commented out easily.
    It is acceptable to use the most modern gcc to develop the code.  Some day this may run in an embedded environment and in that case the code
    can be forked to run on the available version of C++.

+ Requires: termios.h which appears to be readily available in Fedora and other Linux variants, from the POSIX standard.
    Windows and MAC may have some remaining, if not hidden, POSIX support.

+ Testing: Can be tested using this main program, for which lib_tty was created: https://github.com/grantrostig/file_maintenance_clipped

+ IDE: The Qt Creator IDE was used to edit and compile it as a library.
    But a Makefile is provided to compile it as a program using main.cpp which is intended for interactive testing.

+ Author: Grant Rostig, Austin TX USA.

+ TODO: This project is in early stages of development, but does seem to work.
    It requires additional work in the areas of design, coding, documentation and testing.
 - Add "dependency injection" for the source of input keystrokes, currently only from a POSIX tty,
 -- windows tty,
 -- file system file,
 -- pipe,
 -- sockets??
 - Currently only maintenace on a detail data file is supported, other data features such as:
 -- master/detail rows.
 -- dropdowns fields acessing other detail files,
 --- allowing creation of an entry if other detail files are missing an entry.
 - Testing:
 -- Unit
 -- Component
 -- System
 - Post funding of this library project would like to add:
 -- Add a parameter to get_key functions that implement a time interval for which to wait for a character.
 -- Add a parameter to get_key functions that implement a time interval for which to wait for a character, allow for caller logic
    and then resume waiting for the character.
 --- above would allow better user experience during data entry with additional help prompts,
     or a timeout prompt and or subsequent exit of the program.

~ Searchable Symbols in the Source Code ~
This code is open to and appreciates code-reviews, please note that:
SYMBOL     MEANING
 TODO:	   the principal programmer needs to: add, or do, or fix by him/herself.
 TODO?:	   the principal programmer is not sure about something, that should be addressed at some point.
 TODO?X:   the X/other programmer is not sure about something, that should be addressed at some point.
 TODO??:   is a question for verbal discussion at CppMSG.com meetup meetings where the programmer would like help from others.

