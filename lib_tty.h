/* Copyright (c) 2019 Grant Rostig all rights reserved,  grantrostig.com
 */
#ifndef LIB_TTY_H
#define LIB_TTY_H
// posix define might be needed?  todo??:
#define _POSIX_C_SOURCE 200809L

#include <deque>
#include <ios>
#include <iostream>
#include <locale>
#include <optional>
#include <string>
#include <variant>
#include <vector>

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
/*****************************************************************************/
/**************** START Lib_tty     Level Declarations ***********************/
/*****************************************************************************/
using Lt_errno =  int;                    /// The type for lib_tty errnos, similar to Unix errno. todo??: better? >using Lt_errno = typeof (errno);
inline constexpr std::string STRING_NULL {"NULL"};  // SQL DB show an unset field as NULL, which is different than zero length or some magic number.  Here we turn it into a magic number and hope for the best.

/** There are two semantic EOFs:
 *  1) eof_Key_char_singular is a "We probably don't handle eof well."); // todo: more eof handling needed
            file_status = File_status::eof_file_desc logical eof intended by the user by typing in a CTL-D,
 *  2) eof_file_descriptor occurs when a read() fails with that error.
 *  But I'm unclear about the exact distinction between these two especially with regard to COOKED versus RAW on a tty.
 *  todo: refactor these two out since I don't think we need a separate EOFs, since in tty raw mode we recognize ^D as a hot_key.
 */
enum class File_status { /// After reading a char, did we get something good, should we expect more?
  initial_state,         /// initial state, should not remain this way after any tty read() or get().
  other,                /// probably means got a good value.
  unexpected_data,      /// got bad data from hardware error, or user error, ie. something we don't expect or support.
  eof_Key_char_singular,/// got a character that can be interpreted as eof, ie. ^D in Unix, ^Z in DOS/Win?
  eof_library,          /// get application library EOF.
  eof_file_descriptor,  /// get library or OS EOF.
  timed_out,            /// we read, but didn't get a value, or status.
  fail,                 /// we read, but we got format/extraction error.
  bad                   /// we read, but we got serious unrecoverable error.
};

/** The user level intent of all "Categories" of HotKeys
 */
enum class HotKeyFunctionCat {
  nav_field_completion,     // nav == user navigation between elements of a certain type.  here user want to finish that field input and move to next thing.
  nav_intra_field,          // user wants to move within the single user input field.  Currently only single line, so left arrow and end-line, etc.

  navigation_esc,           // user ESCAPE key, is used similarly to nav_field_completion, todo:not sure if it is needed seperately.

  job_control,              // todo: not implemented yet, nor mapped in hot_keys vector. maybe this is not a HotKeyFunctionCat?? // QUIT Ctrl-z,  STOP Ctrl-s, START Ctrl-q ,
  help_popup,               // user is asking for help using keyboard, ie. F1 for help.
  editing_mode,             // <Insert> (or possibly other) HotKey toggles this.
  na,						// todo:
  other                     // todo:, used as ::na filler designation, probably means none.
    //initial_state         // todo:
};

/** The user level intent of a pressed HotKey of this HotKeyFunctionCat "Category"
 */
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
    //initial_state         // todo:
};

/** The user level intent of a pressed HotKey of this HotKeyFunctionCat "Category"
 */
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
    //initial_state         // todo:
};

/** Is one keystroke of a ANSI keyboard of a non-special key like 'a' or '6' or '+', that is a "char"
 *  The most basic character type this associated with a user keyboard on the supported computer/OS. todo??: should this be unsigned char or ssize_t or unit8?
 *  one byte long or the components of a hot key sequence.
 *  todo: figure out what one char can store from an international keyboard, does the code assume it is human visible as a normal Alphanumeric character,
 *  then fix next two lines of documentation.
 */
using KbFundamentalUnit = char;                             /// The simplest thing that can be read from stdin from a tty via a hardware keyboard, on a Linux/POSIX workstation.
using Key_char_singular = KbFundamentalUnit;                /// The most basic character that a keyboard can generate like ASCII or UNICODE 8? or 16? or 32?, which does not generate a multi-btye burst of characters, like F1

/** Is usally one (on a US Kb, but some keys probably have more than one on other language Kbs)
 *  normal/regular alphanumeric/ASCII like characters entered by the user.
 *  todo: Should probably rename this to Kb_regular_keystroke .
 *  was?: Kb_regular_key .
 *  todo: make this the correct/internationalized char type, which would be KbFundamentalUnit from above?? or what???.
*/
using Key_char_i18ns 	    = std::string;

using Hot_key_chars     = std::vector< KbFundamentalUnit >;   /// A sequence of basic chars that are generated by a user single keypress of a Hot_key on a keyboard, ie. ESC, or F1 for help. but does not include a or 2 or /

/** A "Hot_key" is one keystroke of a ANSI keyboard of a SPECIAL key like "F1" or "Insert" or ESC or TAB, that is one or more chars/bytes long.
 *  For multi-byte sequences that flow from one keystroke, it can start with a CSI_ESC or we allow for another other designated char being CSI_ALT.
 */
class Hot_key {
public:
  std::string        my_name            {STRING_NULL};              /// Name given by Lib_tty
  Hot_key_chars      characters         {STRING_NULL.cbegin(),STRING_NULL.cend()}; /// See the type's documentation.
  HotKeyFunctionCat  function_cat       {HotKeyFunctionCat::na};    /// Depending on this value, one or both of the following two data members are used. ::na is the case for ::job_control,::help_popup,::editing_mode,::other.
  FieldCompletionNav f_completion_nav   {FieldCompletionNav::na};   /// gets a value if HotKeyFunctionCat::nav_field_completion, or HotKeyFunctionCat::navigation_esc
  FieldIntraNav      intra_f_nav        {FieldIntraNav::na};        /// gets a value if HotKeyFunctionCat::nav_intra_field

  bool               operator<( Hot_key const &) const;             /// Used to sort the members of a table to enable easy algorithmic lookup by characters within the table.
  std::string        to_string()                 const;             /// Used for debugging only.
};
using Hot_keys = std::vector< Hot_key >; /// Stores all known Hot_keys for internal library use.

/** Is one char or one Hot_key in various forms,  ie. the result of hitting any key whether it is special or not.
 *  or EOF.
 *  Yes, this is everything but the kitchen sink.  Probaly excessive and need refactoring.
 *  todo: does this include an EOF character?
 *  was: //key_variant = std::variant< std::monostate, Key_char_singular, Key_char_i18ns, Hot_key_chars, Hot_key, File_status >;
 */
using   Kb_key_variant = std::variant< std::monostate, Key_char_singular,               Hot_key_chars, Hot_key              >;

/** Tells us if we got a Kb_key and if we "are at"/"or got?" EOF.
 *  _a_ == "and"
 *  todo: we have a problem with File_status, also in Kb_key_variant - we don't need it twice!  This one IS currenlty used.
 *  todo: Need to rework the types/structs that contain Hot_key and other related values, there are TOO many similar ones.
 *  was //using Kb_key_a_fstat = std::pair< Kb_key_variant, File_status >;
    // We return either regular char(s), or a Hot_key
*/
struct Kb_key_a_fstat {
  Kb_key_variant    kb_key_variant  {};                             /// some datatype form of the key
  File_status       file_status     {File_status::initial_state};   /// holds what is happening with cin EOF
};

/** a 3 tuple tells us if we got a Kb_key and?, or? a Hot_key, and, or?, is we got EOF. todo?:
 *  Heavily used everywhere!
 *  todo: Need to rework the types/structs that contain Hot_key and other related values, there are TOO many similar ones.
 *  todo: consider replacing std::tuple/std::pair with struct!
 *  was: //using Kb_value_plus = std::tuple< Key_char_i18ns, Hot_key, File_status >;
 */
struct Kb_value_plus {
  Key_char_i18ns    key_char_i18ns  {STRING_NULL};
  Hot_key           hot_key         {};
  File_status       file_status     {File_status::initial_state};
};

/** Gets one single keystroke from user keyboard, which may consist of multiple characters in a key's multi-byte sequence
 *  Relies on cin being in raw mode!
 *  Called by get_kb_keystrokes_raw().
 *  PUBLIC FUNCTION can also? BE CALLED BY END USER, but not used in the client "file_maintenance_*" programs.
 *  Probably needs debugging, if it is to be called directly. */
Kb_key_a_fstat
get_kb_keystroke_raw();

/** Seeks to get n Key_char_singular(s) from keyboard in raw stty mode.
 *  PUBLIC FUNCTION TO BE CALLED BY END USER, is used in the client "file_maintenance_*" programs.
 *
 * It returns immediately on n number of Key_char_singulars and
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
 * \pre  { std::cin must be attached to stdin, which must be attached to a physical keyboard, or perhaps redirected, but that case has not been tested. }
 * std::cin must be attached to stdin, which must be attached to a physical keyboard, or perhaps redirected, but that case has not been tested.
 *
 * @post { Same as \pre. }
 *
 * The goal is to get one keystroke from the user, of which there could be the following cases:
 * 1) an ASCII 7 or 8 bit character
 * 2) an international (i18n) character that consists of a multibyte sequence of 7 or 8 bit characters
 * 3) an hot_key, defined as the keys on a USA PC keyboard that a user presses and expects the computer to react immediately such as
 *       + Enter/CR/Return
 *       + ESC
 *       + F1 and other function keys including INSERT, and windows meta key?
 * 4) a EOF designated by the user typing Control-D on Linux.
 * 5) a OS generated EOF? maybe when a pipe ends?, or the shell dies?
 * 5) a hardware generated EOF?  maybe when a terminal transmits it somehow?
 *
 * Keystrokes can generate either single or multiple chars where one read via get() or getchar(), gets only one char.
 * as follows:
 *      +single   ASCII 7 or 8 bit chars    char, unsigned char, signed char    8 bits
 *      +multiple IBM PC AT function keys   char                                8 bits
 *    - more esoteric stuff that can be deferred for now. todo:
 *      +single   Unicode 8  UTF-8          char8_t                             8 bits
 *      +multiple Unicode 8  UTF-8          char8_t                             8 bits                  Attempting to handle or at least consider now.
 *      +single   Microsoft Wide Char       wchar_t                             16 bits  AKA UTF-16LE
 *      +multiple Microsoft Wide Char       wchar_t                             16 bits  AKA UTF-16LE
 *      +single   Unicode 16 UTF-16         char16_t                            16 bits
 *      +single   Unicode 32 UTF-32         char32_t                            32 bits
 *
 * To get one ASCII char which may represent some part of a keystroke in UNIX, POSIX, or Linux,
 * the tty is set to a special mode called CBREAK or RAW.  Normally a tty attached to terminal is set to COOKED mode.
 *
    // We return either regular char(s), or a Hot_key
 */
Kb_value_plus
get_kb_keystrokes_raw( size_t const length_in_keystrokes,
                 bool const   is_require_field_completion,
                 bool const   is_echo_skc_to_tty,           /// skc == Key_char_singular
                 bool const   is_allow_control_chars );

/*****************************************************************************/
/**************** END   Lib_tty     Level Declarations ***********************/
/*****************************************************************************/
}  // namespace end Lib_tty
#endif // LIB_TTY_H
