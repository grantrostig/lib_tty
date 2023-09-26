/*  Copyright (c) 2019 Grant Rostig all rights reserved,  grantrostig.com
 */

// TODO: rename .h to .hpp

#include <cstring>
#include <cstdio>
#include <deque>
#include <iomanip>
#include <iterator>
#include <variant>
#include <algorithm>
#include <iostream>
#include <type_traits>
#include <concepts>
#include <csignal>
#include <stacktrace>
#include <source_location>
// POSIX headers
#include <termios.h>
#include "lib_tty.h"
#include "lib_tty_internal.h"

using std::endl, std::cin, std::cout, std::cerr, std::string;
using namespace std::string_literals;

/// TODO??: better alternative? >using namespace Lib_tty; // or $Lib_tty::C_EOF // or $using Lib_tty::C_EOF will this last one work?
namespace Lib_tty {
/*****************************************************************************/
/***************** START Debugging only section ******************************/
/*****************************************************************************/
/// define if asserts are NOT to be checked.  Put in *.h file not *.CPP
//#define 	NDEBUG
/// define I'm Debugging LT.  Put in *.h file not *.CPP
#define  	GR_DEBUG
//#undef  	GR_DEBUG
//#ifdef   	GR_DEBUG
//#endif  # GR_DEBUG

string
source_loc( ) {  // give more detail on error location, used by perror()
    using loc = std::source_location;
    //using ts = std::to_string;  // TODO??: why not?  alternative approach?
    string result {"\n"s+loc::current().file_name() +":"s +std::to_string(loc::current().line()) +std::to_string(loc::current().column())+"]`"s +loc::current().function_name()+"`."s};
    return result;
}
/*****************************************************************************/
/********************** START of C++20 illustration **************************/
/*****************************************************************************/
///// Prints contents of a container such as a vector of int's.
///// Concept used by Templated Function definition
//template<typename Container>                        //template<insertable Container>        // OR these 2 lines currently being used.
//    requires Insertable<Container>
//std::ostream&
//operator<<( std::ostream & out, Container const & c) {
//    if ( not c.empty()) {
//        out << "[";   //out.width(9);  // TODO??: neither work, only space out first element. //out << std::setw(9);  // TODO??: neither work, only space out first element.
//        std::copy(c.begin(), c.end(), std::ostream_iterator< typename Container::value_type >(out, ","));
//        out << "\b]"; out.width(); out << std::setw(0);
//    } else out << "[CONTAINTER IS EMPTY]";
//    return out;
//}
/////  Concept using Function Explicit instantiations that are required to generate code for linker.
/////  TODO??: is the only used if definition is in *.cpp file?
/////  https://isocpp.org/wiki/faq/templates#templates-defn-vs-decl
/////  https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
//template std::ostream & operator<<( std::ostream & , std::vector<std::string> const & );
///// Concept using Function Explicit instantiations that are required to generate code for linker.
//template std::ostream & operator<<( std::ostream & , std::deque<int>          const & );
/*****************************************************************************/
/********************** END   of C++20 illustration **************************/
/*****************************************************************************/
///  Debugging use only at this time.
void print_signal(int const signal) {
    LOGGERS("Signal is:", signal);
    switch (signal) {
    //                  ISO C99 signals. see inital comments in lib_tty.h
    case ( SIGINT):		/* Interactive attention signal.  */
    case ( SIGILL):		/* Illegal instruction.  */
    case ( SIGABRT):	/* Abnormal termination.  */
    case ( SIGFPE):		/* Erroneous arithmetic operation.  */
    case ( SIGSEGV):	/* Invalid access to storage.  */
    case ( SIGTERM):	/* Termination request.  */
        /* Historical signalsspecified by POSIX. */
    case ( SIGHUP):		/* Hangup.  */
    case ( SIGQUIT):	/* Quit.  */
    case ( SIGTRAP):	/* Trace/breakpoint trap.  */
    case ( SIGKILL):	/* Killed.  */
    case ( SIGBUS):		/* Bus error.  */
    case ( SIGSYS):		/* Bad system call.  */
    case ( SIGPIPE):	/* Broken pipe.  */
    case ( SIGALRM):	/* Alarm clock.  */
        /* New(er) POSIX signals (1003.1-2008, 1003.1-2013).  */
    case ( SIGURG):		/* Urgent data is available at a socket.  */
    case ( SIGSTOP):	/* Stop, unblockable.  */
    case ( SIGTSTP):	/* Keyboard stop.  */
    case ( SIGCONT):	/* Continue.  */
    case ( SIGCHLD):	/* Child terminated or stopped.  */
    case ( SIGTTIN):	/* Background read from control terminal.  */
    case ( SIGTTOU):	/* Background write to control terminal.  */
    case ( SIGPOLL):	/* Pollable event occurred (System V).  */
    case ( SIGXCPU):	/* CPU time limit exceeded.  */
    case ( SIGXFSZ):	/* File size limit exceeded.  */
    case ( SIGVTALRM):	/* Virtual timer expired.  */
    case ( SIGPROF):	/* Profiling timer expired.  */
    case ( SIGUSR1):	/* User-defined signal 1.  */
    case ( SIGUSR2):	/* User-defined signal 2.  */
        /* Nonstandard signals found in all modern POSIX systems (including both BSD and Linux)  */
    case ( SIGWINCH):	/* Window size change (4.3 BSD, Sun).  */
    case ( SIGSTKFLT):	/* Stack fault (obsolete).  */
    case ( SIGPWR):		/* Power failure imminent.  */
        LOGGER_("Above is a typical POSIX signal");
        break;
    default:
        LOGGER_("Above is a non-typical POSIX signal");
    }
    /* also note: SIGRTMIN,SIGRTMAX
     * non signals
     SIG_ERR	 ((__sighandler_t) -1)	// Error return.
     SIG_DFL	 ((__sighandler_t)  0)	// Default action.
     SIG_IGN	 ((__sighandler_t)  1)	// Ignore signal.
     __USE_XOPEN
     SIG_HOLD ((__sighandler_t) 2)	// Add signal to hold mask.
    */
}

///  Debugging use only at this time.
std::optional<Hot_key>
find_hot_key(const Hot_key_table &hot_key_table, const Hot_key_chars this_key) {
    for (auto & hk : hot_key_table)
        if ( hk.characters == this_key )
            return hk;
    return {};
}
/*****************************************************************************/
/**************** END   Debugging only section ******************************/
/*****************************************************************************/
/**************** START POSIX level declarations *****************************/
/*****************************************************************************/
/********************** START POSIX OS Signals level declarations ************/
/*****************************************************************************/

Sigaction_termination_return
set_sigaction_for_termination( Sigaction_handler_fn_t handler_in) {  // TODO: why does const have no effect here?
    //LOGGER_ ("Starting.");
    struct sigaction action 		{};    			 // TODO:  TODO??: why is struct required by compiler.  TODO??: does this initialize deeply in c++ case?
    sigemptyset( &action.sa_mask );  				 // Isn't this already empty because of declaration? We do it just to be safe.
    action.sa_flags 				|= ~SA_SIGINFO;  // Invoke signal-catching function with three arguments instead of one.
    action.sa_flags 				|= ~SA_RESTART;  // Restart syscall on signal return.  TODO: not sure this is needed.
    action.sa_sigaction 			=  handler_in;

    // we could block certain signals here, but we choose not to in the case where we prompt the end-user for the password.
    // 		sigset_t block_mask; sigaddset(&block_mask, SIGINT and SIGQUIT); action.sa_mask = block_mask;  // SS 24.7.5 GNU libc manual

    // static, but why did I think that was a good idea? or was it needed? or was it needed by sigaction()? TODO?:
    static struct sigaction action_prior_SIGINT 	{};
    if ( sigaction( SIGINT  , nullptr, /*out*/ &action_prior_SIGINT ) == POSIX_ERROR) {  perror( source_loc().data()); exit(1); } // Just doing a get(), do setting next line
    if ( reinterpret_cast<void *>( action_prior_SIGINT.sa_handler )
         != reinterpret_cast<
                            void *
                            >(SIG_IGN)) {  // we avoid setting a signal on those that are already ignored. TODO: why is this different from My_sighandler_t?
        LOGGER_("SIGINT going to be set");
        if ( sigaction( SIGINT, &action, nullptr) == POSIX_ERROR ) { perror( source_loc().data()); exit(1); }
    }
    // ******* Repeat for other Signals.
        // TODO?: the "2nd if" should be about the same since I'm checking the same condition!!! Need to check the POSIX C code.
    static struct sigaction action_prior_SIGQUIT 	{};
    if ( sigaction( SIGQUIT , nullptr, /*out*/ &action_prior_SIGQUIT ) == POSIX_ERROR) { perror(source_loc().data()); exit(1); }  // just doing a get()
    if (                           action_prior_SIGQUIT.sa_sigaction  // ************ action, not sa_handler TODO debug/refactor
         != reinterpret_cast<
                            void(*) (int, siginfo_t *, void *)
                            >(SIG_IGN)) { // we avoid setting a signal on those that are already ignored.
        LOGGER_("SIGQUIT going to be set");
        if ( sigaction( SIGQUIT, &action, nullptr) == POSIX_ERROR ) { perror(source_loc().data()); exit(1); }  // TODO: Would this only be used if there was a serial line that could generate a HUP signal?
    }

    static struct sigaction action_prior_SIGTERM 	{};
    if ( sigaction( SIGTERM , nullptr, /*out*/ &action_prior_SIGTERM ) == POSIX_ERROR) { perror(source_loc().data()); exit(1); }  // just doing a get()
    if ( action_prior_SIGTERM.sa_sigaction != reinterpret_cast<void(*)(int, siginfo_t *, void *)>(SIG_IGN)) { // we avoid setting a signal on those that are already ignored.
        LOGGER_("SIGTERM going to be set");
        if ( sigaction( SIGTERM, &action, nullptr) == POSIX_ERROR ) { perror(source_loc().data()); exit(1); }  // TODO: Does not seem to work. BUG: Ctrl-\ causes a immediate dump.
    }

    static struct sigaction action_prior_SIGTSTP 	{};
    if ( sigaction( SIGTSTP , nullptr, /*out*/ &action_prior_SIGTSTP ) == POSIX_ERROR) { perror(source_loc().data()); exit(1); }  // just doing a get()
    if ( action_prior_SIGTSTP.sa_sigaction != reinterpret_cast<void(*)(int, siginfo_t *, void *)>(SIG_IGN)) { // we avoid setting a signal on those that are already ignored.
        LOGGER_("SIGTSTP going to be set");
        if ( sigaction( SIGTSTP, &action, nullptr) == POSIX_ERROR ) { perror(source_loc().data()); exit(1); }
    }

    static struct sigaction action_prior_SIGHUP 	{};
    if ( sigaction( SIGHUP , nullptr, /*out*/ &action_prior_SIGHUP ) == POSIX_ERROR) { perror(source_loc().data()); exit(1); }  // just doing a get()
    if ( action_prior_SIGHUP.sa_sigaction != reinterpret_cast<void(*)(int, siginfo_t *, void *)>(SIG_IGN)) { // we avoid setting a signal on those that are already ignored.
        LOGGER_("SIGHUP going to be set" );
        if ( sigaction( SIGHUP, &action, nullptr) == POSIX_ERROR ) { perror(source_loc().data()); exit(1); }
    }
    return { action_prior_SIGINT, action_prior_SIGQUIT, action_prior_SIGTERM, action_prior_SIGTSTP, action_prior_SIGHUP };
}

// forward declaration
Lib_tty::Sigaction_return
set_sigaction_for_inactivity( Sigaction_handler_fn_t handler_in );

/// signal handler function to be called when a timeout alarm goes off via a user defined signal is received
void handler_inactivity(int const sig, Siginfo_t *, void *) {  // TODO: why can't I add const here without compiler error?
    LOGGERS("This signal got us here:", sig);
    print_signal( sig );
    set_sigaction_for_inactivity( handler_inactivity );  //  re-create the handler we are in so we can get here again when handling is called for!?!? // TODO: WHY, since it seems to have been used-up/invalidated somehow?
    // TODO: ?? raise (sig);
    // TODO: ?? signal(sig, SIG_IGN);
}

void handler_termination(int const sig, Siginfo_t *, void *) {
    LOGGERS("This signal # got us here:", sig);
    print_signal( sig );
    set_sigaction_for_termination( handler_termination );  // re-create the handler we are in so we can get here again when handling is called for!?!? // TODO: WHY, since it seems to have been used-up/invalidated somehow?
}

/// put signal handling back? TODO:
void sigaction_restore_for_termination( Sigaction_termination_return const & actions_prior ) {
    if (sigaction( SIGINT,  &actions_prior.action_prior1, nullptr) == POSIX_ERROR ) { perror(source_loc().data()); exit(1); }
    else {LOGGER_("SIGINT set to original state" )};
    if (sigaction( SIGQUIT, &actions_prior.action_prior2, nullptr) == POSIX_ERROR ) { perror(source_loc().data()); exit(1); }
    else {LOGGER_("SIGQUIT set to original state" )};
    if (sigaction( SIGTERM, &actions_prior.action_prior3, nullptr) == POSIX_ERROR ) { perror(source_loc().data()); exit(1); }
    else {LOGGER_("SIGTERM set to original state" )};
    if (sigaction( SIGTSTP, &actions_prior.action_prior4, nullptr) == POSIX_ERROR ) { perror(source_loc().data()); exit(1); }
    else {LOGGER_("SIGTSTP set to original state" )};
    if (sigaction( SIGHUP,  &actions_prior.action_prior5, nullptr) == POSIX_ERROR ) { perror(source_loc().data()); exit(1); }
    else {LOGGER_("SIGHUP set to original state" )};
    return;
}

/// called by enable_inactivity_handler
Lib_tty::Sigaction_return
set_sigaction_for_inactivity( Sigaction_handler_fn_t handler_in ) {
    struct sigaction action {};
    sigemptyset( &action.sa_mask );
    action.sa_flags 	|= ~SA_SIGINFO;  // Invoke signal-catching function with three arguments instead of one.
    action.sa_flags 	|= ~SA_RESTART;  // Restart syscall on signal return.
    action.sa_sigaction	=   handler_in;  // TODO:  TODO: recursive macro is disabled by compiler?

    // we could block certain signals here, but we choose not to in the case where we prompt the use for the password.
    // sigset_t block_mask; sigaddset(&block_mask, SIGINT and SIGQUIT); action.sa_mask = block_mask;  // SS 24.7.5 GNU libc manual

    int 				signal_for_user  	 { SIGRTMIN }; /// from Linux source code: Return number of available real-time signal with highest priority.  */
    struct 	sigaction 	action_prior {};
    LOGGERS("Signal going to be set is int SIGRTMIN, which is:", signal_for_user);
    if ( sigaction( signal_for_user , &action, /* out */ &action_prior ) == POSIX_ERROR )
    {
        perror(source_loc().data()); exit(1);
    }
    return { signal_for_user, action_prior};
}

/** Not used in file_manager, so not sure it has any use beyond test program called: ../cpp_by_example/lib_tty_posix_signal_timer_settime_periodic/main.cpp:35:
 *  Called internally every time the timer interval needs to be reset to wait for the full time. In other words after we get a character, we start waiting all over again.
 *  Called by enable_inactivity_handler.
 *  Initially and every time the timer interval needs to be reset to wait for the full time,
 *  even if it had not expired.
 *  In other words after we get a character, we start waiting all over again.
void set_a_run_inactivity_timer(const timer_t & inactivity_timer, const int seconds) {  // TODO: is timer_t a pointer or NOT or struct or int?!?
    int settime_flags 						{};				// TIMER_ABSTIME is only flag noted. Not used here.
    static itimerspec timer_specification 	{};				// TODO: does this need to be static, ie. what does timer_settime() do with it?
    timer_specification.it_value.tv_sec 	= seconds;
    timer_specification.it_interval.tv_sec 	= timer_specification.it_value.tv_sec;  // interval is same value. TODO: BUG: buggy behaviour when interval is very short ie. 1000 or long 1000000.
    if ( timer_settime( inactivity_timer, settime_flags, &timer_specification, / out / nullptr) == POSIX_ERROR) { perror(source_loc().data()); exit(1); }
    return;
}
/// Configures and sets timer and runs timer which waits for additional user kb characters.  Timer needs to be deleted when no longer wanted.
/// Not used in file_manager, so not sure it has any use beyond test program called: ../cpp_by_example/lib_tty_posix_signal_timer_settime_periodic/main.cpp:35:
/// TODO: consider replacing std::tuple/std::pair with struct!
std::tuple<timer_t &, int, struct sigaction>
enable_inactivity_handler(const int seconds) {
    static sigevent inactivity_event  {};
    inactivity_event.sigev_notify	= SIGEV_SIGNAL;  // Notify via signal.
    inactivity_event.sigev_signo  	= SIGRTMIN;	   // Return number of available real-time signal with highest priority.
    // grostig TODO bug? auto [sig_user, action_prior] 	= set_sigaction_for_inactivity( handler_inactivity );
    Sigaction_return result = set_sigaction_for_inactivity( handler_inactivity );

    static timer_t inactivity_timer   {};
    if ( timer_create( CLOCK_REALTIME, &inactivity_event, / out / &inactivity_timer) == POSIX_ERROR) { perror(source_loc().data()); exit(1); }  // TODO: address of a pointer? seriously ptr to ptr?
    set_a_run_inactivity_timer( inactivity_timer, seconds );
    // grostig TODO bug? return { inactivity_timer, sig_user, action_prior };
    return { inactivity_timer, result.signal_for_user, result.action_prior };
} */

/// TODO:verify> stop waiting for additional keyboard characters from the user? delete the CSI/ESC timer
void disable_inactivity_handler(const timer_t inactivity_timer, const int sig_user, const struct sigaction old_action) {
    LOGGERS("disable inactivity handler on signal: ", sig_user);
    if ( timer_delete( inactivity_timer) 			    == POSIX_ERROR) { perror( source_loc().data()); exit(1); } // should print out message based on ERRNO // TODO: fix this up. TODO: this throws in c lang???
    if ( sigaction(    sig_user, &old_action, nullptr) 	== POSIX_ERROR) { perror( source_loc().data()); exit(1); } // should print out message based on ERRNO // TODO: fix this up.  TODO: __THROW ???
}

/*****************************************************************************/
/********************** END   POSIX OS signals level definitions *************/
/********************** START POSIX OS termios level definitions *************/
/*****************************************************************************/
/// to show what is happening on standard-in/cin. Used for debugging. TODO??: how do I pass in cin or cout to this?
void print_iostate(const std::istream &stream) {
    LOGGER_("Is:");
    if (stream.rdstate() == std::ios_base::goodbit) {LOGGER_("goodbit only, ")};
    if (stream.rdstate() &  std::ios_base::goodbit) {LOGGER_("goodbit, ")};
    if (stream.rdstate() &  std::ios_base::eofbit)  {LOGGER_("eofbit, ")};
    if (stream.rdstate() &  std::ios_base::failbit) {LOGGER_("failbit, ")};
    if (stream.rdstate() &  std::ios_base::badbit)  {LOGGER_("badbit,")};
}

/// created as a "hack" since operator== doesn't work reliably? on structs.
bool check_equality(Lib_tty::Termios const &termios, Lib_tty::Termios const &termios2){  // Used for debugging using assert().
    /* https://embeddedgurus.com/stack-overflow/2009/12/effective-c-tip-8-structure-comparison/
     * https://isocpp.org/blog/2016/02/a-bit-of-background-for-the-default-comparison-proposal-bjarne-stroustrup
     * https://stackoverflow.com/questions/141720/how-do-you-compare-structs-for-equality-in-c
     * https://stackoverflow.com/questions/46995631/why-are-structs-not-allowed-in-equality-expressions-in-c
     * https://stackoverflow.com/questions/7179174/why-doesnt-c-provide-struct-comparison/47056810#47056810
     * if ( termios.c_iflag != termios2.c_iflag
        && termios.c_oflag  != termios2.c_oflag
        && termios.c_cflag  != termios2.c_cflag
        && termios.c_lflag  != termios2.c_lflag
        && termios.c_line   != termios2.c_line
        && termios.c_ispeed != termios2.c_ispeed
        && termios.c_ospeed != termios2.c_ospeed ) return false;
    for (int i=0; i< NCCS; ++i)
        if (termios.c_cc[i] != termios2.c_cc[i]) return false; */
    // Apparently this is inreliable, but perhaps with this simple datastructure it will work on most/all machines?
    if ( std::memcmp( &termios, &termios2, sizeof termios) != 0) return false;  // does not set ERRNO.  TODO:?? Are Termios structures initialized, including the possible padding when defined as below?  How does c++ know the padding that the Linux kernel expects in a system call?
    return true;
}

Lib_tty::Termios & termio_get() { // uses POSIX  // TODO TODO: what are advantages of other version of this function?
    static Termios termios;
    if (auto result = tcgetattr( fileno(stdin), &termios); result == POSIX_ERROR) { // TODO: throw() in signature?
        int errno_save = errno;
        LOGGERS("Standard-in is not a tty keyboard??",errno_save);
        errno = errno_save;
        perror( source_loc().data() );
        LOGGER_("We will exit(1) for this error");
        exit(1);
    }
    return termios;  // copy of POD?
}

/// We set POSIX terminal with Termios control attributes.
/// Applications that need ALL of the requested changes to work properly should follow tcsetattr()
/// with a call to tcgetattr() and compare then appropriate field values.
/// We do the check after this call within the caller.  TODO consider doing it in here.
/// TODO not called yet, refactor for 3 uses.
void termio_set( Termios const & termios_new ) { // uses POSIX
    if ( auto result = tcsetattr( fileno(stdin), TCSADRAIN, /*IN*/ &termios_new );
        result == POSIX_ERROR ) {
        int errno_save = errno;
        LOGGERS("Standard in is not a tty keyboard with errno of:", errno_save);
        errno = errno_save;
        perror( source_loc().data() );
        exit(1);
    }
    Termios const termios_current { termio_get() };
    assert( check_equality( termios_new, termios_current ) && "Tcsetattr failed at least partially." );
};

Lib_tty::Termios & termio_set_raw() { // uses POSIX
    cin.sync_with_stdio( false );  									// TODO:  iostreams bug?  This is required for timer time-out else a bug occurs.
    static Termios termios_orig { termio_get() };
    Termios 	   termios_new 	{ termios_orig };                   // https://www.gnu.org/software/libc/manual/html_mono/libc.html#Mode-Data-Types
        // /usr/include/sys/ttydefaults.h
        // /usr/include/bits/termios-c_lflag.h
        // local modes
    termios_new.c_lflag 		&= static_cast<tcflag_t>(~ISIG);    // TODO?: turn off ???  // want this bit ON for cbreak mode.
    termios_new.c_lflag 		&= static_cast<tcflag_t>(~ICANON);  // turn off "canonical" or "cooked" mode and go to "non-canonical" or "raw" mode, ie. don't wait for <Enter>. // want this bit OFF for cbreak mode.
    termios_new.c_lflag 		&= static_cast<tcflag_t>(~ECHO);    // turn off "echo" mode, ie. don't automatically show the characters being typed. // want this bit OFF for cbreak mode.
    //termios_new.c_lflag 		&= static_cast<tcflag_t>(~ECHOE);     // TODO?: not touched?
    //termios_new.c_lflag 		&= static_cast<tcflag_t>(~ECHOK);     // TODO?: not touched?
    //termios_new.c_lflag 		&= static_cast<tcflag_t>(~ECHONL);    // TODO?: not touched?
    //termios_new.c_lflag 		&= static_cast<tcflag_t>(~ECHOCTL);   // TODO?: not touched?
    //termios_new.c_lflag 		&= static_cast<tcflag_t>(~ECHOPRT);   // TODO?: not touched?
    //termios_new.c_lflag 		&= static_cast<tcflag_t>(~ECHOKE);    // TODO?: not touched?
    //termios_new.c_lflag 		&= static_cast<tcflag_t>(~FLUSHO);    // TODO?: not touched?
    //termios_new.c_lflag 		&= static_cast<tcflag_t>(~NOFLSH);    // TODO?: not touched?
    //termios_new.c_lflag 		&= static_cast<tcflag_t>(~PENDIN);    // TODO?: not touched?
    //termios_new.c_lflag 		&= static_cast<tcflag_t>(~TOSTOP);    // TODO?: not touched?
    termios_new.c_lflag 		&= static_cast<tcflag_t>(~IEXTEN);  // TODO?: turn off ??? maybe not!
    //termios_new.c_lflag 		&= static_cast<tcflag_t>(~EXTPROC);   // TODO?: not touched?
    //termios_new.c_lflag 		&= static_cast<tcflag_t>(~XCASE);     // TODO?: not touched?
        // /usr/include/bits/termios-c_iflag.h
        // local modes
    termios_new.c_iflag 		&= static_cast<tcflag_t>(~IGNBRK);  // TODO?: turn off ???
    termios_new.c_iflag 		&= static_cast<tcflag_t>(~BRKINT);  // TODO?: turn off ???
    //termios_new.c_iflag 		&= static_cast<tcflag_t>(~IGNPAR);    // TODO?: not touched?
    termios_new.c_iflag 		&= static_cast<tcflag_t>(~PARMRK);  // TODO?: turn off ???
    termios_new.c_iflag 		&= static_cast<tcflag_t>(~INPCK);   // TODO?: turn off ???
    termios_new.c_iflag 		&= static_cast<tcflag_t>(~ISTRIP);  // TODO?: turn off ???
    termios_new.c_iflag 		&= static_cast<tcflag_t>(~INLCR);   // TODO?: turn off ???
    termios_new.c_iflag 		&= static_cast<tcflag_t>(~IGNCR);   // TODO?: turn off ???
    termios_new.c_iflag 		&= static_cast<tcflag_t>(~ICRNL);   // TODO?: turn off ???  // want this bit OFF for cbreak mode.
    //termios_new.c_iflag 		&= static_cast<tcflag_t>(~IUCLC);     // TODO?: not touched?
    termios_new.c_iflag 		&= static_cast<tcflag_t>(~IXON);    // TODO?: turn off ???
    //termios_new.c_iflag 		&= static_cast<tcflag_t>(~IXANY);     // TODO?: not touched?
    //termios_new.c_iflag 		&= static_cast<tcflag_t>(~IXOFF);     // TODO?: not touched?
    //termios_new.c_iflag 		&= static_cast<tcflag_t>(~IMAXBEL);   // TODO?: not touched?
    //termios_new.c_iflag 		&= static_cast<tcflag_t>(~IUTF8);     // TODO?: not touched?
        // /usr/include/bits/termios-c_oflag.h
        // output modes
    termios_new.c_oflag 		&= static_cast<tcflag_t>(~OPOST);   // turn off all post-process output processing. NOTE: there are about 20 more that are not touched, decided not to list them in comments, since I don't thinkn they are needed.
        // /usr/include/bits/termios-c_cc.h
        // http://www.unixwiz.net/techtips/termios-vmin-vtime.html
        // not sure what to call these c? cc? what are they?
    termios_new.c_cc[VTIME] 	= 0; 								// wait forever to get the next char.  //NOTE: there are about 15 more that are not touched, decided not to list them in comments, since I don't thinkn they are needed.
    termios_new.c_cc[VMIN]  	= 1;  								// get minimun one char
    termio_set( termios_new );
    return termios_orig;
}

void termio_restore( Lib_tty::Termios const &termios_orig) { // uses POSIX  // TODO: do you like my const 2x, what is effect calling POSIX?
    termio_set( termios_orig );
    cin.sync_with_stdio(true);  // TODO:  iostreams bug?  This is required for timer time-out bug occurs.
    return;
}

Lib_tty::Termios &
termio_set_timer( cc_t const time) {  // uses POSIX
    static Termios termios_orig { termio_get() }; // TODO: why does this compile with termios and &termios?
    Termios termios_new = termios_orig;
    cin.sync_with_stdio(false);  // TODO:  iostreams bug?  This is required for timer time-out bug occurs.
    termios_new.c_cc[VTIME] = time;  // wait some time to get that char
    termios_new.c_cc[VMIN]  = 0;  // no minimum char to get
    termio_set( termios_new );
    return termios_orig;
}

/*****************************************************************************/
/********************** END   POSIX OS termios level definitions *************/
/**************** END   POSIX level definitions ******************************/
/*****************************************************************************/
/**************** START Lib_tty specific code ********************************/
/*****************************************************************************/

/********** START Hot_key Class specific code ********************************/
/// Free function.  //TODO??: or should this be a member function? Why?
bool is_hk_chars_equal( Hot_key const & left, Hot_key const & right ) {
    return ( left.characters == right.characters );
}

bool Hot_key::operator< ( Hot_key const  & in ) const {
    return ( characters < in.characters );
}

std::string
Hot_key::to_string() const {  // found in lib_tty.h
    std::string s {my_name};  // TODO: finish this
    return s;
}
/********** END   Hot_key Class specific code ********************************/

/// Allows eof on fd, but aborts on all other stati.
/// Utility function called only twice within lib_tty.
File_status
get_successfull_iostate_cin() {
    File_status file_status {File_status::initial_state};
    if ( cin.eof() ) {
        file_status = File_status::eof_file_descriptor;
        LOGGER_("CIN is eof")
    } else {
        if ( cin.fail() ) {
            file_status = File_status::fail;
            assert( false && "cin is bad() how did that happen?  We don't handle it." );
        } else {
            if ( cin.bad() ) {
                file_status = File_status::bad;
                assert( false && "cin is bad() how did that happen?  We don't handle it." );
            } else {
                file_status = File_status::good; // grostig
                // TODO?: might be good, but what if a timed get timed out with an alarm signal? Needs to be checked and possible reworking of logic.
                LOGGER_("Cin is good.")
                assert( cin.good() && "Logic error:Should be good by exclusion.");
            }
        }
    }
    assert( file_status != File_status::initial_state && "Postcondition8: this should have been modified.");
    return file_status;
}

/// File_status is not acceptable for the Kb_key_a_fstat we got, so result is an error.
/// The question is can we recover?
bool is_adequate_file_status( File_status const file_status )  { // **** CASE on File Status
    static_assert( std::is_enum_v< File_status > , "Precondition/Logic_error, should be an enum class.");
    switch (file_status) {
    case File_status::good :                        LOGGER_("File_status is: good"); // TODO fix relationship to "other"
        return true;
    case File_status::other_user_kb_char_data_HACK :LOGGER_("File_status is: other"); //
        return true;
    //case File_status::eof_Key_char_singular :       LOGGER_("File_status is: keyboard eof, which is a hotkey"); //
        //return true;
    //case File_status::eof_library :                 LOGGER_("File_status is: keyboard eof, which is a hotkey"); //
        //return true;
    case File_status::unexpected_user_kb_char_data_HACK :
        cout << "\ais_ignore_key_file_status: bad keyboard character sequence, try again."; // we throw away bad character sequence or char // TODO: handle scrolling and dialog
        break;
    case File_status::timed_out :       // handles case where the user is hand typing in the CSI hot_key.
        cout << "\ais_ignore_key_file_status: keyboard timeout, try again.";
        break;
    case File_status::eof_file_descriptor :
        assert( false && "File descriptor is at eof.");  // TODO: is this correct, or should we not ignore it?
        break;
    case File_status::bad :
        LOGGER_("File_status is bad"); //
        assert( false && "File descriptor is bad.");  // TODO: is this correct, or should we not ignore it?
        break;
    case File_status::fail :
        LOGGER_("File_status is fail"); //
        assert( false && "File descriptor is fail.");  // TODO: is this correct, or should we not ignore it?
        break;
    case File_status::initial_state :
        assert( false && "File_status should be set by now.");  // TODO: is this correct, or should we not ignore it?
        break;
    }
    return false;  // we will add back the input character that we have decided to ignore.
}

/// give it the string "EOF" and you get back 4 or ^D */
char find_posix_char_from_posix_name(const Ascii_Posix_table &vec, const std::string name) {
    for (auto & ch : vec) {
        if ( ch.posix_id == name )
            return ch.posix_char;
    }
    std::string err_message {"Bad name for a keyboard key, it being:"+name+"."};
    assert( false && err_message.c_str());
    // we never get here.
}

/** Determines if the parameter is a hot_key, or contains the initial characters of a hot_key.
 *  Is called repetitively by caller to incrementally check the accumulated chars that are input from a single keystroke of the user.
 *  Handles both single char hot_keys and multi-byte sequence hot_keys.
 *  Only used internally to lib_tty. */
Hotkey_o_errno
consider_hot_key( Hot_key_chars const & candidate_hk_chars ) {
    assert( not candidate_hk_chars.empty() && "Precondition.");
    /* https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/termios.h.html
       https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap11.html#tag_11_01_09
       http://osr600doc.xinuos.com/en/SDK_sysprog/TDC_SpecialChars.html
       https://www.gnu.org/software/bash/manual/html_node/Commands-For-Killing.html NOTE: this link provides the GNU unix "OR" statements regarding the meaning/use of some CTRL characters below.  It also cover some M-? characters, I presume that means "meta key"?
       /usr/include/sys/ttydefaults.h
       more of the above, not placed in table below: https://www.gnu.org/software/bash/manual/html_node/Commands-For-Text.html
       stty -a $ // intr = ^C; quit = ^\; erase = ^?; kill = ^U; eof = ^D; eol = <undef>; eol2 = <undef>; swtch = <undef>; start = ^Q; stop = ^S; susp = ^Z; rprnt = ^R; werase = ^W; lnext = ^V; discard = ^O; min = 1; time = 0;

       https://www.fysh.org/~zefram/keyboard/future_keyboard.txt https://www.fysh.org/~zefram/ttyerase/
        * more punctuation characters and the Ctrl modifier, for typing the full
        ASCII character set (including unprintables)
        * communications control keys such as Break (remember when your terminal
        and host were separate entities?)
        * terminal feature keys, such as cursor keys
        * programmable function keys
        * the numeric keypad of an adding machine, because it's a better layout
        for typing numeric data
        * editor function keys
        * more modifier keys: Alt, Option, Meta, Hyper, Super
        * the Windows keys
        * power management keys
        * web browser shortcut keys
        * audio playback control keys
        After removing all of the completely unusable control characters, it
        turns out that Ctrl could only reliably be used with twenty of the forty
        basic keys (A, B, C, D, E, F, G, K, L, N, O, P, R, T, U, V, W, X, Y, Z).
     */
    static const Ascii_Posix_table ascii_posix_map {                            // TODO??: why is constexpr allowed, is it the vector?
    //ascii_id, ascii_name,          posix_id, ascii_ctrl+, \+,  ascii_char, posix_char
        {"NUL", "Null",					"EOL", 		'@',	0,		0,	0},   // [ctl-spacebar does this!] (typically, ASCII NUL) is an additional line delimiter, like the NL character. EOL is not normally used. // If ICANON is set, the EOL character is recognized and acts as a special character on input (see ``Local modes and line disciplines'' in termio(M)).  probably of not used or even available as a char from a PC or any type of keyboard?!
        {"SOH",	"start_of_heading",		"???", 	    'a',	0,		1,	1},   //
        {"STX",	"start_of_text",		"???",	 	'b',	0,		2,	2},   //
        {"ETX",	"end_of_text",			"INTR", 	'c',	0,		3,	3},   // interrupt/stop the program / paste (from copy-paste)?/-SIGINT
        {"EOT",	"end_of_transmission",	"EOF",		'd',  	0, 	4,	4},   // I perfer to call this "end of file", apparently this is a ascii character char when in POSIX raw mode tty.
        {"ENQ",	"Enquiry",				"???",		'e',  	0, 	5,	5},   //
        {"ACK",	"Acknowledgement",		"???",		'f',  	0, 	6,	6},   //
        {"BEL",	"bell",					"BELL?",	'g', 	'a',	7,	7},   // TODO: NOT IMPLEMENTED in hot_key_table // Ctrl-g = \a  alert?
        {"BS",	"backspace",		 	"BS",		'h',	'b',	8,	8},   // BS alternative, how related to tty erase? TODO: NOT IMPLEMENTED in hot_key_table // character erase character works on linux and DOS? Backspace on MacOS use DEL for ERASE?
        {"HT",	"horizontal_tab",		"TAB",		'i',	't',	9,	9},
        {"LF",	"line_feed",			"NL-LF",	'j',	'n',	10,	10},  // end the line. newline. NL posix normal line delimiter. On linux with ANSI keyboard this is "Enter" key, was "Return" on typewriters and old PCs? What about "CR"
                                                                              // EOL/EOL2 from stty is \r\n or 13, 10, or CR,LF  https://www.ni.com/en-us/support/documentation/supplemental/21/labview-termination-characters.html
        {"VT",	"vertical_tab",			"VT?",		'k',	'v',	11,	11},  // TODO: NOT IMPLEMENTED in hot_key_table  // cursor down one line, like what one would call LF Line feed.
                                                                              // OR kill-line (C-k): Kill the text from point to the end of the line. With a negative numeric argument, kill backward from the cursor to the beginning of the current line.
        {"FF",	"form_feed",			"FF?",		'l',	'f',	12,	12},  // TODO: NOT IMPLEMENTED in hot_key_table // redisplay page.
        {"CR",	"carriage_return",		"CR",		'm',	'r',	13,	13},  // CR note the 'r', end the line. DOS style LF/NL, but is two characters: \n\r?
        {"SO",	"shift_out",			"???",		'n',	0,		14,	14},  //
        {"SI",	"shift_in",				"DISCARD",	'o',	0,		15,	15},  //
        {"DLE",	"data link escape",		"???",		'p',	0,		16,	16},  //
        {"DC1",	"device_control_1/XON",	"START",	'q',	0,		17,	17},  // not usable, TODO: NOT IMPLEMENTED in hot_key_table // resume tty output  IXON is a tty setting not a character!
        {"DC2",	"device_control_2",		"REPRINT",	'r',	0,		18,	18},  // TODO: NOT IMPLEMENTED in hot_key_table // redraw the current line.
        {"DC3", "device_control_3/XOFF","STOP",		's',	0,		19,	19},  // not usable, TODO: NOT IMPLEMENTED in hot_key_table // suspend tty output IXOFF is a tty setting not a character!
        {"DC4", "device_control_4",		"STATUS",	't',	0,		20,	20},  // TODO: NOT IMPLEMENTED in hot_key_table // on macOS and BSD.
        {"NAK",	"neg. acknowledge OR line_erase",
                                        "KILL",		'u',	0,		21,	21},  // TODO: NOT IMPLEMENTED in hot_key_table // deletes entire line being typed. TODO: "line erase character" kills the current input line when using the POSIX shell?
                                                                              // OR TODO? C-x??: backward-kill-line (C-x Rubout): Kill backward from the cursor to the beginning of the current line. With a negative numeric argument, kill forward from the cursor to the end of the current line.
        {"SYNC","synchronous_idle",		"LNEXT",	'v',	0,		22,	22},  // TODO: NOT IMPLEMENTED in hot_key_table // paste (from copy-paste)?/
        {"ETB",	"end_of_tranmission_block",
                                        "WERASE",   'w',	0,		23,	23},  // TODO: NOT IMPLEMENTED in hot_key_table // erase the last word typed.
                                                                              // OR unix-word-rubout (C-w): Kill the word behind point, using white space as a word boundary. The killed text is saved on the kill-ring.
        {"CAN",	"cancel",				"CANCEL?",	'x',	0,		24,	24},  // TODO?: C-u??? TODO: NOT IMPLEMENTED in hot_key_table // cancel the input line? / cut (from copy-paste)? /
        {"EM",	"end_of_medium",		"???",		'y',	0,		25,	25},  // TODO: NOT IMPLEMENTED? in hot_key_table // OR yank (C-y): Yank the top of the kill ring into the buffer at point.
        {"SUB",	"substitute",			"SUSP",		'z',	0,		26,	26},  // TODO: NOT IMPLEMENTED in hot_key_table //send a terminal stop signal. posix job-control: suspend process.  note: use "fg" to bring to foreground, "bg" to make it run in background.  AKA SIGSTOP?-SIGTSTP
        {"ESC",	"escape",				"ESC",		'[',	'e',	27,	27},  // ESC key, the first char of POSIX CSI Control Sequence Introducer
        {"FS",	"file_separator",		"QUIT",		'\\',	0,		28,	28},  // posix - nothing!?!, unix job-control: quit process and create a "core file". TODO: WARNING: may include current contents of memory??-SIGQUIT
        {"GS",	"group_separator",		"???",		']',	0,		29,	29},  //
        {"RS",	"record_separator",		"???",		'^',	0,		30,	30},  //
        {"US",	"unit_separator",		"???",		'_',	0,		31,	31},  //
        {"DEL",	"delete",				"ERASE",	'?',	0,	   127,127},  // BS alternative, how related to tty erase?. erase the last character typed. similar to "BS" // TODO: NOT IMPLEMENTED in hot_key_table //  BS/ERASE on MacOS?

        {" ",	"space",				"???",	    0,		' ',    32, 32},  // Simple space character or blank. TODO: why is this here with special chars?
        {"\\",	"backslash",			"???",		0,		0x5C,   92, 92},  // Simple character. TODO: why is this here with special chars?
    };
    //static_assert( not ascii_posix_map.empty() );  TODO??: how to make this consexpr, and would it benefit size or speed of program?
    static       Hot_key_table 		 hot_key_table {
        // my_name,     char sequence AKA characters,                                                   Cat,                                        Nav,                                    IntraNav
        // first the single key char action keys that are the good old Unix shell standard.
        {"escape",		{find_posix_char_from_posix_name(ascii_posix_map, "ESC"),
                         NO_MORE_CHARS},                                                  HotKeyFunctionCat::navigation_esc,        FieldCompletionNav::esc,			 	FieldIntraNav::na},
                            // TODO?: do we need NO_MORE_CHARS? or do other single character hot_keys like <TAB>
        {"eof",			{find_posix_char_from_posix_name(ascii_posix_map, "EOF")},        HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::eof,			 	FieldIntraNav::na},
        {"quit",		{find_posix_char_from_posix_name(ascii_posix_map, "QUIT")}, 	  HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::quit_signal,		FieldIntraNav::na},
        {"interrupt",	{find_posix_char_from_posix_name(ascii_posix_map, "INTR")},       HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::quit_signal,		FieldIntraNav::na},

        {"tab",			{find_posix_char_from_posix_name(ascii_posix_map, "TAB")},        HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::skip_one_field, 	FieldIntraNav::na},

        {"CR_aka_CTL-M", {find_posix_char_from_posix_name(ascii_posix_map, "CR")},        HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::down_one_field, 	FieldIntraNav::na},
        {"LF_aka_CTL-J", {find_posix_char_from_posix_name(ascii_posix_map, "NL-LF")},     HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::down_one_field, 	FieldIntraNav::na},

        {"kill",		  {find_posix_char_from_posix_name(ascii_posix_map, "KILL")},     HotKeyFunctionCat::nav_intra_field,		FieldCompletionNav::na,		 			FieldIntraNav::kill},
        {"backspace_left_erase", {find_posix_char_from_posix_name(ascii_posix_map, "BS")},HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na,					FieldIntraNav::backspace_left_erase},
        {"erase_left",    {find_posix_char_from_posix_name(ascii_posix_map, "ERASE")},    HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na,					FieldIntraNav::erase_left},  // TODO: is this correct for macOS or Linux or PC??

        // Secondly the multicharacter ESC sequences for the XTERM initially and then VT100 or ANSI.SYS? keyboard, which might be different as in "termcap" on some other hardware.
        // TODO: Make sure I have ALL keystrokes from USA pc keyboard.  XTERM https://en.wikipedia.org/wiki/ANSI_escape_code#Terminal_input_sequences
        {"f1",			{CSI_ESC,'O','P'}, 				HotKeyFunctionCat::help_popup, 				FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f2",			{CSI_ESC,'O','Q'}, 				HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f3",			{CSI_ESC,'O','R'}, 				HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f4",			{CSI_ESC,'O','S'}, 				HotKeyFunctionCat::nav_field_completion,    FieldCompletionNav::exit_with_prompts, FieldIntraNav::na},
        {"up_arrow",	{CSI_ESC,'[','A'}, 				HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::browse_up, 		FieldIntraNav::na},
        {"down_arrow",	{CSI_ESC,'[','B'}, 				HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::browse_down, 	FieldIntraNav::na},
        {"right_arrow",	{CSI_ESC,'[','C'}, 				HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::move_right},
        {"left_arrow",	{CSI_ESC,'[','D'}, 				HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::move_left},
        {"end",			{CSI_ESC,'[','F'}, 				HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::goto_end},
        {"key_pad_5",	{CSI_ESC,'[','G'}, 				HotKeyFunctionCat::none,			 		    FieldCompletionNav::na, 			FieldIntraNav::na},
        {"home",		{CSI_ESC,'[','H'}, 				HotKeyFunctionCat::nav_intra_field,			FieldCompletionNav::na,			 	FieldIntraNav::goto_begin},
        {"shift-tab",	{CSI_ESC,'[','Z'}, 				HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::up_one_field, 	FieldIntraNav::na},
        // VT100/220
        {"insert",		{CSI_ESC,'[',    '2','~'}, 		HotKeyFunctionCat::editing_mode,			FieldCompletionNav::na,			 	FieldIntraNav::na},
        {"delete",		{CSI_ESC,'[',    '3','~'}, 		HotKeyFunctionCat::nav_intra_field,			FieldCompletionNav::na, 			FieldIntraNav::delete_char},
        {"pageup",	  	{CSI_ESC,'[',    '5','~'}, 		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::page_up,		FieldIntraNav::na},
        {"pagedown",	{CSI_ESC,'[',    '6','~'}, 		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::page_down,		FieldIntraNav::na},
        {"f5",			{CSI_ESC,'[','1','5','~'}, 		HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f6",			{CSI_ESC,'[','1','7','~'}, 		HotKeyFunctionCat::none, 					    FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 54 '6'
        {"f7",			{CSI_ESC,'[','1','8','~'}, 		HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f8",			{CSI_ESC,'[','1','9','~'}, 		HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f9",			{CSI_ESC,'[','2','0','~'}, 		HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f10",			{CSI_ESC,'[','2','1','~'}, 		HotKeyFunctionCat::none, 					    FieldCompletionNav::na, 			FieldIntraNav::na}, // TODO: is this like ESC or EOF_CHAR?
        {"f11",			{CSI_ESC,'[','2','3','~'}, 		HotKeyFunctionCat::none, 					    FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 50
        {"f12",			{CSI_ESC,'[','2','4','~'},  	HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        // VT100/220, but not on USA PC AT keyboard
        {"f13",			{CSI_ESC,'[','2','5','~'},  	HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f14",			{CSI_ESC,'[','2','6','~'},  	HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f15",			{CSI_ESC,'[','2','7','~'},  	HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f16",			{CSI_ESC,'[','2','8','~'},  	HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f17",			{CSI_ESC,'[','3','1','~'},  	HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f18",			{CSI_ESC,'[','3','2','~'},  	HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f19",			{CSI_ESC,'[','3','3','~'},  	HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f20",			{CSI_ESC,'[','3','4','~'},  	HotKeyFunctionCat::none,					    FieldCompletionNav::na, 			FieldIntraNav::na},
        // ;2 is shift aka shf
        //{"shf-insert",	{CSI_ESC,'[','2',	 ';','2','~'}, 	HotKeyFunctionCat::editing_mode,			FieldCompletionNav::na,			 	FieldIntraNav::na},
        {"shf-delete",		{CSI_ESC,'[','3', 	 ';','2','~'},	HotKeyFunctionCat::nav_intra_field,			FieldCompletionNav::na, 			FieldIntraNav::delete_char},
        //{"shf-pageup",	{CSI_ESC,'[','5',    ';','2','~'}, 	HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::page_up,		FieldIntraNav::na},
        //{"shf-pagedown",	{CSI_ESC,'[','6',    ';','2','~'}, 	HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::page_down,		FieldIntraNav::na},
        {"shf-up_arrow",	{CSI_ESC,'[','1',    ';','2','A'},	HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::browse_up, 		FieldIntraNav::na},
        {"shf-down_arrow",	{CSI_ESC,'[','1',    ';','2','B'},	HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::browse_down, 	FieldIntraNav::na},
        {"shf-right_arrow",	{CSI_ESC,'[','1',    ';','2','C'},	HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::move_right},
        {"shf-left_arrow",	{CSI_ESC,'[','1',    ';','2','D'},	HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::move_left},
        //{"shf-keypad_5",	{'5'},				 				HotKeyFunctionCat::na,			 		FieldCompletionNav::na, 			FieldIntraNav::na},
        //???{"shf-home",	{CSI_ESC,'[','1',    ';','2','H'}}, HotKeyFunctionCat::nav_intra_field,		FieldCompletionNav::na,			 	FieldIntraNav::goto_begin},
        //???{"shf-end",	{CSI_ESC,'[','1',    ';','2','F'}}, HotKeyFunctionCat::nav_intra_field, 	FieldCompletionNav::na, 			FieldIntraNav::goto_end},
        {"shf-f1",			{CSI_ESC,'[','1',    ';','2','P'}, 	HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"shf-f2",			{CSI_ESC,'[','1',    ';','2','Q'}, 	HotKeyFunctionCat::none, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 54 '6'
        {"shf-f3",			{CSI_ESC,'[','1',    ';','2','R'}, 	HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"shf-f4",			{CSI_ESC,'[','1',    ';','2','S'}, 	HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"shf-f5",			{CSI_ESC,'[','1','5',';','2','~'}, 	HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"shf-f6",			{CSI_ESC,'[','1','7',';','2','~'}, 	HotKeyFunctionCat::none, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 54 '6'
        {"shf-f7",			{CSI_ESC,'[','1','8',';','2','~'}, 	HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"shf-f8",			{CSI_ESC,'[','1','9',';','2','~'}, 	HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"shf-f9",			{CSI_ESC,'[','2','0',';','2','~'}, 	HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        //???{"shf-f10",	{CSI_ESC,'[','2','1',';','2','~'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // TODO: is this like ESC or EOF_CHAR?
        {"shf-f11",			{CSI_ESC,'[','2','3',';','2','~'}, 	HotKeyFunctionCat::none, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 50
        {"shf-f12",			{CSI_ESC,'[','2','4',';','2','~'},  HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        // shift-keypad_key's is just like "NumLock", you get the numbers, not the arrows etc., ie. like shift for the keypad. This may be false!
        // ;5 is ctl
        //{"ctl-insert",	{CSI_ESC,'[','2',    ';','5','~'}, 		HotKeyFunctionCat::editing_mode,			FieldCompletionNav::na,			 	FieldIntraNav::na},
        {"ctl-delete",		{CSI_ESC,'[','3',    ';','5','~'}, 		HotKeyFunctionCat::nav_intra_field,			FieldCompletionNav::na, 			FieldIntraNav::delete_char},
        {"ctl-pageup",		{CSI_ESC,'[','5',    ';','5','~'}, 		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::page_up,		FieldIntraNav::na},
        {"ctl-pagedown",	{CSI_ESC,'[','6',    ';','5','~'}, 		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::page_down,		FieldIntraNav::na},
        {"ctl-up_arrow",	{CSI_ESC,'[','1',    ';','5','A'},		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::browse_up, 		FieldIntraNav::na},
        {"ctl-down_arrow",	{CSI_ESC,'[','1',    ';','5','B'},		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::browse_down, 	FieldIntraNav::na},
        {"ctl-right_arrow",	{CSI_ESC,'[','1',    ';','5','C'},		HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::move_right},
        {"ctl-left_arrow",	{CSI_ESC,'[','1',    ';','5','D'},		HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::move_left},
        {"ctl-keypad_5",	{CSI_ESC,'[','1',    ';','5','E'},		HotKeyFunctionCat::none, 						FieldCompletionNav::na, 			FieldIntraNav::na},
        //???{"ctl-home",	{CSI_ESC,'[','1',    ';','5','H'}},		HotKeyFunctionCat::nav_intra_field,			FieldCompletionNav::na,			 	FieldIntraNav::goto_begin},
        //???{"ctl-end",	{CSI_ESC,'[','1',    ';','5','F'}},		HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::goto_end},
        {"ctl-f1",			{CSI_ESC,'[','1',    ';','5','P'}, 	HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"ctl-f2",			{CSI_ESC,'[','1',    ';','5','Q'}, 	HotKeyFunctionCat::none, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 54 '6'
        {"ctl-f3",			{CSI_ESC,'[','1',    ';','5','R'}, 	HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"ctl-f4",			{CSI_ESC,'[','1',    ';','5','S'}, 	HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"ctl-f5",			{CSI_ESC,'[','1','5',';','5','~'}, 	HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"ctl-f6",			{CSI_ESC,'[','1','7',';','5','~'}, 	HotKeyFunctionCat::none, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 54 '6'
        {"ctl-f7",			{CSI_ESC,'[','1','8',';','5','~'}, 	HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"ctl-f8",			{CSI_ESC,'[','1','9',';','5','~'}, 	HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"ctl-f9",			{CSI_ESC,'[','2','0',';','5','~'}, 	HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},
        //{"ctl-f10",		{CSI_ESC,'[','2','1',';','5','~'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // TODO: is this like ESC or EOF_CHAR?
        {"ctl-f11",			{CSI_ESC,'[','2','3',';','5','~'}, 	HotKeyFunctionCat::none, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 50
        {"ctl-f12",			{CSI_ESC,'[','2','4',';','5','~'},  HotKeyFunctionCat::none,					FieldCompletionNav::na, 			FieldIntraNav::na},

        // ;3 is alt
        {"alt-insert",		{CSI_ESC,'[','2',';','3','~'}, 		HotKeyFunctionCat::editing_mode,			FieldCompletionNav::na,			 	FieldIntraNav::na},
        {"alt-delete",		{CSI_ESC,'[','3',';','3','~'}, 		HotKeyFunctionCat::nav_intra_field,			FieldCompletionNav::na, 			FieldIntraNav::delete_char},
        {"alt-pageup",		{CSI_ESC,'[','5',';','3','~'}, 		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::page_up,		FieldIntraNav::na},
        {"alt-pagedown",	{CSI_ESC,'[','6',';','3','~'}, 		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::page_down,		FieldIntraNav::na},
        {"alt-up_arrow",	{CSI_ESC,'[','1',';','3','A'},		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::browse_up, 		FieldIntraNav::na},
        {"alt-down_arrow",	{CSI_ESC,'[','1',';','3','B'},		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::browse_down, 	FieldIntraNav::na},
        {"alt-right_arrow",	{CSI_ESC,'[','1',';','3','C'},		HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::move_right},
        {"alt-left_arrow",	{CSI_ESC,'[','1',';','3','D'},		HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::move_left},
        {"alt-keypad_5",	{CSI_ESC,'[','1',';','3','E'},		HotKeyFunctionCat::none, 						FieldCompletionNav::na, 			FieldIntraNav::na},
        /*???{"alt-home",	{CSI_ESC,'[','1',    ';','3','H'}},		HotKeyFunctionCat::nav_intra_field,			FieldCompletionNav::na,			 	FieldIntraNav::goto_begin},
        //???{"alt-end",	{CSI_ESC,'[','1',    ';','3','F'}},		HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::goto_end},
        //{"alt-f1",			{CSI_ESC,'[','1',    ';','3','P'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        //{"alt-f2",			{CSI_ESC,'[','1',    ';','3','Q'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 54 '6'
        //{"alt-f3",			{CSI_ESC,'[','1',    ';','3','R'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        //{"alt-f4",			{CSI_ESC,'[','1',    ';','3','S'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        //{"alt-f5",			{CSI_ESC,'[','1','5',';','3','~'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        //{"alt-f6",			{CSI_ESC,'[','1','7',';','3','~'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 54 '6'
        //{"alt-f7",			{CSI_ESC,'[','1','8',';','3','~'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        //{"alt-f8",			{CSI_ESC,'[','1','9',';','3','~'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        //{"alt-f9",			{CSI_ESC,'[','2','0',';','3','~'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        //{"alt-f10",			{CSI_ESC,'[','2','1',';','3','~'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // TODO: is this like ESC or EOF_CHAR?
        //{"alt-f11",			{CSI_ESC,'[','2','3',';','3','~'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 50
        //{"alt-f12",			{CSI_ESC,'[','2','4',';','3','~'},  HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        // ;6 is ctl-shf
        // ?? is ctl-alt
        // ?? is shf-alt
         *  TODO: where did I get the idea of these next 4?  Not consistent with what I have above.
            * Shift-f1		{ESC,LSB,'2',   59,80}
            * Shift-f2		{ESC,LSB,'2',   59,81}
            * CTRL-f1		{ESC,LSB,FIVE,  59,80}
            * CTRL-f2		{ESC,LSB,FIVE,  59,81}
            === Some more notes on keys and their meaning ===
            Left Arrow			Moves the cursor one character to the left.
            Shift+Left Arrow	Moves and selects text one character to the left.
            Right Arrow			Moves the cursor one character to the right.
            Shift+Right Arrow	Moves and selects text one character to the right.
            Home				Moves the cursor to the beginning of the line.
            End					Moves the cursor to the end of the line.
            Backspace			Deletes the character to the left of the cursor.
            Ctrl+Backspace		Deletes the word to the left of the cursor.
            Delete				Deletes the character to the right of the cursor.
            Ctrl+Delete			Deletes the word to the right of the cursor.
            Ctrl+A				Select all.
            Ctrl+C				Copies the selected text to the clipboard.
            Ctrl+Insert			Copies the selected text to the clipboard.
            Ctrl+K				Deletes to the end of the line.
            Ctrl+V				Pastes the clipboard text into line edit.
            Shift+Insert		Pastes the clipboard text into line edit.
            Ctrl+X				Deletes the selected text and copies it to the clipboard.
            Shift+Delete		Deletes the selected text and copies it to the clipboard.
            Ctrl+Z				Undoes the last operation.
            Ctrl+Y				Redoes the last undone operation.
        */
    };
    assert( not ascii_posix_map.empty() && "Logic error.");
    assert( not hot_key_table.empty()        && "Logic error.");
    if ( static bool once {false}; ! once) {
        once = true;
        assert ( !hot_key_table.empty() && "We don't allow empty hotkeys set.");
        std::sort( hot_key_table.begin(), hot_key_table.end() );
        if ( auto dup = std::adjacent_find( hot_key_table.begin(), hot_key_table.end(), is_hk_chars_equal ); dup != hot_key_table.end() ) {
            LOGGERS("Duplicate hot_key:", dup->my_name);
            assert( false && "We don't allow duplicate hotkey character sequences.");
        }
#ifndef GR_DEBUG
        // TODO??: make it accept operator<< : LOGGERS("Run once only now, here are the hot_key_table,characters:",hot_key_table)
        //LOGGER_("Run once only now, here are the hot_key_table,characters:")
        for (auto & i : hot_key_table) {
            LOGGERS("hot_key:   ", i.my_name);
            LOGGERS("it's chars:", i.characters);
        }
#endif
    }
    LOGGERS("Candidate_hk_chars:", candidate_hk_chars);
    Hot_key const candidate_hk_search_target { {}, candidate_hk_chars };
    auto    const firstmatch_o_prior_itr            { std::lower_bound( hot_key_table.begin(), hot_key_table.end(), candidate_hk_search_target )};  // first that matches, or the prior value (ie before the next too big number).
    if ( firstmatch_o_prior_itr != hot_key_table.end() ) {
        LOGGERS("First match or prior hot_key_table entry:",   firstmatch_o_prior_itr->my_name);
        LOGGERS("Characters within that hot_key_table entry:", firstmatch_o_prior_itr->characters);
    }
    if ( firstmatch_o_prior_itr == hot_key_table.end() ) {
        LOGGER_("Return:No match")
        return E_NO_MATCH;  //RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
    }
    bool is_full_or_partial_match {false};
    if ( std::equal( candidate_hk_chars.begin(),	// not the same as == since length of candidate is not length of hot_key
                     candidate_hk_chars.end(),
                     firstmatch_o_prior_itr->characters.begin(),
                     std::next( firstmatch_o_prior_itr->characters.begin(), candidate_hk_chars.size() )  // Points to character beyond (ie. end()) of the entry in hot_key_table.  Note we check this a the function is called for each subsequent character, so we should not run off the end of the hot_key_table entry.
                   ) ) { // We checked at most the minimum number of characters (ie. size) of the lengths of either.  See above line and comment.
        is_full_or_partial_match = true;
        LOGGER_("Is_full_or_partial_match, but is it a full?");
    }
    if ( is_full_or_partial_match ) {
        if ( firstmatch_o_prior_itr->characters.size() == candidate_hk_chars.size() ) {
            LOGGER_("Return:A match!")
            return *firstmatch_o_prior_itr;  //RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        else {
            LOGGER_("Return:Partial match, must check for next input character");
            return E_PARTIAL_MATCH;  //RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
        }
    }
    LOGGER_("Return:No match");
    return E_NO_MATCH;  //RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
}

Kb_key_a_fstat
get_kb_keystroke_raw() {
    assert( cin.good() && "Precondition.");
    // @return values
    // THIS
    Key_char_singular   first_kcs       {CHAR_NULL} ;
    // OR
    Hot_key_chars       hot_key_chars   {STRING_NULL.cbegin(),STRING_NULL.cend()};
    // AND
    File_status         file_status     {File_status::initial_state};
    LOGGER_("Pre  cin.get():");
    cin.get( first_kcs );           // TODO: first_kcs == 0 // does this ever happen? TODO: 0 == the break character or what else could it mean?
    LOGGER_("Post cin.get():");
    assert( cin && "Logic error: cin is not good.");
    assert( first_kcs != 0 && "Logic error: Got a zero from cin.get().");  // Note: here we consider a zero as a failed cin.get(), below on a timed get we start with a magic number and we expect to get a zero 0 if the timer expires. TODO verify and refactor relationship between TIMED_GET_NOT_SET, TIMED_GET_NULL and CHAR_NULL.
    file_status = get_successfull_iostate_cin();

    bool is_potential_CSI_ALT   {false};
    hot_key_chars.clear();
    if ( first_kcs == CSI_ALT ) {
        hot_key_chars.push_back( CSI_ESC );
        is_potential_CSI_ALT = true;
    }
    else
        hot_key_chars.push_back( first_kcs );

    // ******* Handle the single simple regular char case first and return it.
    if ( first_kcs != CSI_ESC && first_kcs != ESC_KEY ) {
        assert( hot_key_chars.size() == 1 && "Logic error.");
        Hotkey_o_errno hot_key_candidate = consider_hot_key( hot_key_chars );
        if ( std::holds_alternative< Hot_key >( hot_key_candidate ) ) {
            // ******* Handle hot_key that is a single ASCII char first and return it.
            Hot_key_chars hkc   { std::get< Hot_key >( hot_key_candidate ).characters };
            assert( not hkc.empty()                           && "Postcondition9.");
            assert( file_status != File_status::initial_state && "Postcondition1.");
            return { std::get< Hot_key >( hot_key_candidate ), file_status}; //RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
        } else {
            // ******* Handle single simple regular ASCII first and return it.
            assert( std::holds_alternative< Lt_errno >( hot_key_candidate ));
            assert( first_kcs   != 0                          && "Postcondition10.");
            assert( file_status != File_status::initial_state && "Postcondition2.");
            return { first_kcs, file_status}; //RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
        }
    }
    LOGGER_("We have a CSI, so we'll loop to get the whole multi-byte sequence");
    // ******* So far, we have one kb char. Is it a hot_key? Which is could be a singlebyte (CR TAB or ESC) or a multibyte function key like F1.
    while ( true ) {
//        if ( cin.eof() || first_kcs == 0) {             // ????????????????????? does this evern happen? TODO: 0 == the break character or what else could it mean?
//            assert( false && "logic error or needs more work."); // TODO: more eof handling needed
//            assert( (cin.eof() || first_kcs == 0) && "We probably don't handle eof well."); // TODO: more eof handling needed
//            file_status = File_status::eof_file_descriptor;
//            return { hot_key_chars, file_status}; //RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
//        };

        // ******* We might have one or more characters from that single keystroke,
        // ******* so let's get another char within a potential multibyte sequence, which would come very quickly before our timer on the get() expires.
        Key_char_singular   timed_kcs       {TIMED_GET_NOT_SET};  // TODO: verify and refactor relationship between TIMED_GET_NOT_SET, TIMED_GET_NULL and CHAR_NULL.
        Termios const       termios_orig    { termio_set_timer( VTIME_ESC )}; // Set stdin to return with no char if not arriving within timer interval, meaning it is not a multicharacter ESC sequence. Or, a mulitchar ESC seq will provide characters within the interval.
        LOGGER_("Pre  cin.get():");
        cin.get( timed_kcs );  				// see if we get chars too quickly to come from a human, but instead is a multibyte sequence.
        LOGGER_("Post cin.get():");
        file_status = get_successfull_iostate_cin();
        //assert( not is_kb_key_bad_dt_file_status( file_status )); TODO:
        termio_restore( termios_orig );
//            // TODO??: could I use peek() to improve this code?
//        if ( cin.eof() ) {                       // TODO: Is this code needed?  Why commented out? this appears to be triggered by ESC alone, ie. the time expires.  Had thought that just the char would be 0.
//            assert( (cin.eof()) && "Post timer, we probably don't handle eof well."); // TODO: more eof handling needed
//            file_status = File_status::eof_file_descriptor;
//            return { hot_key_chars, file_status}; //RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
//        }
        if ( timed_kcs == TIMED_GET_NULL )
        {                                             // no kbc immediately available within waiting time. NOTE: Must do this check first! if we didn't get another char within prescribed time, it is just a single ESC!// TODO: MAGIC NUMBER
            LOGGER_("Got a timed NULL.");
            file_status = File_status::timed_out;
            hot_key_chars.push_back( NO_MORE_CHARS ); // add a flag value to show a singular ESC TODO: is this needed?? in superficial testing is seems not!  // TODO: MAGIC NUMBER.
            cin.clear();                              // TODO: required after a timer failure has been triggered? Seems to be, why? // note: we have no char to "putback"!
        }
        else {
            LOGGERS("Got a timed char.", timed_kcs);
            // file_status = File_status::other_user_kb_char_data_HACK;  // grostig  just because we determined that we got a good char, doesn't change the file_status, so leave it alone! It was already set above after the get/read().
            hot_key_chars.push_back( timed_kcs );   // We got another char, and we hope it may be part of a multi-byte sequence.
        }

        // ******* Let's see if we now have a single or multybyte Hot_key and can return, or we need to loop again to finalize our the hot_key or error on an unrecognized key sequence.
        Hotkey_o_errno const hot_key_or_error { consider_hot_key( hot_key_chars )};  // We may have a single char, or multi-byte sequence, which is either complete, or only partially read. TODO: consider using ref for speed?
        if ( std::holds_alternative< Hot_key >( hot_key_or_error ) ) {  // We have a real hot_key, so we are done!
            // ******* Hot_key
            assert( first_kcs != 0 && "Postcondition11.");
            assert( not hot_key_chars.empty() && "Postcondition12.");
            assert( File_status::initial_state != file_status && "Postcondition3.");
            //return { std::get< Hot_key >(hot_key_or_error), File_status::other_user_kb_char_data_HACK };  //RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR // TODO: file_status is what? might be EOF or other?
            return { std::get< Hot_key >(hot_key_or_error), file_status };  //RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR // TODO: file_status is what? might be EOF or other?
        }
        else {
            LOGGERS("We have an Lt_errno on considering hot_key_chars:", std::get< Lt_errno >( hot_key_or_error ) );
            switch ( std::get< Lt_errno >( hot_key_or_error ) ) {
            case E_NO_MATCH:                                                // we got a CSI, but what followed didn't match any of the subsequent chars of a multi-byte sequence.
                assert( is_potential_CSI_ALT && "Postcondition13.");
                assert( not hot_key_chars.empty() && "Postcondition14.");   // we have the CSI and zero or more characters appropriate characters, which is what makes it bad.
                assert( file_status != File_status::initial_state && "Postcondition4.");
                //return { hot_key_chars, File_status::unexpected_user_kb_char_data_HACK }; //RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
                return { hot_key_chars, file_status }; //RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
                break;
            case E_PARTIAL_MATCH:  // lets get some more timed input chars to see if we get complete a hot_key.
                continue;          // to the top of the while loop.
                break;
            }
        }
    } //******* END while *******
    assert( false && "We should never get here.");
}

/// Is true/ignore hot_key, if category isn't a HotKeyFunctionCat::nav_field_completion OR SIMILAR HK.
/// Doesn't ignore TODO:
/// Only used once as a helper function, internally to lib_tty
bool is_ignore_hot_key( HotKeyFunctionCat const hot_key_function_cat ) {
    LOGGERS("hot_key_function_cat: ", (int)hot_key_function_cat);
    // **** CASE on hot key Function Category
    switch ( hot_key_function_cat ) {
    case HotKeyFunctionCat::initial_state :
        //cout << "\aLast key press not handled here, so ignored.\n";
        assert(false && "Logic_error: hot_key_function_cat should have been set prior." );  // TODO:
        return true;
        break;
    case HotKeyFunctionCat::editing_mode :
        // TODO: how do we use this?  do we ignore the character?? is_editing_mode_insert = ! is_editing_mode_insert;
        LOGGER_("Editing mode toggled");
        return true;
        break;
    // All subsequent cases appear to be exactly the same except for debugging messages.
    case HotKeyFunctionCat::nav_field_completion :
        LOGGER_("Navigation completion");
        break;
    case HotKeyFunctionCat::navigation_esc :
        LOGGER_("Navigation ESC");
        break;
    case HotKeyFunctionCat::nav_intra_field :
        LOGGER_("Nav_intra_field mode");
        // TODO:  clarify comment: only done for get_value_raw() above, here we let the caller do it: nav_intra_field( hot_key, value, value_index, echo_skc_to_tty);
        break;
    case HotKeyFunctionCat::job_control :
        LOGGER_("Job control");
        break;
    case HotKeyFunctionCat::help_popup :
        LOGGER_("Help Pop-Up");
        break;
    case HotKeyFunctionCat::none :
        LOGGER_("HotKeyFunctionCat::na");
        break;
    }
    return false;
}

/// Is true if we will accept/validate this char for inclusion on a get from the keyboard
/// Usually we don't want control chars!  When might we?  Passwords? Printer control sequences?
/// Only used once as a helper function, internally to lib_tty
bool is_usable_char( Key_char_singular const skc, bool const is_allow_control_chars ) {
    assert( skc != '\0' && "Precondition: Did not expect char of 0, but maybe it is acceptable and this assert needs to change." );
    LOGGERS("Char is:", static_cast< int >( skc ));
    int const i	{ static_cast<int>(skc) };
    return is_allow_control_chars ? ( isprint(i) || iscntrl(i) ) && // allowing control chars, in for example in a password ;)
                                      not( i==17 || i==19 ) 	    // except not: XON & XOFF,  TODO: what about SCROLL LOCK? is that even a character??
                                                                    // but note some may have been parsed out as hot_key_table prior to this test.
                                  :   isprint(i);
}

/// Is true if we disallow the character.
/// TODO: mixing logic and user output may not be good here.
/// Only used once as a helper function, internally to lib_tty
bool is_ignore_kcs( Key_char_singular const skc,
                    bool              const is_allow_control_chars,
                    bool              const is_ring_bell_on_ignore,
                    bool              const is_echo_skc_to_tty )
{
    assert( skc != '\0' && "Precondition: Did not expect char of 0, but maybe it is acceptable and this assert needs to change." );
    LOGGERS("Char is:", static_cast< int >( skc ));
    if ( is_usable_char( skc, is_allow_control_chars )) {
        if ( is_echo_skc_to_tty ) {
            cout << skc <<endl;
        }
        return false;
    }
    else {
        if ( is_ring_bell_on_ignore ) {
            cout << "\a";
        }
        cout << "Last key press invalid/unusable here, so ignored."<<endl;
        return true;
    }
    assert(false && "We never get here.");
}

Kb_value_plus
get_kb_keystrokes_raw( size_t const length_in_keystrokes,
                       bool   const is_require_field_completion_key,
                       bool   const is_echo_skc_to_tty,
                       bool   const is_allow_control_chars
                     ) {
    assert( cin.good() && "Precondition.");
    assert( length_in_keystrokes > 0 && "Precondition: Length must be greater than 0." );   // TODO: must debug n>1 case later.
    Key_char_i18ns 	    key_char_i18ns_result 	{STRING_NULL};  /// The char(s) in the keystroke.
    Hot_key		 		hot_key_result          {};  /// The hot_key that might have been found.
    File_status  		file_status_result      {File_status::initial_state};
    size_t 		 		additional_skc 			{length_in_keystrokes};  // TODO: we presume that bool is worth one and it is added for the CR we require to end the value of specified length.
    HotKeyFunctionCat   hot_key_function_cat  	{HotKeyFunctionCat::none};			// reset some variables from prior loop if any, specifically old/prior hot_key.
    //unsigned 			int value_index			{0}; // Note: Points to the character beyond the current character (presuming zero origin), like an STL iterator it.end(), hence 0 == empty field.
    //bool 		 		is_editing_mode_insert  {true};
    cin.exceptions( std::istream::failbit );            // Throw on fail of cin.  TODO??: maybe interactions with CIN should include more calls to cin.good() etc., and rdstate/ios_base::badbit etc.
    Termios const termios_orig 	{ termio_set_raw() };   /// Used to restore our keyboard to COOKED.
    key_char_i18ns_result.clear();                      // for this loop, we will consider size == 0 to be STRING_NULL.
    do {  // ******* BEGIN do_while to Gather char(s) to make a n-length value or until we get a "completion" Hot_key, or number of chars, or error.**************************
        bool is_ignore_kcs_local    {false};                 /// Suppress this character if user enters it.  TODO: I may not need these locals set by function calls, but I'm not sure until all TODO S are done
        bool is_ignore_hot_key_local{false};                 /// In case editing mode is toggled. TODO: not implemented yet.  TODO: I may not need these locals set by function calls, but I'm not sure until all TODO S are done
        bool is_adequate_fs_local   {false};                 /// Don't worry about these file_descriptor stati.  TODO: I may not need these locals set by function calls, but I'm not sure until all TODO S are done
        hot_key_result              = {};  				     // reset some variables from prior loop if any, specifically old/prior hot_key.
        hot_key_function_cat        = HotKeyFunctionCat::none; // reset some variables from prior loop if any, specifically old/prior hot_key.
                                                               // TODO?: may not need this local, but not sure untill below TODOs are done.

        Kb_key_a_fstat const    kb_key_a_fstat  { get_kb_keystroke_raw() };  // READ RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
        file_status_result 		  		        = kb_key_a_fstat.file_status;
        LOGGERS("Just read a keystroke, file_status:", (int)kb_key_a_fstat.file_status);
        if ( (is_adequate_fs_local = is_adequate_file_status( file_status_result )))   // file_status is OK
        {
            LOGGERS("is_ignore_key_fd:", is_adequate_fs_local);
            if      ( std::holds_alternative< Key_char_singular >( kb_key_a_fstat.kb_key_variant )) {
                Key_char_singular const kcs { std::get < Key_char_singular >( kb_key_a_fstat.kb_key_variant ) };
                LOGGERS("is kcs:", kcs);
                if ( not ( is_ignore_kcs_local = is_ignore_kcs( kcs, is_allow_control_chars, true, is_echo_skc_to_tty))) {  // TODO: parameterize is_ring_bell_on_ignore, just true here.
                    LOGGERS("is kcs:", kcs);
                    key_char_i18ns_result += kcs;
                }
            }
            else if ( std::holds_alternative< Hot_key > ( kb_key_a_fstat.kb_key_variant )) {
                hot_key_result 			    = std::get < Hot_key >( kb_key_a_fstat.kb_key_variant );  // TODO:?? is this a copy?
                hot_key_function_cat        = hot_key_result.function_cat;
                LOGGERS("is hk:", hot_key_result.my_name);
                LOGGERS("is hot_key_function_cat:", (int)hot_key_result.function_cat);
                //                if ( ( hot_key_function_cat = hot_key_rv.function_cat ) == HotKeyFunctionCat::editing_mode ) {
                //                    is_editing_mode_insert 	= ! is_editing_mode_insert;  // TODO: Do we use this value here, or down the call stack?
                //                    cerr << "Function_cat: Editing mode is insert: "<<is_editing_mode_insert << endl;
                //                }

                is_ignore_hot_key_local            = is_ignore_hot_key( hot_key_function_cat );    // TODO: refactor to use within applicable tests and while 123.
            }
            else {
                LOGGER_("Throwing away this key stroke, trying again to get one" )
                cout<<"\aError:Throwing away this key stroke, trying again to get one."<<endl;
                assert( false && "Logic error:Not sure why we got here, we require that either a Key_char_singular or a Hot_key entered.");
            }
        }
        else {
            assert( false &&"Logic error/omission: on file_status we need to handle this case!");  // file_status is NOT ACCEPTABLE  or is NOT ADEQUATE, which might be different, not sure.
        }
        if ( is_adequate_fs_local || not is_ignore_kcs_local || not is_ignore_hot_key_local  ) {
            LOGGER_("This gotten key stroke a good single regular char");
            --additional_skc;           // TODO??: do we need to, or can we check for underflow on size_t?
        }
        assert(true && "Logic error: nav and nav_eof and file_status::eof_Key_char_singular are not consistent");  // TODO: might be worthwhile to add?

    } while (   additional_skc              >  0                                        &&
                hot_key_function_cat        == HotKeyFunctionCat::none                  &&
                file_status_result          != File_status::eof_file_descriptor
                //file_status_result          != File_status::eof_Key_char_singular       &&
                //file_status_result          != File_status::eof_library                 &&
            );      // TODO: also NEED TO HANDLE hot_key_chars alone?  eof of both types?  intrafield?  editing mode? monostate alone
    //******* END   do_while ****************************************************************************************************************
    //******* START while    Either we already got a "completion" hot_key (if required) or we shall enter the loop and get one now throwing away other keystrokes.
    while (     is_require_field_completion_key                                         &&  // TODO: probably totally wrong, check it.
                hot_key_result.function_cat != HotKeyFunctionCat::nav_field_completion  &&  // TODO: may need more cats like intra_field, editing_mode?
                hot_key_result.function_cat != HotKeyFunctionCat::navigation_esc        &&
                file_status_result          != File_status::eof_file_descriptor
                //file_status_result          != File_status::eof_Key_char_singular       &&
                //file_status_result          != File_status::eof_library                 &&
          )
    {
        Kb_key_a_fstat const kb_key_a_fstat { get_kb_keystroke_raw() };  // READ RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
        file_status_result                  = kb_key_a_fstat.file_status;
        if ( Kb_key_variant const k { kb_key_a_fstat.kb_key_variant }; std::holds_alternative< Hot_key >( k ))
            hot_key_result = std::get< Hot_key >( k );          // We are not stepping on a usable completion hot_key from n-length loop due to the check for this while loop.
        else
            cout<< "\aError: expecting a CR or other special completion key, try again."<<endl;
    }   //******* END   while *****************************************************************************************************************
    termio_restore( termios_orig );
    /*XXXXX */
    // We RETURN either N regular char(s), OR both the N regular char(s) and the latest Hot_key, OR just a Hot_key
    assert( file_status_result != File_status::initial_state && "Postcondition5.");
    if ( not key_char_i18ns_result.empty() ) {
        LOGGERS("My character(s) are/is:", key_char_i18ns_result);
        LOGGERS("My character(s) length is:", key_char_i18ns_result.size());
        if (        hot_key_result.my_name == STRING_NULL ) {
            assert( hot_key_result.my_name == STRING_NULL && "Postcondition6: result not set.");
            assert( not key_char_i18ns_result.empty() && "Postcondition16: result not set.");
            return { key_char_i18ns_result, {},             file_status_result };
        } else {
            assert( hot_key_result.my_name != STRING_NULL && "Postcondition6: result not set.");
            assert( not key_char_i18ns_result.empty() && "Postcondition16: result not set.");
            return { key_char_i18ns_result, hot_key_result, file_status_result };
        }
    } else {
        LOGGERS("Hot_key name is:", hot_key_result.my_name );
        assert(     hot_key_result.my_name != STRING_NULL  && "Postcondition7: result not set.");
        assert(         key_char_i18ns_result.empty() && "Postcondition7: result not set.");
        return     { {}                   , hot_key_result, file_status_result };
    }
    assert( false && "We never get here.");
}

}  // namespace end Lib_tty
