/*
 * Copyright (c) 2019 Grant Rostig all rights reserved,  grantrostig.com
 * BOOST 1.0 license
 */
#ifndef LIB_TTY_H
#define LIB_TTY_H
#define _POSIX_C_SOURCE 200809L

#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <ios>
#include <iostream>
#include <locale>
#include <optional>
#include <string>
#include <thread>
#include <variant>
#include <vector>
// C lang API via C++ provided compatible functions.
#include <cassert>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
// POSIX headers
#include <termios.h>
// #include <unistd.h>  // I thought I needed it, but apparently not.

//#define NDEBUG   // define if asserts are NOT to be checked.

/** Lib_tty README (aka LT and Lt)
 *  A C++ library used to read keyboard(aka kb) / tty input from the user on a character by character basis, as single raw characters like ASCI,
 *  even when they occur within a multibyte sequence such as with kb functions keys such as F1. Noteably it is able to react to a
 *  single key press without waiting for CR or Enter or other line completion user input.  This allows the program to react immediately,
 *  in what is commonly called "raw" and or "CBREAK" mode on a terminal/tty.
 *  Uses: C++17 (mostly just C++14), except for one test function that demonstrates C++ Concepts (C++20) and can be commented out
 *  easily since it is used for debugging.
 *  Requires: termios.h (meaning: terminal input/output system call) which appears to be readily available in Fedora and other
 *  Linux variants, derived from the POSIX UNIX standard.  May also exist on Apple MAC and Windows MinGW, and maybe even Windows POSIX compatibility.
 *  Testing: can be tested manually by a programmger, using this the included main.cpp which can generate a testing program.
 *  Also testing can be manually done by an end-user, using the application program for which lib_tty was created,
 *  which is called "file maintenance" https://github.com/grantrostig/file_maintenance_clipped
 *  The Qt Creator IDE was used to edit and compile the library.  The main program is compiled using the included Makefile.
 *  This project is in early stages of development, but does seem to work.
 *  Except it seems to keep raw mode even after stty sane was restored between each get_kb_keys_raw() call.
 */
namespace Lib_tty {
using std::string;
using namespace std::chrono_literals; // for wait().  todo??: is there a better way? Marc may know.

/// provides string with location in source code. Used for debugging.
std::string source_loc();

//inline constexpr   ssize_t   C_EOF =             EOF; /// value is: -1 (not 0 as in some older C books 1996 !)  // todo: why are these ssize_t/long and not short int? /// apparently"g not needed by this libary, but worth noting here.
//inline constexpr   ssize_t   C_FERR =            EOF; /// apparenlty not needed by this libary, but worth noting here.
//inline constexpr   ssize_t   POSIX_EOF =         0;   /// apparenlty not needed by this libary, but worth noting here.

inline constexpr   ssize_t     POSIX_ERROR =       -1;  /// yes, believe it or not, it is not zero, which I think is good. :)

using              Lt_errno =  int;                    /// The type for lib_tty errnos, similar to Unix errno. todo??: better? >using Lt_errno = typeof (errno);
inline constexpr   ssize_t     TIMED_NULL_GET =    0;  /// Designates that no automatic additional multi-byte sequence chars are readable from the keyboard, used for CSI_ESC handling.  todo: 0 might be bad, especially since above is also 0 and they are used in same function.
inline constexpr   ssize_t     NO_MORE_CHARS =     0;  /// Character used like as a "flag", and is concatinated to a singular CSI_ESC read from user, to generate an artificial multi-byte sequence to denote that the CSI_ESC is alone and is not part/start of a multibyte sequence.  But the CSI_ESC alone is important in that it represents the Escape/ESC key, which is a hot_key.
inline constexpr   Lt_errno    E_NO_MATCH =        1;  /// Designates that a hot_key has not been found after examining all raw characters of a multi-byte sequence of a key stroke. todo: new convention for lib_tey errno-like codes
inline constexpr   Lt_errno    E_PARTIAL_MATCH =   2;  /// Designates that we have a partial match on the prefix characters, leading to a possible E_NO_MATCH, of above line. todo: new convention for lib_tey errno-like codes
inline constexpr   cc_t        VTIME_ESC =         1;  /// Designates 1/10 th of a second, the shortest time, and keyboard will easily provide any ESC sequence subsequent characters within that time. VTIME and VMIN are POSIX constructs

/**  *** POSIX level declarations *** */
/**  *** POSIX OS Signals level declarations *** */

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
 * c_lflag        3538{7}  3538{5} =  todo: which values for what?
 *
 * todo: Not sure if this source code would be useful, I removed it for some reason.  Maybe what I have now serves the same purpose, or
 * if I choose to do it later and so there is no such check currently.
 *      if ( char * my_tty_name = ttyname(fileno(stdin)); my_tty_name == nullptr ) // could have used isatty().
 *          perror("user_input_raw: not a tty.");
 *      else
 *          cerr<< "ttyname: "<< my_tty_name << endl;
 */

// using Sigaction =   sigaction;  // todo??: will this work, apparently not?  tried it?  Note the strange c code below.

/** Used by the sigaction POSIX "system call" to designate the action to be taken when a signal arrives to the user process.
 *  The action consists of running a "signal handler" function and sa_flags, which are specified in the user code, running in user space.
 *  Here I have only enforced the C++ class name capitalization convention of the POSIX C type name.
 */
using Siginfo_t =   siginfo_t;

/** Used by the sigaction POSIX "system call" to designate the C type of the signal handler action function to be taken when a signal arrives to the user process.
 *  Here I have only enforced the C++ class name capitalization convention of the POSIX C type name.  todo: not sure why I ended up with this strange struct, instead of just a typedef.
 */
using Sigaction_handler_fn_t =
    void(
            int,
            Siginfo_t *,
            void *          // ucontext_t*
        );                  // todo??: TODO why does POSIX have the wrong sighandler_t.  Ie. 1 input parameter versus 3? // todo??: Maybe use std::function here somehow?
                            // todo??: ideas: using Handler_func_signature = std::function< sighandler_t(int, siginfo_t *, void *)>;
                            // todo??: more ideas: std::function< sighandler_t >; // todo: ideas: typedef void ( * my_magic)(int const, siginfo_t *, void*);

/// todo?: some POSIX thing?
/** Used by the sigaction POSIX "system call" to designate the C type of the signal handler action function to be taken when a signal arrives to the user process.
 *  Here I have only enforced the C++ class name capitalization convention of the POSIX C type name.  todo: not sure why I ended up with this strange struct, instead of just a typedef.
 */
struct Sigaction_return {
    int       signal_for_user;           // todo??: why not init this int?
    struct    sigaction                 // Structure describing the action to be taken when a signal arrives.
                        action_prior;   // TODO?? what is this special use of struct statement?  What about an init of this?
};

/// A type datastructure to simply function calls and parameters and return types.  todo: I don't recall why there are five, but I didn't just make up that number. :) Actually I think it is because I choose only 5IGNALs  todo??: how would I make an array of this strange sub type?
struct Sigaction_termination_return {
                                     /// action_prior_SIGINT, action_prior_SIGQUIT, action_prior_SIGTERM, action_prior_SIGTSTP, action_prior_SIGHUP todo: another way to do this: ~/src/libexcept/libexcept/report_signal.cpp
    struct sigaction &action_prior1; /// Structure describing the action to was to have been taken when a signal arrived. Used for restoring tty back to normal/prior state
    struct sigaction &action_prior2;
    struct sigaction &action_prior3;
    struct sigaction &action_prior4;
    struct sigaction &action_prior5;
};

/**  *** POSIX termial IO level declarations *** */

/// C++ class name capitalization convention of the POSIX C type.
using Termios = termios;    // Tty terminal IO & speed structure, used for getting and setting them. // Enforcing the C++ struct type name capitalization convention for the POSIX C type.  I like it that way.

/// to get user's tty termial status
Termios &termio_get();

/// to allow capturing one character at a time and gaining control immediately after user presses that key, ie. not buffered.
Termios &termio_set_raw();

/// to assist in finding additional keys after CSI/ESC of a multi-character function key.
Termios &termio_set_timer( cc_t const time);

/// to normal cooked? condition
void     termio_restore( Termios const &termios);      	/** restore terminal state to what it was when program was started??? todo: verify my comment here. */


/**  *** Application Level Declarations *** */

/** There are two semantic EOFs:
 * 1) eof_simple_key_char is a "We probably don't handle eof well."); // todo: more eof handling needed
            file_status = File_status::eof_file_desc logical eof intended by the user by typing in a CTL-D,
 * 2) eof_file_descriptor occurs when a read() fails with that error.
 * But I'm unclear about the exact distinction between these two especially with regard to COOKED versus RAW on a tty.
 * todo: refactor these two out since I don't think we need a separate EOFs, since in tty raw mode we recognize ^D as a hot_key. */
enum class File_status { /// After reading a char, did we get something good, should we expect more?
  eof_simple_key_char, /// got a character that can be interpreted as eof, ie. ^D in Unix, ^Z in DOS/Win?
  eof_file_descriptor, /// get library or OS EOF.
  unexpected_data,     /// got bad data from hardware error, or user error, ie. something we don't expect or support.
  timed_out,           /// we read, but didn't get a value, or status.
  other                /// probably means got a good value.
};                     // todo: eof is a hot_key ie. CTRL-D , so should not be here.

/** the user level intent of all "Categories" of HotKeys */
enum class HotKeyFunctionCat {
  nav_field_completion,     // nav == user navigation between elements of a certain type.  here user want to finish that field input and move to next thing.
  nav_intra_field,          // user wants to move within the single user input field.  Currently only single line, so left arrow and end-line, etc.

  navigation_esc,           // user ESCAPE key, is used similarly to nav_field_completion, todo:not sure if it is needed seperately.

  job_control,              // todo: not implemented yet, nor mapped in hot_keys vector. maybe this is not a HotKeyFunctionCat?? // QUIT Ctrl-z,  STOP Ctrl-s, START Ctrl-q ,
  help_popup,               // user is asking for help using keyboard, ie. F1 for help.
  editing_mode,             // <Insert> (or possibly other) HotKey toggles this.
  na,						// todo:
  other                     // todo:, used as ::na filler designation, probably means none.
};

/** the user level intent of a pressed HotKey of this HotKeyFunctionCat "Category" */
enum class FieldCompletionNav { ///the intent of the user as demonstrated by the last navigation hot_key entered at a prompt. NOTE: each of these has a HotKeyFunctionCat (or is
                                /// ignored/not currently assigned) in hot_keys object in .cpp file.  NOTE: also, each has multiple entries in FNIMap object in file_maintenance
                                /// project which uses lib_tty library.
  no_result,                    /// there was no interaction, and so there is no result.  is this different from ::na?
  up_one_field,
  down_one_field, 				// MOST COMMON response, meaning <CR> <ENTER> <RETURN> key
  skip_one_field, 				// same as <TAB> with no data value.
  skip_to_end_of_fields,
  page_up,
  page_down,
  browse_up,       				// if at default value selection input, then select prior value on list of default values, else consider this as 'up_one_field'.
  browse_down,     				//  also see prior comment.
  save_form_as_is, 				// skip_to_end_of_fields and save?

  esc, 							// skip_to_end_of_fields and don't save, just prompt?
  //help,  						// todo: is this a erronious duplicate from HotKeyFunctionCat?

  eof,                                 // same handling as esc?, no more like <Enter> but save_form_as_is, then get out? todo:
  interrupt_signal,                    // same as esc, no, more like CTRL-C. todo:
  exit_immediately = interrupt_signal, // todo: fix this.
  exit_with_prompts,
  quit_signal, // exit program immediately, and produce a core dump.  todo: security hole for memory? todo:
  kill_signal, // WARNING: this may be invalid since it also appears in FieldIntraNav, however I guess I was thinking that it might have a different meaning if we are not dealing
               // with a input field, but in another contect where we could interpret it as follows: exit program immediately like CTRL-C or posix kill KILL Ctrl-U command? todo:
  na
};

/** the user level intent of a pressed HotKey of this HotKeyFunctionCat "Category" */
enum class FieldIntraNav {
  move_left,
  move_right,
  backspace_left_erase, // delete char to left of cursor
  erase_left,           // delete char to left of cursor
  delete_char,          // delete char to right of cursor
  goto_begin,
  goto_end,
  kill, // remove content of entire line.
  na
};

/** todo: the user level handling of subsequent character key presses
enum class Input_mode { /// todo: not implemented yet.  // clashes with the boolean I toggle in  HotKeyFunctionCat????, that is partially implemented.
  insert,
  overwrite
}; */

/** each row of this struct, documents a potential user key press, and places them within various nameing conventions, mostly ASCII and POSIX */
struct Ascii_posix_relation {
  string ascii_id{};
  string ascii_name{};
  string posix_name{};
  char   ascii_ctrl_char{}; // note: lowercase and uppercase: for example Ctrl-j and J.
  char   c_char{};          // note: \n for LF
  char   ascii_char{};      // ascii_char and posix_char appear to be the same.  Don't remember why I have both separately, probably some sort of flexibility...?
  char   posix_char{};
};

/** a lookup table for user keyboard key presses */
using Ascii_Posix_map = std::vector< Ascii_posix_relation >; // todo: use proper capitalization.

/** Is one keystroke of a ANSI keyboard of a non-special key like 'a' or '6' or '+', that is a "char"
 *  The most basic character type this associated with a user keyboard on the supported computer/OS. todo??: should this be unsigned char or ssize_t or unit8?
 *  one byte long or the components of a hot key sequence.
 *  todo: figure out what one char can store from an international keyboard, does the code assume it is human visible as a normal Alphanumeric character,
 *  then fix next two lines of documentation.*/
using KbFundamentalUnit = char;

using Simple_key_char 	= KbFundamentalUnit;                /// the most basic character that a keyboard can generate like ASCII or UNICODE 8? or 16? or 32?, which does not generate a multibye burst of characters, like F1
using Hot_key_chars 	= std::vector<KbFundamentalUnit>;   /// a sequence of basic characters that are generated by a user single keypress on a keyboard, ie. ESC, or F1 for help.
using Kb_regular_value 	= string;                           /// is one or more normal alphanumeric/ASCII like characters entered by the user //  todo: make this the correct/internationalized char type, which would be KbFundamentalUnit from above?? or what???.

/** The first char of the POSIX CSI Control Sequence Introducer, the initial ESC character of a series of characters that designates one type of hot_key,
 *  Is the first char in Hot_key_chars. Note there are also hot_keys that are single char such as CTRL-D, or the ESC key on its own.
 *  todo: Could this be implemented as a "termcap" like table. */
constexpr KbFundamentalUnit CSI_ESC = 27;

/** Is lib_tty's customized manual alternative to the CSI Control Sequence Introducer CSI_ESC, which is the first char in multi-byte sequence Hot_key_chars such as F1.
 *  A cell phone (or other limited/alternate keyboard) user can type this character and follow it by the codes that a full keyboard would.
 *  so this allows manual entry of very special function keys, etc. */
constexpr KbFundamentalUnit CSI_ALT = '`';

/** A "Hot_key" is one keystroke of a ANSI keyboard of a SPECIAL key like "F1" or "Insert" or ESC or TAB, that is one or more chars/bytes long.
 * For multi-byte sequences, it can start with a CSI_ESC or we allow for another other designated char being CSI_ALT. */
struct Hot_key {
  string             my_name{};
  Hot_key_chars      characters{};
  HotKeyFunctionCat  function_cat{HotKeyFunctionCat::na};      // depending on this value, either of the next two are used, like a discriminated union, or neither are used at all, shown // as ::na. which is the case for ::job_control,::help_popup,::editing_mode,::other.
  FieldCompletionNav f_completion_nav{FieldCompletionNav::na}; // gets a value if HotKeyFunctionCat::nav_field_completion, or HotKeyFunctionCat::navigation_esc
  FieldIntraNav      intra_f_nav{FieldIntraNav::na};           // gets a value if HotKeyFunctionCat::nav_intra_field

  bool               operator<(Hot_key const &) const;         // used to sort the list of easy lookup by characters.
  std::string        to_string() const;                        // for debugging.
};
/// Stores all known Hot_keys for internal library use.
using  Hot_keys = std::vector< Hot_key >;

/** A Hotkey OR an ERRNO.
 *  todo: Need to rework the types/structs that contain Hot_key and other related values, there are TOO many similar ones.
 */
using  Hotkey_o_errno = std::variant< Hot_key, Lt_errno >; /// _o_ == "exclusive or"

/** Hotkey OR a file_status
 *  todo: Need to rework the types/structs that contain Hot_key and other related values, there are TOO many similar ones.
 */
using  Hot_key_o_fstat = std::variant< Hot_key, File_status >; /// _o_ == "exclusive or"

/** Is one char or one Hot_key in various forms,  ie. the result of hitting any key whether it is special or not.
 *  or EOF.
 *  todo: does this include an EOF character?
 *  todo: Need to rework the types/structs that contain Hot_key and other related values, there are TOO many similar ones.
 */
using Kb_key_variant = std::variant< std::monostate, Simple_key_char, Hot_key_chars, Hot_key, File_status >;  // todo: maybe File_status is not needed in RAW case.

/** Tells us if we got a Kb_key and if we got EOF.
 *  _a_ == "and"
 *  todo: we have a problem with File_status, also in Kb_key_variant - we don't need it twice!  This one is not currenlty used?.
 *  todo: consider replacing std::tuple/std::pair with struct!
 *  todo: Need to rework the types/structs that contain Hot_key and other related values, there are TOO many similar ones.
*/
using Kb_key_a_fstat = std::pair< Kb_key_variant, File_status >;

/** a 3 tuple tells us if we got a Kb_key and?, or? a Hot_key, and, or?, is we got EOF. todo?:
 * todo: Need to rework the types/structs that contain Hot_key and other related values, there are TOO many similar ones.
 *  todo: consider replacing std::tuple/std::pair with struct!
 */
using Kb_value_plus = std::tuple< Kb_regular_value, Hot_key, File_status >;
// todo?: is this old, or a new idea? using Kb_value_o_hkey    = std::variant< Kb_regular_value, Hot_key >;

/** Gets one single keystroke from user keyboard, which may consist of multiple characters in a key's multi-byte sequence
 *  Relies on cin being in raw mode!
 *  Called by get_kb_keys_raw().
 *  PUBLIC FUNCTION can also? BE CALLED BY END USER, but not used in the client "file_maintenance_*" programs.
 *  Probably needs debugging, if it is to be called directly. */
Kb_key_a_fstat
get_kb_keystroke();

/** Seeks to get n simple_key_chars from keyboard in raw stty mode.
 *  PUBLIC FUNCTION TO BE CALLED BY END USER, is used in the client "file_maintenance_*" programs.
 *
 * It returns immediately on n number of simple_key_chars and
 * that is the reason 'raw' mode is used/set by this function.
 * The immediate return is what differentiates it from get_value_raw().
 *
 * It can also return 'early' if an appropriate hot-key is entered,
 * but that is not required for the function to return, as already stated above.
 * It may encounter EOF (which is also a hot-key).
 *
 * This function is generally used with n=1.
 * The user's goal is to just grab one key and take action such as in a menu, where a selection is just one key.
 * However, in case some field would want automatic action on n>1, we have provided that feature.
 *
 * Perhaps some menu takes two keys to select the option?  Probably not, but the feature exists.
 * Note that only the last function key pressed by the user, as part of the sequence of keys will be set in result.
 *
 * This function sets POSIX terminal settings and signals for the time that it is getting characters,
 * and lastly restores the terminal to original configuration
 *
 * todo: probably should rename to: get_kb_keystrokes_raw()
 *
 */
Kb_value_plus
get_kb_keys_raw( size_t const length_in_keystrokes,
                 bool const   is_require_field_completion,
                 bool const   is_echo_skc_to_tty,           /// skc == Simple_key_char
                 bool const   is_allow_control_chars );      /// todo: was is_strip_control_chars and now may be a bug?


/** Give it "CSI [ A" get back the end user understandable string name of the hot_key, ie. "right arrow"
 *  Debugging use only at this time. */
std::optional<Hot_key>
find_hot_key(const Hot_keys &hot_keys, const Hot_key_chars this_key);

}  // namespace end Lib_tty

#endif // LIB_TTY_H
