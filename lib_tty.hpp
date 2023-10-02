/* Copyright (c) 2019 Grant Rostig all rights reserved,  grantrostig.com
 */
#pragma once
#define _POSIX_C_SOURCE 200809L // warning: posix define might also be needed in here in future.
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
inline constexpr std::string       STRING_NULL {"NULL"};    /// Value is unset/not-set, similar to how a SQL DB shows an unset field as NULL, which is different than zero length or some magic number.  Here we turn it into a magic number and hope for the best.

/** There are two semantic EOFs:
 *  1) eof_Key_char_singular is a "We probably don't handle eof well."); // TODO: more eof handling needed
            file_status = File_status::eof_file_desc logical eof intended by the user by typing in a CTL-D,
 *  2) eof_file_descriptor occurs when a read() fails with that error.
 *  But I'm unclear about the exact distinction between these two especially with regard to COOKED versus RAW on a tty.
 *  TODO: refactor these two out since I don't think we need a separate EOFs, since in tty raw mode we recognize ^D as a hot_key.
 */
enum class File_status { /// After reading a char, did we get something good, should we expect more?
  initial_state,         /// initial state, should not remain this way after any tty read() or get().
  good,                  /// we read and all is good() on cin.  TODO: implement this throughout lib_tty.
  timed_out,             /// we read, but didn't get a value, could represent case where the user is hand typing in the CSI hot_key.
  //eof_Key_char_singular, /// got a character that can be interpreted as eof, ie. ^D in Unix, ^Z in DOS/Win?
  //eof_library,           /// get application library EOF.  TODO: not used, determine if different than hot_key CTRL-D, which is a nav_field_completion
  eof_file_descriptor,   /// get library or OS EOF.
  fail,                  /// we read, but we got format/extraction error.
  bad                   /// we read, but we got serious unrecoverable error.
  //other_user_kb_char_data_HACK,      /// probably means got a good kb_char value.  TODO was associated with "good", but need to separate the concepts and fix kb_char error return value, get this error code out of here. Also related to Lt_errno type.
  //unexpected_user_kb_char_data_HACK  /// got bad data from hardware error, or user error, ie. something we don't expect or support. TODO: fix kb_char error return value, get this error code out of here. Also related to Lt_errno type.
};

/** The "Categories" of user level intent expressed by use of HotKeys
 */
enum class HotKeyFunctionCat {
  none,						// a hot_key that is not yet assigned to a user intent function in the table.
  none_i18n,				// NOT a hot_key, but instead a french or other keyboard that uses special multibyte sequences, similar to hot_keys but are not since they don't express IntentNav.
  help_popup,               // user is asking for help using keyboard, ie. F1 for help.
  nav_intra_field,          // user wants to move within the single user input field.  Currently we have only a single line interface, so left arrow and end-line, <BS> backspace, <DEL> etc.
                            // TODO?: NOTE: Should such nav_intra be handled by the client of the libaray, or what handling should be done with this libary when reading keystrokes?
  editing_mode,             // <Insert> (or possibly other) HotKey toggles this.

  nav_field_completion,     // nav == user navigation between elements of a certain type. Here user want to finish that field input and move to next thing.
                            // note: this include CTRL-D entered by user, but not other types of EOF such as from the device.
  navigation_esc,           // user ESCAPE key, is used similarly to nav_field_completion, TODO: not sure if it is needed seperately.

  job_control,              // TODO: not implemented yet, nor mapped in hot_keys vector. maybe this is not a HotKeyFunctionCat?? // QUIT Ctrl-z,  STOP Ctrl-s, START Ctrl-q ,
  initial_state             // TODO: probably need some assert for logic errors.
};

/// The user level intent of a pressed HotKey of this HotKeyFunctionCat "Category",
/// The intent of the user as demonstrated by the last navigation hot_key entered at a prompt.
/// User's intent for what should happen after inputting a particular hot_key at a data field.
/// NOTE: each of these has a HotKeyFunctionCat (or is ignored/not currently assigned) in hot_keys object in .cpp file.
/// NOTE: also, each has multiple entries in FNIMap object in file_maintenance project which uses lib_tty library.
enum class InteractionIntentNav {
    initial_state,              // NULL - never set.
    na,                         // not applicable
    no_result,                  // there was no interaction, and so there is no result.

    //help,  				    // TODO: is this a erronious duplicate from HotKeyFunctionCat?
    esc, 					    // skip_to_end_of_fields and don't save, just prompt?
    eof,                        // same handling as esc?, no more like <Enter> but save_form_as_is, then get out? TODO:
    interrupt_signal,           // same as esc, no, more like CTRL-C. TODO:
    quit_signal,                // exit program immediately, and produce a core dump.  TODO: security hole for memory? TODO:
    kill_signal,                // WARNING: this may be invalid since it also appears in FieldIntraNav, however I guess I was thinking that it might have a different meaning if we are not dealing with a input field, but in another contect where we could interpret it as follows: exit program immediately like CTRL-C or posix kill KILL Ctrl-U command? TODO:

    // *** universal navigation commands, this group operates at all levels
    exit_pgm_immediately = interrupt_signal, // TODO: fix this.
    exit_pgm_with_prompts,
    exit_fn_immediately,
    exit_fn_with_prompts,
    // *** menu specific navigation commands  	- supplemented by universals'
    retain_menu,                // Once the action selected from a particular menu completes, stay at that sub-menu.
    prior_menu,                 // Once the action selected from a particular menu completes, DO NOT stay at that sub-menu, but instead drop back to the prior menu in the navigation stack.  This was initally intended for actions that only input one value, so in fact the menu action was simply an input field.  May now be impractical or useless, but it could still be usefull/simpler/easier to use if just grabbing a simple value.
    prior_menu_discard_value,   // Related to "prior_menu", but can't think of an example right now.  TODO:
    main_menu,                  // Once the action selected from a particular menu completes, goto the main menu.
    exit_all_menu,              // Once the action selected from a particular menu completes, exit all menus, not sure where that would take the user? Perhaps the startup of the program where basic initialization questions are asked of the user before the main menu is displayed.
    // *** dialog specific navigation commands  - supplemented by menus'
    continue_forward_pagination,
    continue_backward_pagination,
    // *** detail_row 			- supplemented by menus'
    save_form_as_is, 			// skip_to_end_of_fields and save?
    page_up,
    page_down,
    up_one_field,
    down_one_field, 			// MOST COMMON response, meaning <CR> <ENTER> <RETURN> key
    skip_one_field, 			// same as <TAB> with no data value.
    skip_to_end_of_fields,
    // *** block  			 	   supplemented by menus'
    up_one_block,               // Top of input block, or if there then top of prior block. A block would be a sub-section of an input screen, if there were two or more sections. A complex example would be having a master/detail input screen.
    down_one_block,             // Bottom of input block, or if there then bottom of next block. A block would be a sub-section of an input screen, if there were two or more sections. A complex example would be having a master/detail input screen.
    // *** inter_row
    next_row,
    prior_row,
    first_row,
    last_row,
    // *** field
    browse_up,     				// if at default value selection input, then select prior value on list of default values, else consider this as 'up_one_field'.
    browse_down   				//  also see prior comment.
    //store_value
                                // TODO: should we have a separate enum for dialFieldCompletionNavogs, menus, data_fields?
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
    //initial_state         // TODO:
};

/** Is one keystroke of a ANSI keyboard of a non-special key like 'a' or '6' or '+', that is a "char"
 *  The most basic character type this associated with a user keyboard on the supported computer/OS. TODO??: should this be unsigned char or ssize_t or unit8?
 *  one byte long or the components of a hot key sequence.
 *  The simplest thing that can be read from stdin from a tty via a hardware keyboard, on a Linux/POSIX workstation.
 *  TODO: figure out what one char can store from an international keyboard, does the code assume it is human visible as a normal Alphanumeric character,
 *  then fix next two lines of documentation.
 */
using KbFundamentalUnit = char;
using Key_char_singular = KbFundamentalUnit;                // The most basic character that a keyboard can generate like ASCII or UNICODE 8? or 16? or 32?, which does not generate a multi-btye burst of characters, like F1
inline constexpr KbFundamentalUnit CHAR_NULL {'\0'};        // Value is unset/not-set, similar to how a SQL DB shows an unset field as NULL, which is different than zero length or some magic number.  Here we turn it into a magic number and hope for the best.

/** Is usally one (on a US Kb, but some keys probably have more than one on other language Kbs)
 *  normal/regular alphanumeric/ASCII like characters entered by the user.
 *  TODO: Should probably rename this to Kb_regular_keystroke .
 *  was?: Kb_regular_key .
 *  TODO: make this the correct/internationalized char type, which would be KbFundamentalUnit from above?? or what???.
*/
using Hot_key_chars 	= std::vector< KbFundamentalUnit >;   // A sequence of basic chars that are generated by a user single keypress of a Hot_key on a keyboard, ie. ESC, or F1 for help. but does not include a or 2 or '/', but NOTE they could also enter them manually.
using Key_chars_i18n 	= std::vector< KbFundamentalUnit >;   // One or a sequence of basic chars that are entered or generated by a user using international KB characters.

/** "Hot_key" is one keystroke of a ANSI keyboard of a SPECIAL key like "F1" or "Insert" or ESC or TAB, that is one or more chars/bytes long.
 *  Here a "row" represents relalated data of a Hot_key, like a row in a relational database.
 *  For multi-byte sequences that flow from one keystroke, it can start with a CSI_ESC or we allow for another other designated char being CSI_ALT.
 */
class Hot_key_row {
public:
  std::string          my_name            {STRING_NULL};                        // Name given by Lib_tty
  Hot_key_chars        characters         {STRING_NULL.cbegin(),STRING_NULL.cend()}; // See the type's documentation.
  HotKeyFunctionCat    function_cat       {HotKeyFunctionCat::initial_state};   // depending on this value, one or both of the following two data members are used. ::na is the case for ::job_control,::help_popup,::editing_mode,::other.
  InteractionIntentNav interaction_intent_nav {InteractionIntentNav::na};       // Gets a value if HotKeyFunctionCat::nav_field_completion, or HotKeyFunctionCat::navigation_esc
  FieldIntraNav        intra_f_nav        {FieldIntraNav::na};                  // Gets a value if HotKeyFunctionCat::nav_intra_field
  bool            operator<( Hot_key_row const &) const;                        // Used to sort the members of a table to enable easy algorithmic lookup by the characters field, within the table.
  std::string     to_string()                 const;                            // Used for debugging only.
}; using Hot_key_table = std::vector< Hot_key_row >;                            // Stores all known Hot_key_rows for internal library use.  Table like in the sense of a relational database.

/** A "Key_chars_i18n" is one keystroke of a international keyboard of a non-ascii key, that is one or more chars/bytes long.
 *  Here a "row" represents relalated data, like a row in a relational database.
 *  For multi-byte sequences that flow from one keystroke, it can start with a ???? or we allow for another other designated char being CSI_ALT.
 */
class Key_i18n_row {
public:
  std::string          my_name            {STRING_NULL};                        // Name given by Lib_tty
  Key_chars_i18n       characters         {STRING_NULL.cbegin(),STRING_NULL.cend()}; // See the type's documentation.
  //Keyi18nFunctionCat    function_cat       {HotKeyFunctionCat::initial_state};   // depending on this value, one or both of the following two data members are used. ::na is the case for ::job_control,::help_popup,::editing_mode,::other.
  InteractionIntentNav interaction_intent_nav {InteractionIntentNav::na};       //
  FieldIntraNav        intra_f_nav        {FieldIntraNav::na};                  //
  bool            operator<( Key_i18n_row const &) const;                 // Used to sort the members of a table to enable easy algorithmic lookup by the characters field, within the table.
  std::string     to_string()                 const;                            // Used for debugging only.
}; using Key_i18n_table = std::vector< Key_i18n_row >;              // Stores all known international chars/keystrokes for internal library use.  Table like in the sense of a relational database.

namespace exp1 {            //experimental NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
class Chars_error {
  Chars_error() = delete;     // TODO??: construction not allowed pure virtual without any functions.
};
class Hot_key_multi_char_error      : public Chars_error {
};
class I18n_multi_char_error         : public Chars_error  {
};

enum class Chars_error_e {
    initial_value,
    wrong_key,
    illegal_key,            // redundant?
    file_status             // redundant?
};

class Kb_get_result {
    //std::optional<Chars>multi_chars;   // TODO template stuff
    File_status         fs              {};                                     // status of the cin/file descriptor, ie EOF or BAD...
    bool                is_manual;                                              // entered by hand one char by one char
    bool                is_partial;                                             // incompletely entered by hand (or broken keyboard or our logic error)
    bool                is_error;                                               // redundant
    Chars_error_e       c_error;

    virtual std::string         to_string() const;                              //
};

class Key_char_singular_get_result            : public Kb_get_result {
    KbFundamentalUnit   kbf             {};
};                // The most basic character that a keyboard can generate like ASCII or UNICODE 8? or 16? or 32?, which does not generate a multi-btye burst of characters, like F1
class Hot_key_multi_char_get_result            : public Kb_get_result {  // TODO??: tag-dispatching type if empty body ie. {};
    Hot_key_row         hot_key_row     {};
};
class I18n_multi_char_get_result               : public Kb_get_result {
    Key_chars_i18n      characters      {};
};
class Hot_key_multi_char_manual_get_result     : public Hot_key_multi_char_get_result { // redundant // entered by hand one char by char
};
class I18n_multi_char_manual_get_result        : public I18n_multi_char_get_result {  // redundant
};
class I18n_multi_char_partial_get_result       : public I18n_multi_char_manual_get_result {  // redundant  // keys only partially entered
};
class Hot_key_multi_char_partial_get_result    : public I18n_multi_char_get_result {  // redundant
};

class Kb_get_result2 {
  std::variant<std::monostate, Key_char_singular_get_result, Hot_key_multi_char_get_result > r;
};
} // namespace NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

/** Is one char or one Hot_key in various forms,  ie. the result of hitting any key whether it is special or not OR EOF.
 *  TODO: An international i18n char could look like a hotkey since it might start with <ESC> or something else special, this is not yet accounted for in this code.
 *  TODO: char does this include an EOF character?
 */
using   Kb_key_variant =     std::variant< std::monostate, Key_char_singular, Key_chars_i18n, Key_i18n_row, Hot_key_row >;
using   Kb_key_row_variant = std::variant< std::monostate, Key_char_singular,                 Key_i18n_row, Hot_key_row >;

/** A return value of either a regular char(s) OR a Hot_key AND if we "are at"/"or got?" EOF.
 *  _a_ == "and"
 *  TODO: Need to rework the types/structs that contain Hot_key and other related values, there are TOO many similar ones.
*/
struct Kb_key_a_stati {
  Kb_key_variant    kb_key_variant       {std::monostate {}};                             // some datatype form of the key
  bool              is_had_partial_match {false};
  File_status       file_status          {File_status::initial_state};   /// holds what is happening with cin EOF
};

/** A return value which tells us if we got a Kb_key and?, or? a Hot_key, and, or?, if we got EOF. TODO?:
 *  Heavily used everywhere!
 *  TODO: Need to rework the types/structs that contain Hot_key and other related values, there are TOO many similar ones.
 */
struct Kb_value_plus_old {
  Key_chars_i18n    key_chars_i18n  {STRING_NULL.cbegin(),STRING_NULL.cend()};
  Hot_key_row       hot_key         {};
  File_status       file_status     {File_status::initial_state};
};
struct Kb_value_plus {  // XXXXX new
  std::vector<Kb_key_row_variant>   keystrokes      {std::monostate{}};
  bool                              is_partial      {false};
  File_status                       file_status     {File_status::initial_state};
};
/** Gets one single keystroke from user keyboard, which may consist of multiple characters in a key's multi-byte sequence
 *  Relies on cin being in raw mode!
 *  Called by get_kb_keystrokes_raw().
 *  PUBLIC FUNCTION could it be that also? Can be called by end user, but not used in the client "file_maintenance_*" programs.
 *  Probably needs much more debugging, if it is to be called directly. */
Kb_key_a_stati
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
 *    - more esoteric stuff that can be deferred for now. TODO:
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
    // We RETURN either N regular char(s), OR both the N regular char(s) and the latest Hot_key, OR just a Hot_key
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
