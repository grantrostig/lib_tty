# lib_tty

Used to read keyboard / tty/ kb input on a character by character basis as raw characters even within a multibyte sequence such as with functions keys such as F1.
Also is able to react to a single key press without waiting for CR or Enter.

Uses: C++17 (or probaly even C++11), except for one test function that demonstrates C++ Concepts and can be commented out easily.

Requires: termios.h which appears to be readily available in Fedora and other Linux variants, from the POSIX standard.

Can be tested using this main program, for which lib_tty was created: https://github.com/grantrostig/file_maintenance_clipped

The Qt Creator IDE was used to edit and compile it.

This project is in early stages of development, but does seem to work.
