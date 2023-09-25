*** lib_tty ***
+(c) Copyright Grant Rostig 2018, all rights reserved.

NOTE: Any code, comments, submissions, or assistance provided to the author by any party, becomes property of the owner of the code repository (curently Grant Rostig), free of charge.

Used to read keyboard / tty / kb input on a "character by character" basis as raw characters, even within a multibyte sequence such as with functions keys such as F1.  

Also is able to react to a single key press without waiting for CR or Enter.

+Uses: mostly C++17 (or probaly even C++11), except for one test function that demonstrates C++ Concepts and can be commented out easily.  It is acceptable to use the most modern gcc to develop the code.  Some day this may run in an embedded environment and in that case the code can be forked to run on the available version of C++.

+Requires: termios.h which appears to be readily available in Fedora and other Linux variants, from the POSIX standard.  Windows and MAC may have some remaining, if not hidden, POSIX support.

+Testing: Can be tested using this main program, for which lib_tty was created: https://github.com/grantrostig/file_maintenance_clipped

+IDE: The Qt Creator IDE was used to edit and compile it as a library.  But a Makefile is provided to compile it as a program using main.cpp which is intended for interactive testing.

+Author: Grant Rostig, Austin TX USA.

+ NOTE: Any code, comments, submissions, or assistance provided to the author by any party, becomes property of the owner of the code repository (curently Grant Rostig), free of charge.

+todo: This project is in early stages of development, but does seem to work in the areas of design, coding, documentation and testing.

~ Searchable Symbols in the Source Code ~
SYMBOL     MEANING
 TODO:	   the principal programmer needs todo.
 TODO?:	   the principal programmer is not sure about something, that should be addressed.
 TODO?X:   the X/other programmer is not sure about something, that should be addressed.
 TODO??:   is a question for verbal discussion at CppMSG.com meetup meetings.

