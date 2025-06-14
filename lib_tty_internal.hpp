/* Copyright (c) 2019 Grant Rostig all rights reserved,  grantrostig.com
 */
#pragma once

#define _POSIX_C_SOURCE 200809L  // required for Fedora and Debian Linux, don't know about WIN or MAC.
#include "lib_tty.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <locale>
#include <optional>
#include <string>
#include <thread>
#include <variant>
#include <vector>
#include <type_traits>
// C lang API via C++ provided compatible functions.
#include <cassert>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iterator>
// POSIX headers
#include <termios.h>
// #include <uistd.h>  // I thought I needed it, but apparently not.

// SYMBOL     MEANING
// TODO:     the principal programmer needs todo.
// TODO?:	 the principal programmer is not sure about something, that should be addressed.
// TODO?:X   the X programmer is not sure about something, that should be addressed.
// TODO??:   is a question for verbal discussion at CppMSG.com meetup meetings.

//constexpr std::string    STRING_NULL{"NULL"};    // Value is unset/not-set, similar to how a SQL DB shows an unset field as NULL, which is different than zero length or some magic number.  Here we turn it into a magic number and hope for the best.

//#define NDEBUG   // define if asserts are NOT to be checked.
// Some crude logging that prints source location, where X prints a variable, and R adds \n\r (which is usefull when tty in in RAW or CBREAK mode. Requires C++20.
  #define LOGGER_(  msg )  using loc = std::source_location;std::cout.flush();std::cerr.flush();std::cerr<<    "["<<loc::current().file_name()<<':'<<std::setw(4)<<loc::current().line()<<','<<std::setw(3)<<loc::current().column()<<"]`"<<loc::current().function_name()<<"`:" <<#msg<<           "."    <<endl;cout.flush();cerr.flush();
  #define LOGGER_R( msg )  using loc = std::source_location;std::cout.flush();std::cerr.flush();std::cerr<<"\r\n["<<loc::current().file_name()<<':'<<std::setw(4)<<loc::current().line()<<','<<std::setw(3)<<loc::current().column()<<"]`"<<loc::current().function_name()<<"`:" <<#msg<<           ".\r\n"<<endl;cout.flush();cerr.flush();
  #define LOGGERX(  msg, x)using loc = std::source_location;std::cout.flush();std::cerr.flush();std::cerr<<    "["<<loc::current().file_name()<<':'<<std::setw(4)<<loc::current().line()<<','<<std::setw(3)<<loc::current().column()<<"]`"<<loc::current().function_name()<<"`:" <<#msg<<".:{"<<x<<"}."    <<endl;cout.flush();cerr.flush();
  #define LOGGERXR( msg, x)using loc = std::source_location;std::cout.flush();std::cerr.flush();std::cerr<<"\r\n["<<loc::current().file_name()<<':'<<std::setw(4)<<loc::current().line()<<','<<std::setw(3)<<loc::current().column()<<"]`"<<loc::current().function_name()<<"`:" <<#msg<<".:{"<<x<<"}.\r\n"<<endl;cout.flush();cerr.flush();
//#define LOGGER_( );
//#define LOGGERX( );
//#define LOGGER_R( );
//#define LOGGERXR( );

template<class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };

// Requires that a type has insertion operator
// Concept definition - used by a template below.
template <typename Container>
concept Insertable = requires( std::ostream & out ) {
    requires not std::same_as<std::string, Container>;                                    // OR $ std::is_same <std::string, Container>::value OR std::is_same_v<std::string, Container>;
    { out << typename Container::value_type {} } -> std::convertible_to<std::ostream & >; // OR just $ { out << typename Container::value_type {} };
};

// Prints contents of a container such as a vector of int's.
// Concept used by Templated Function definition
template<typename Container>                        //template<insertable Container>        // OR these 2 lines currently being used.
    requires Insertable<Container>
std::ostream &
operator<<( std::ostream & out, Container const & c) {
    if ( not c.empty()) {
        out << "[<";   //out.width(9);  // TODO??: neither work, only space out first element. //out << std::setw(9);  // TODO??: neither work, only space out first element.
        std::copy(c.begin(), c.end(), std::ostream_iterator< typename Container::value_type >( out, ">,<" ));
        out << "\b\b\b>]"; out.width(); out << std::setw(0);
    } else out << "[CONTAINTER IS EMPTY]";
    return out;
}


namespace Lib_tty {
namespace Detail {
using std::string;
using namespace std::chrono_literals; // for wait().  TODO??: is there a better way? Marc may know.
using namespace std::string_literals;

// provides string with location in source code. Used for debugging.
std::string source_loc();

//inline constexpr   ssize_t   C_EOF =             EOF; // value is: -1 (not 0 as in some older C books 1996 !)  // TODO: why are these ssize_t/long and not short int? // apparently"g not needed by this libary, but worth noting here.
//inline constexpr   ssize_t   C_FERR =            EOF; // apparenlty not needed by this libary, but worth noting here.
//inline constexpr   ssize_t   POSIX_EOF =         0;   // apparenlty not needed by this libary, but worth noting here.

inline constexpr   ssize_t     POSIX_ERROR =       -1;  // yes, believe it or not, it is not zero, which I think is good. :)

inline constexpr   cc_t        VTIME_ESC =         1;  // Designates 1/10 th of a second, the shortest time, and keyboard will easily provide any ESC sequence subsequent characters within that time. VTIME and VMIN are POSIX constructs

inline constexpr   ssize_t     TIMED_GET_NOT_SET = 99; // Designates that no automatic additional multi-byte sequence chars are readable from the keyboard, used for CSI_ESC handling.  TODO: 0 might be bad, especially since above is also 0 and they are used in same function.
                                                       // TODO verify and refactor relationship between TIMED_GET_NOT_SET, TIMED_GET_NULL and CHAR_NULL.
inline constexpr   ssize_t     TIMED_GET_GOT_NULL =    0;  // Designates that no automatic additional multi-byte sequence chars are readable from the keyboard, used for CSI_ESC handling.  TODO: 0 might be bad, especially since above is also 0 and they are used in same function.
inline constexpr   ssize_t     NO_MORE_CHARS =     0;  // Character used like as a "flag", and is concatinated to a singular CSI_ESC read from user, to generate an artificial multi-byte sequence to denote that the CSI_ESC is alone and is not part/start of a multibyte sequence.  But the CSI_ESC alone is important in that it represents the Escape/ESC key, which is a hot_key.
using              Lt_errno =  int;                    // The type for lib_tty errnos, similar to Unix errno. TODO??: better? >using Lt_errno = typeof (errno);
inline constexpr   Lt_errno    E_NO_MATCH =        1;  // Designates that a hot_key has not been found after examining all raw characters of a multi-byte sequence of a key stroke. TODO: new convention for lib_tey errno-like codes
inline constexpr   Lt_errno    E_PARTIAL_MATCH =   2;  // Designates that we have a partial match on the prefix characters, leading to a possible E_NO_MATCH, of above line. TODO: new convention for lib_tey errno-like codes
inline constexpr   Lt_errno    I18N_MATCH =        8;  // TODO: far future, HACK for i18n chars that might have a CSI or not, or time out or not.

/*****************************************************************************/
/**************** START POSIX level declarations *****************************/
/*****************************************************************************/
/********************** START POSIX OS Signals level declarations ************/
/*****************************************************************************/
/* https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/signal.h.html
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#trap
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/kill.html
 * https://www.gnu.org/software/libc/manual/html_node/Standard-Signals.html
 * https://en.wikipedia.org/wiki/Signal_(IPC)
 * https://dsa.cs.tsinghua.edu.cn/oj/static/unix_signal.html
 * kill command does: * SIGHUP * SIGINT * SIGQUIT * SIGABRT * SIGKILL * SIGALARM * SIGTERM and more, see print_signal().
 *
 * stty -a
 * speed 38400 baud; rows 24; columns 80; line = 0;
 * intr = ^C; quit = ^\; erase = ^?; kill = ^U; eof = ^D; eol = <undef>;
 * eol2 = <undef>; swtch = <undef>; start = ^Q; stop = ^S; susp = ^Z; rprnt = ^R;
 * werase = ^W; lnext = ^V; discard = ^O; min = 1; time = 0;
 * -parenb -parodd -cmspar cs8 -hupcl -cstopb cread -clocal -crtscts
 * -ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr icrnl ixon -ixoff
 * -iuclc -ixany -imaxbel iutf8
 * opost -olcuc -ocrnl onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0
 * isig icanon iexten echo echoe echok -echonl -noflsh -xcase -tostop -echoprt
 * echoctl echoke -flusho -extproc
 *
 * https://unix.stackexchange.com/questions/102940/how-do-you-find-what-keys-the-erase-and-line-kill-characters-are-in-ubuntu
 * The stty settings only apply when the terminal is in “cooked mode”, which is the extremely
 * crude command line editor built into the terminal driver. Modern shells have their own editor
 * and set the terminal to raw mode, so the stty settings don't apply. Bash, csh and ksh emulate
 * the cooked mode edition characters, whereas tcsh, zsh and fish stick to their own key bindings.
 *
 *            Cannonical / Raw wait
 * c_cc[5] VTIME  0      / 10
 * c_cc[6] VMIN   1      / 0
 * c_lflag        3538{7}  3538{5} =  TODO: which values for what?
 *
 * TODO: Not sure if this source code would be useful, I removed it for some reason.  Maybe what I have now serves the same purpose, or
 * if I choose to do it later and so there is no such check currently.
 *      if ( char * my_tty_name = ttyname(fileno(stdin)); my_tty_name == nullptr ) // could have used isatty().
 *          perror("user_input_raw: not a tty.");
 *      else
 *          cerr<< "ttyname: "<< my_tty_name << endl;
 *
 *
 *  using Sigaction =   sigaction;  // TODO??: will this work, apparently not?  tried it?  Note the strange c code below.
 *
 */

/** Used by the sigaction POSIX "system call" to designate the action to be taken when a signal arrives to the user process.
 *  The action consists of running a "signal handler" function and sa_flags, which are specified in the user code, running in user space.
 *  Here I have only enforced the C++ class name capitalization convention of the POSIX C type name.
 */
using Siginfo_t =   siginfo_t;

/** Used by the sigaction POSIX "system call" to designate the C type of the signal handler action function to be taken when a signal arrives to the user process.
 *  Here I have only enforced the C++ class name capitalization convention of the POSIX C type name.  TODO: not sure why I ended up with this strange struct, instead of just a typedef.
 */
using Sigaction_handler_fn_t =
    void(
            int,
            Siginfo_t *,
            void *          // ucontext_t*
        );                  // TODO??: TODO why does POSIX have the wrong sighandler_t.  Ie. 1 input parameter versus 3? // TODO??: Maybe use std::function here somehow?
                            // TODO??: ideas: using Handler_func_signature = std::function< sighandler_t(int, siginfo_t *, void *)>;
                            // TODO??: more ideas: std::function< sighandler_t >; // TODO: ideas: typedef void ( * my_magic)(int const, siginfo_t *, void*);

// TODO?: some POSIX thing?
/** Used by the sigaction POSIX "system call" to designate the C type of the signal handler action function to be taken when a signal arrives to the user process.
 *  Here I have only enforced the C++ class name capitalization convention of the POSIX C type name.  TODO: not sure why I ended up with this strange struct, instead of just a typedef.
 */
struct Sigaction_return {
    int       signal_for_user;           // TODO??: why not init this int?
    struct    sigaction                 // Structure describing the action to be taken when a signal arrives.
                        action_prior;   // TODO?? what is this special use of struct statement?  What about an init of this?
};

// A type datastructure to simply function calls and parameters and return types.  TODO: I don't recall why there are five, but I didn't just make up that number. :) Actually I think it is because I choose only 5IGNALs  TODO??: how would I make an array of this strange sub type?
struct Sigaction_termination_return {
                                     // action_prior_SIGINT, action_prior_SIGQUIT, action_prior_SIGTERM, action_prior_SIGTSTP, action_prior_SIGHUP TODO: another way to do this: ~/src/libexcept/libexcept/report_signal.cpp
    struct sigaction &action_prior1; // Structure describing the action to was to have been taken when a signal arrived. Used for restoring tty back to normal/prior state
    struct sigaction &action_prior2;
    struct sigaction &action_prior3;
    struct sigaction &action_prior4;
    struct sigaction &action_prior5;
};

/*****************************************************************************/
/********************** END   POSIX OS Signals level declarations ************/
/*****************************************************************************/
/********************** START POSIX termial IO level declarations ************/
/*****************************************************************************/

// C++ class name capitalization convention of the POSIX C type.
using Termios = termios;    // Tty terminal IO & speed structure, used for getting and setting them. // Enforcing the C++ struct type name capitalization convention for the POSIX C type.  I like it that way.

// to get user's tty termial status
Termios& termio_get();

// to allow capturing one character at a time and gaining control immediately after user presses that key, ie. not buffered.
Termios& termio_set_raw();

// to assist in finding additional keys after CSI/ESC of a multi-character function key.
Termios& termio_set_timer( cc_t const time);

// to normal cooked? condition
void     termio_restore( Termios const &termios);      	/** restore terminal state to what it was when program was started??? TODO: verify my comment here. */

/*****************************************************************************/
/********************** END   POSIX termial IO level declarations ************/
/*****************************************************************************/
/**************** END   POSIX level declarations *****************************/
/*****************************************************************************/
/**************** START Application level declarations ***********************/
/*****************************************************************************/

/** TODO: the user level handling of subsequent character key presses
enum class Input_mode { // TODO: not implemented yet.  // clashes with the boolean I toggle in  HotKeyFunctionCat????, that is partially implemented.
  insert,
  overwrite
    //initial_state         // TODO:
}; */

/** each row of this struct, documents a potential user key press, and places them within various nameing conventions, mostly ASCII and POSIX */
struct Ascii_posix_relation {
  string const ascii_id;
  string const ascii_name;
  string const posix_id;
  char   const ascii_ctrl_char; // note: lowercase and uppercase: for example Ctrl-j and J.
  char   const c_char;          // note: \n for LF
  char   const ascii_char;      // ascii_char and posix_char appear to be the same.  Don't remember why I have both separately, probably some sort of flexibility...?
  char   const posix_char;
};

/** a lookup table for user keyboard key presses */
using Ascii_Posix_table = const std::vector< Ascii_posix_relation >; // TODO: use proper capitalization.

/** Assume any kb in use here has an ESC key and it does what we expect */
constexpr KbFundamentalUnit ESC_KEY = 27;
/** The first char of the POSIX CSI Control Sequence Introducer, the initial ESC character of a series of characters that designates one type of hot_key,
 *  Is the first char in Hot_key_chars. Note there are also hot_keys that are single char such as CTRL-D, or the ESC key on its own.
 *  TODO: Could this be implemented as a "termcap" like table. */
constexpr KbFundamentalUnit CSI_ESC = 27;

/** Is lib_tty's customized user manual entry alternative to initiate the CSI Control Sequence Introducer CSI_ESC, which is the first char in multi-byte sequence Hot_key_chars such as F1.
 *  A cell phone (or other limited/alternate keyboard) user can type this character and follow it by the codes that a full keyboard would do instantly on pressing a keystroke.
 *  Hence this allows manual entry of special function keys, etc.
 *  We chose ` but others include hat ^, double quote ", <CTRL>+<ALT>[ and <CTRL>+<ALT>] (those two actually generate a CSI_ESC followed by the square braket.
 *  https://www.aflahaye.nl/en/frequently-asked-questions/how-to-type-french-accents/#/
 *  TODO: Do we handle a single alt character as a plain character if it does not start a defined mulitbyte sequence? */
constexpr KbFundamentalUnit CSI_MANUAL_ENTRY = '`';

/** Variant that returns either a Hotkey OR an ERRNO.
 *  _o_ === "exclusive or"
 *  Used only as return value by one function, with only one caller (at several locations) which is internal.
 *  TODO: consider renaming to Hot_key vs Hotkey.
 */
using Hot_key_o_errno    = std::variant< Hot_key_table_row, Lt_errno >;
using I18n_key_o_errno  = std::variant< I18n_key_table_row, Lt_errno >;

/** Give it "CSI [ A" get back the end user understandable string name of the hot_key, ie. "right arrow"
 *  Debugging use only at this time. */
std::optional<Hot_key_table_row>
find_hot_key(const Hot_key_table &hot_key_table, const I18n_key_chars this_key);

/*****************************************************************************/
/**************** END   Application Level Declarations ***********************/
/*****************************************************************************/
}  // namespace end Detail NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}  // namespace end Lib_tty NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
