#define _POSIX_C_SOURCE 200809L
#ifndef LIB_TTY_H
#define LIB_TTY_H
/*
 * Copyright 2019 Grant Rostig all rights reserved,
 * BOOST
 */
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
#include <cstddef>
#include <cstdio>
#include <cstdlib>
// POSIX
#include <signal.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#define LT_NDEBUG
#undef LT_DEBUG
using namespace std::chrono_literals; // for wait().
using std::endl, std::cin, std::cout, std::cerr, std::string;

constexpr   cc_t        VTIME_ESC =         1;  // 1/10 th of a second, the shortest time, and keyboard will easily provide any ESC sequence subsequent characters within that time.
constexpr   ssize_t     C_EOF =             EOF;// value is: -1 (not 0 as in some older C books 1996 !)  // todo: why are these ssize_t/long and not short int?
constexpr   ssize_t     C_FERR =            EOF;
constexpr   ssize_t     POSIX_EOF =         0;
constexpr   ssize_t     POSIX_ERROR =       -1;
using       Lt_errno =  int;                    // using Lt_errno = typeof (errno);
constexpr   Lt_errno    E_NO_MATCH =        1;  // todo: new convention for my errno like codes
constexpr   Lt_errno    E_PARTIAL_MATCH =   2;  // todo: new convention for my errno like codes
constexpr   ssize_t     NO_MORE_CHARS =     0;  // Identifier to add to CSI_ESC to denote it is not part of a multibyte sequence.
constexpr   ssize_t     TIMED_NULL_GET =    0;  // Flag to show no automatic additional chars appear from the keyboard, used for CSI_ESC.
/* https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/signal.h.html
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#trap
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/kill.html
 * kill command does:
 * SIGHUP
 * SIGINT
 * SIGQUIT
 * SIGABRT
 * SIGKILL
 * SIGALARM
 * SIGTERM
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
 * ?? https://www.gnu.org/software/bash/manual/html_node/Bindable-Readline-Commands.html
 * 			Cannonical	  Raw		wait
 * c_cc[5] VTIME  0	 	  =			10
 * c_cc[6] VMIN   1	 	  =			 0
 * c_lflag	    3538{7}  3538{5}	 =

//if ( char * my_tty_name = ttyname(fileno(stdin)); my_tty_name == nullptr ) // could have used isatty().
//    perror("user_input_raw: not a tty.");
//else
//    cerr<< "ttyname: "<< my_tty_name << endl;
 */

using Termios =     termios;    // terminal & IO & speed structure, used for setting them. // C++ class name capitalization convention of the POSIX C type;
using Siginfo_t =   siginfo_t;  // The siginfo_t structure is passed as the second parameter to a user signal handler function, if the SA_SIGINFO flag was specified when the handler
                                // was installed with sigaction().  // C++ class name capitalization convention of the POSIX C type;
using Sigaction_handler_fn_t =  void(
        int,
        Siginfo_t *,
        void *                      // ucontext_t*
    );                              // todo: TODO why does POSIX have the wrong sighandler_t.  Ie. 1 input parameter versus 3? // maybe use std::function
                                    // todo: ideas: using Handler_func_signature = std::function< sighandler_t(int, siginfo_t *, void *)>;
                                    // std::function< sighandler_t >; // todo: ideas: typedef void ( * my_magic)(int const, siginfo_t *, void*);

//using Sigaction_return =        std::tuple<int /*signal_for_user*/, struct sigaction>; // todo: complete this: replace std::tuple/std::pair with struct!
struct Sigaction_return {
  int /*signal_for_user*/,
  struct sigaction
};

struct Sigaction_termination_return {
  struct sigaction &action_prior1;
  struct sigaction &action_prior2;
  struct sigaction &action_prior3;
  struct sigaction &action_prior4;
  struct sigaction &action_prior5;
};

void print_signal(int const signal);
void handler_termination(int const sig, Siginfo_t *, void *); // The function invoked when handling a termination signal.  // todo: NO: void handler_termination( My_sighandler_t );
                                                              // // todo: NO: My_sighandler_t handler_termination;
void handler_inactivity(int const sig, Siginfo_t *, void *);  // The function invoked when handling an inactivity signal.

Sigaction_termination_return set_sigaction_for_termination(Sigaction_handler_fn_t handler_in); // todo: ideas: use c++20 span.
// void 				sigaction_restore_for_termination(  struct sigaction const action_old1, struct sigaction const action_old2, struct sigaction const
// action_old3
// );
void sigaction_restore_for_termination(Sigaction_termination_return const &);

Sigaction_return set_sigaction_for_inactivity(Sigaction_handler_fn_t handler_in);
void set_a_run_inactivity_timer(timer_t const &inactivity_timer_ptr, int const seconds);

std::tuple<timer_t &, int, struct sigaction> enable_inactivity_handler(int const seconds);
void disable_inactivity_handler(timer_t const inactivity_timer_ptr, int const sig_user, struct sigaction const old_action);

void print_iostate(std::istream const &stream);
bool check_equality(Termios const &, Termios const &);

// void termio_get_old ( Termios & /*OUT*/ status_orig_p);
Termios &termio_get();
Termios &termio_set_raw();
Termios &termio_set_timer(cc_t const time);
void termio_restore(Termios const &terminal_status);
// void termio_unset_timer( Termios const & terminal_status_orig_p );

/* The idea is that eof is logical eof like the user typing in a CTL-D, where eof_file_descriptor is when a read fails with that error,
 * but I'm unclear about the exact distinction between those especially with regard to COOKED versus RAW on a tty.
 * todo: refactor this out since we don't need a separate EOF, since in raw mode we recognize it as a hot_key. */
enum class File_status {
  eof_simple_key_char, // got a character that can be interpreted as eof, ie. ^D in Unix, ^Z in DOS/Win?
  eof_file_descriptor, // get library or OS EOF.
  unexpected_data,     // got bad data from hardware error, or user error, ie. something we don't expect or support.
  timed_out,           // we read, but didn't get a value, or status.
  other                // probably means got a good value.
};                     // todo: eof is a hot_key, so should not be here.

enum class HotKeyFunctionCat {
  nav_field_completion, // nav == navigation between elements of a certain type.

  nav_intra_field,
  navigation_esc, // is used similarly to nav_field_completion, todo:not sure if it is needed seperately.

  job_control,  // todo: not implemented yet, or mapped in hot_keys vector. maybe is not a HotKeyFunctionCat?? // QUIT Ctrl-z,  STOP Ctrl-s, START Ctrl-q ,
  help_popup,   // like F1 for help.
  editing_mode, // <Insert> toggles this.
  na,           //
  other         // used as ::na filler designation, probably means none.
};

enum class FieldCompletionNav { // the intent of the user as demonstrated by the last navigation hot_key entered at a prompt. NOTE: each of these has a HotKeyFunctionCat (or is
                                // ignored/not currently assigned) in hot_keys object in .cpp file.  NOTE: also, each has multiple entries in FNIMap object in file_maintenance
                                // project which uses lib_tty library.
  no_result,                    // there was no interaction, and so there is no result.  is this different from ::na?
  up_one_field,
  down_one_field, // MOST COMMON response, meaning <CR> <ENTER> <RETURN> key
  skip_one_field, // same as <TAB> with no data value.
  skip_to_end_of_fields,
  page_up,
  page_down,
  browse_up,       // if at default value selection input, then select prior value on list of default values, else consider this as 'up_one_field'.
  browse_down,     //  also see prior comment.
  save_form_as_is, // skip_to_end_of_fields and save?

  esc, // skip_to_end_of_fields and don't save, just prompt?
  // help,  // todo: is a bad duplicate from HotKeyFunctionCat?

  eof,                                 // same handling as esc?, no more like <Enter> but save_form_as_is, then get out? todo:
  interrupt_signal,                    // same as esc, no, more like CTRL-C. todo:
  exit_immediately = interrupt_signal, // todo: fix this.
  exit_with_prompts,
  quit_signal, // exit program immediately, and produce a core dump.  todo: security hole for memory? todo:
  kill_signal, // WARNING: this may be invalid since it also appears in FieldIntraNav, however I guess I was thinking that it might have a different meaning if we are not dealing
               // with a input field, but in another contect where we could interpret it as follows: exit program immediately like CTRL-C or posix kill KILL Ctrl-U command? todo:
  na
};

enum class FieldIntraNav { // each of these has a HotKeyFunctionCat
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

enum class Input_mode { // todo: not implemented yet.  // clashes with the boolean I toggle, that is partially implemented.
  insert,
  overwrite
};

struct Ascii_posix_relation {
  string ascii_id{};
  string ascii_name{};
  string posix_name{};
  char ascii_ctrl_char{}; // note: lowercase and uppercase: for example Ctrl-j and J.
  char c_char{};          // note: \n for LF
  char ascii_char{};      // ascii_char and posix_char appear to be the same.  Don't remember why I have both separately, probably some sort of flexibility...?
  char posix_char{};
};
using Ascii_Posix_map = std::vector<Ascii_posix_relation>; // todo: use proper capitalization.

// todo: figure out what one char can store from an international keyboard, does the code assume it is human visible as a normal Alphanumeric character, then fix next two lines of
// documentation.
using KbFundamentalUnit = char; // todo:should this be unsigned or ssize_t? Is one keystroke of a ANSI keyboard of a non-special key like 'a' or '6' or '+', that is a "char" one
                                // byte long or the components of a hot key sequence.
using Simple_key_char = KbFundamentalUnit; // a single ascii value that represents one key press on a keyboard, which does not generate a multibye burst of characters, like F1
using Hot_key_chars = std::vector<KbFundamentalUnit>;
using Kb_regular_value = string;          // a value is one or more normal alphanumeric characters entered by the user //  todo: make this the correct/internationalized char type.
constexpr KbFundamentalUnit CSI_ESC = 27; // the first char of POSIX CSI Control Sequence Introducer, the ESC character that designates a hot_key, is the first char in
                                          // Hot_key_chars. this could turn into a "termcap" like table.
constexpr KbFundamentalUnit CSI_ALT =
    '`'; // the first char of our special manual alternative CSI Control Sequence Introducer, the character that designates a hot_key, is the first char in Hot_key_chars. The cell
         // phone (or other limited/alternate keyboard) user can type this character and follow it by the codes that their full keyboard would.

/* a "Hot_key" is one keystroke of a ANSI keyboard of a SPECIAL key like "F1" or "Insert" or ESC or TAB, that is one or more chars/bytes long.
 * It can start with a CSI_ESC or other designated char. */
struct Hot_key {
  string my_name{};
  Hot_key_chars characters{};
  HotKeyFunctionCat function_cat{HotKeyFunctionCat::na}; // depending on this value, either of the next two are used, like a discriminated union, or neither are used at all, shown
                                                         // as ::na. which is the case for ::job_control,::help_popup,::editing_mode,::other.
  FieldCompletionNav f_completion_nav{FieldCompletionNav::na}; // gets a value if HotKeyFunctionCat::nav_field_completion, or HotKeyFunctionCat::navigation_esc
  FieldIntraNav intra_f_nav{FieldIntraNav::na};                // gets a value if HotKeyFunctionCat::nav_intra_field
  bool operator<(Hot_key const &) const;                       // used to sort the list of easy lookup by characters.
};
using Hot_keys = std::vector<Hot_key>;
using Hotkey_o_errno = std::variant<Hot_key, Lt_errno>;

using Hot_key_o_fstat = std::variant<Hot_key, File_status>; // _o_ == "exclusive or"

// a "Kb_key" is one char or one Hot_key, ie. the result of hitting any key whether it is special or not. todo: does this include an EOF character?
using Kb_key_optvariant = std::variant<std::monostate, Simple_key_char, Hot_key_chars, Hot_key,
                                       File_status>;              // todo: maybe File_status is not needed in RAW case.
using Kb_key_a_fstat = std::pair<Kb_key_optvariant, File_status>; // _a_ == "and"  todo: we have a problem with File_status, don't need it twice!  This one is not used.
                                                                  // todo: complete this: replace std::tuple/std::pair with struct!
using Kb_value_plus = std::tuple<Kb_regular_value, Hot_key, File_status>;
// using Kb_value_o_hkey    = std::variant< Kb_regular_value, Hot_key >;

std::optional<Hot_key> find_hot_key(const Hot_keys &hot_keys, const Hot_key_chars this_key);

Kb_key_a_fstat get_kb_key(bool const is_strip_control_chars = true);

void nav_intra_field(Hot_key const &hk, Kb_regular_value &value, unsigned int &value_index, bool const is_echo_chars = true);

/*****
 *  get_kb_keys_raw()
 * Seeks to get simple_key_chars (0-n in length).
 *
 * It returns immediately on 'n' simple_key_chars and
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
 * Maybe a menu takes two keys?  Probably not, but the feature exists.
 */
Kb_value_plus get_kb_keys_raw(size_t length_in_simple_key_chars, bool const is_require_field_completion = true, bool const echo_skc_to_tty = true,
                              bool const is_strip_control_chars = true, bool const is_password = false);

#endif // LIB_TTY_H
