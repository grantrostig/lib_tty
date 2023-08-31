#include "lib_tty.h"
#include <cstring>
#include <iomanip>
#include <iterator>
//#include <variant>
#include <algorithm>
#include <iostream>
#include <type_traits>
#include <concepts>
#include <stacktrace>
#include <source_location>
using std::endl, std::cin, std::cout, std::cerr, std::string;
/// todo??: better alternative? $using namespace Lib_tty; // or $Lib_tty::C_EOF // or $using Lib_tty::C_EOF will this last one work?
namespace Lib_tty {

/// define if asserts are NOT to be checked.  Put in *.h file not *.CPP
//#define 	NDEBUG
/// define I'm Debugging LT.  Put in *.h file not *.CPP
#define  	GR_DEBUG
//#undef  	GR_DEBUG
//#ifdef   	GR_DEBUG
//#endif  # GR_DEBUG
#define LOGGER_( msg )   using loc = std::source_location;std::cerr<<"["<<loc::current().file_name()<<':'<<loc::current().line()<<','<<loc::current().column()<<"]`"<<loc::current().function_name()<<"`:" <<#msg<<".\n";
#define LOGGERS( msg, x )using loc = std::source_location;std::cerr<<"["<<loc::current().file_name()<<':'<<loc::current().line()<<','<<loc::current().column()<<"]`"<<loc::current().function_name()<<"`:" <<#msg<<",{"<<x<<"}.\n";

/*  *** START Debugging only section *** */
//  *** START of C++20 illustration ***
/** print_vec replacement
 * Debugging only.
 * todo??: Illustration of how to print vectors and some other containers with various types of members/structs.
 * todo??: What forward declartions would I have to do to move this to bottom of this file, to get it out of the way?
 */
template <typename T>
concept can_insert = requires( std::ostream & out, T my_t ) {
    // todo??: is this the same, or just more clear to me? $ { out << my_t; } -> is_convertable <std::ostream & >;
    // todo??: does my_t have std::ostream_iterator<T>?  how related, or needed, or my misunderstanding
    { out << my_t };
};

/// print_vec replacement
template<typename T>            // utility f() to print vectors
std::ostream&
operator<<( std::ostream & out, const std::vector<T> & v) requires can_insert<T> {
    if (!v.empty()) {
        out << '<';
        std::copy(v.begin(), v.end(), std::ostream_iterator<T>(out, "&"));
        // out << "\b\b]";
        out << ">";
    }
    return out;
}

void print_vec( const std::vector<char> & v) {  /// Debugging only. Obsolete if above works?
    LOGGER_("Elements:[")
    for (auto const i: v) {
        cerr << std::setw(3) << (int)i << "&";
    }
    cerr << "\b]";
}
// 	*** END   of C++20 illustration ***

void print_signal(int const signal) {
    LOGGERS( "Signal is:", signal);
    switch (signal) {
    /* ISO C99 signals.  */
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
        LOGGER_( "Non-typical POSIX signal");
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

/*  *** END   Debugging only section *** */

bool is_hk_chars_equal( Hot_key const & left, Hot_key const & right ) {
    return ( left.characters == right.characters );
}

bool Hot_key::operator< ( Hot_key const  & in ) const {  // found in lib_tty.h
    return ( characters < in.characters );
}

std::string
Hot_key::to_string() const {  // found in lib_tty.h
    std::string s {my_name};  // todo: finish this
    return s;
}


Sigaction_termination_return
set_sigaction_for_termination( Sigaction_handler_fn_t handler_in) {  // todo: TODO why does const have no effect here?
    LOGGER_ ("lib_tty:set_sigaction_for_termination(): starting.");
    struct sigaction action 		{};    			 // todo:  TODO?? why is struct required by compiler.  TODO?? does this initialize deeply in c++ case?
    sigemptyset( &action.sa_mask );  				 // Isn't this already empty because of declaration? We do it just to be safe.
    action.sa_flags 				|= ~SA_SIGINFO;  // Invoke signal-catching function with three arguments instead of one.
    action.sa_flags 				|= ~SA_RESTART;  // Restart syscall on signal return.  todo: not sure this is needed.
    action.sa_sigaction 			=  handler_in;

    // we could block certain signals here, but we choose not to in the case where we prompt the end-user for the password.
    // 		sigset_t block_mask; sigaddset(&block_mask, SIGINT and SIGQUIT); action.sa_mask = block_mask;  // SS 24.7.5 GNU libc manual

    static struct sigaction action_prior_SIGINT 	{};
    if (sigaction( SIGINT , nullptr, /*out*/ &action_prior_SIGINT ) == POSIX_ERROR) { perror("lib_tty:"); exit(1); }  // just doing a get()

    //if (action_prior_SIGINT.sa_sigaction != reinterpret_cast<void(*)(int, siginfo_t *, void *)>(SIG_IGN)) { // we avoid setting a signal on those that are already ignored. todo: TODO why is this different from My_sighandler_t?
    //if ( (void *) action_prior_SIGINT.sa_sigaction != (void *) SIG_IGN) { // we avoid setting a signal on those that are already ignored. todo: TODO why is this different from My_sighandler_t?
    //if (action_prior_SIGINT.sa_sigaction != reinterpret_cast<void(*)(int)>(SIG_IGN)) { // we avoid setting a signal on those that are already ignored. todo: TODO why is this different from My_sighandler_t?

    if ( reinterpret_cast<void *>(action_prior_SIGINT.sa_handler) != reinterpret_cast<void *>(SIG_IGN)) { // we avoid setting a signal on those that are already ignored. todo: TODO why is this different from My_sighandler_t?
        LOGGER_( "set_sigaction_for_termination(): SIGINT going to be set.");
        if (sigaction(SIGINT, &action, nullptr) == POSIX_ERROR ) { perror("lib_tty:"); exit(1); }
    }

    static struct sigaction action_prior_SIGQUIT 	{};
    if (sigaction( SIGQUIT , nullptr, /*out*/ &action_prior_SIGQUIT ) == POSIX_ERROR) { perror("lib_tty:"); exit(1); }  // just doing a get()
    if (action_prior_SIGQUIT.sa_sigaction != reinterpret_cast<void(*)(int, siginfo_t *, void *)>(SIG_IGN)) { // we avoid setting a signal on those that are already ignored.
        LOGGER_( "set_sigaction_for_termination(): SIGQUIT going to be set." );
        if (sigaction(SIGQUIT, &action, nullptr) == POSIX_ERROR ) { perror("lib_tty:"); exit(1); }  // todo: Would this only be used if there was a serial line that could generate a HUP signal?
    }

    static struct sigaction action_prior_SIGTERM 	{};
    if (sigaction( SIGTERM , nullptr, /*out*/ &action_prior_SIGTERM ) == POSIX_ERROR) { perror("lib_tty:"); exit(1); }  // just doing a get()
    if (action_prior_SIGTERM.sa_sigaction != reinterpret_cast<void(*)(int, siginfo_t *, void *)>(SIG_IGN)) { // we avoid setting a signal on those that are already ignored.
        LOGGER_( "set_sigaction_for_termination(): SIGTERM going to be set." );
        if (sigaction(SIGTERM, &action, nullptr) == POSIX_ERROR ) { perror("lib_tty:"); exit(1); }  // todo: Does not seem to work. BUG: Ctrl-\ causes a immediate dump.
    }

    static struct sigaction action_prior_SIGTSTP 	{};
    if (sigaction( SIGTSTP , nullptr, /*out*/ &action_prior_SIGTSTP ) == POSIX_ERROR) { perror("lib_tty:"); exit(1); }  // just doing a get()
    if (action_prior_SIGTSTP.sa_sigaction != reinterpret_cast<void(*)(int, siginfo_t *, void *)>(SIG_IGN)) { // we avoid setting a signal on those that are already ignored.
        LOGGER_("set_sigaction_for_termination(): SIGTSTP going to be set.");
        if (sigaction(SIGTSTP, &action, nullptr) == POSIX_ERROR ) { perror("lib_tty:"); exit(1); }  // todo: Does not seem to work. BUG: Ctrl-\ causes a immediate dump.
    }

    static struct sigaction action_prior_SIGHUP 	{};
    if (sigaction( SIGHUP , nullptr, /*out*/ &action_prior_SIGHUP ) == POSIX_ERROR) { perror("lib_tty:"); exit(1); }  // just doing a get()
    if (action_prior_SIGHUP.sa_sigaction != reinterpret_cast<void(*)(int, siginfo_t *, void *)>(SIG_IGN)) { // we avoid setting a signal on those that are already ignored.
        LOGGER_( "set_sigaction_for_termination(): SIGHUP going to be set." );
        if (sigaction(SIGHUP, &action, nullptr) == POSIX_ERROR ) { perror("lib_tty:"); exit(1); }  // todo: Does not seem to work. BUG: Ctrl-\ causes a immediate dump.
    }
    return { action_prior_SIGINT, action_prior_SIGQUIT, action_prior_SIGTERM, action_prior_SIGTSTP, action_prior_SIGHUP };
}

// forward declaration
Sigaction_return
set_sigaction_for_inactivity( Sigaction_handler_fn_t handler_in );

/* signal handler function to be called when a timeout alarm goes off via a user defined signal is received */
void handler_inactivity(int const sig, Siginfo_t *, void *) {  // todo: TODO why can't I add const here without compiler error?
    LOGGERS( "This signal got us here:", sig);
    print_signal( sig );
    set_sigaction_for_inactivity( handler_inactivity );  //  re-create the handler we are in so we can get here again when handling is called for!?!? // todo: WHY, since it seems to have been used-up/invalidated somehow?
    // todo: ?? raise (sig);
    // todo: ?? signal(sig, SIG_IGN);
}

void handler_termination(int const sig, Siginfo_t *, void *) {
    LOGGERS( "This signal # got us here:", sig);
    print_signal( sig );
    set_sigaction_for_termination( handler_termination );  // re-create the handler we are in so we can get here again when handling is called for!?!? // todo: WHY, since it seems to have been used-up/invalidated somehow?
}
/// put signal handling back? todo:
    void sigaction_restore_for_termination( Sigaction_termination_return const & actions_prior ) {
    if (sigaction( SIGINT,  &actions_prior.action_prior1, nullptr) == POSIX_ERROR ) { perror("lib_tty:"); exit(1); }
    else {LOGGER_( "SIGINT set to original state." )};
    if (sigaction( SIGQUIT, &actions_prior.action_prior2, nullptr) == POSIX_ERROR ) { perror("lib_tty:"); exit(1); }
    else {LOGGER_( "SIGQUIT set to original state." )};
    if (sigaction( SIGTERM, &actions_prior.action_prior3, nullptr) == POSIX_ERROR ) { perror("lib_tty:"); exit(1); }
    else {LOGGER_( "SIGTERM set to original state." )};
    if (sigaction( SIGTSTP, &actions_prior.action_prior4, nullptr) == POSIX_ERROR ) { perror("lib_tty:"); exit(1); }
    else {LOGGER_( "SIGTSTP set to original state." )};
    if (sigaction( SIGHUP,  &actions_prior.action_prior5, nullptr) == POSIX_ERROR ) { perror("lib_tty:"); exit(1); }
    else {LOGGER_( "SIGHUP set to original state." )};
    return;
}

/// called by enable_inactivity_handler
Sigaction_return
set_sigaction_for_inactivity( Sigaction_handler_fn_t handler_in ) {
    struct sigaction action {};
    sigemptyset( &action.sa_mask );
    action.sa_flags 	|= ~SA_SIGINFO;  // Invoke signal-catching function with three arguments instead of one.
    action.sa_flags 	|= ~SA_RESTART;  // Restart syscall on signal return.
    action.sa_sigaction	=   handler_in;  // todo:  TODO recursive macro is disabled by compiler?

    // we could block certain signals here, but we choose not to in the case where we prompt the use for the password.
    // sigset_t block_mask; sigaddset(&block_mask, SIGINT and SIGQUIT); action.sa_mask = block_mask;  // SS 24.7.5 GNU libc manual

    int 				signal_for_user  	 { SIGRTMIN };
    struct 	sigaction 	action_prior {};
    LOGGERS( "Signal going to be set is int SIGRTMIN, which is:", signal_for_user);
    if (sigaction( signal_for_user , &action, /* out */ &action_prior ) == POSIX_ERROR) { perror("lib_tty:unable to set signal action, exit(1) now."); exit(1); }
    return {signal_for_user, action_prior};
}

/** Called internally every time the timer interval needs to be reset to wait for the full time. In other words after we get a character, we start waiting all over again.
 *  Called by enable_inactivity_handler.
 *  Initially and every time the timer interval needs to be reset to wait for the full time,
 *  even if it had not expired.
 *  In other words after we get a character, we start waiting all over again. */
void set_a_run_inactivity_timer(const timer_t & inactivity_timer, const int seconds) {  // todo: is timer_t a pointer or NOT or struct or int?!?
    int settime_flags 						{};				// TIMER_ABSTIME is only flag noted. Not used here.
    static itimerspec timer_specification 	{};				// todo: does this need to be static, ie. what does timer_settime() do with it?
    timer_specification.it_value.tv_sec 	= seconds;
    timer_specification.it_interval.tv_sec 	= timer_specification.it_value.tv_sec;  // interval is same value. todo: BUG: buggy behaviour when interval is very short ie. 1000 or long 1000000.
    if ( timer_settime( inactivity_timer, settime_flags, &timer_specification, /*out*/ nullptr) == POSIX_ERROR) { perror("lib_tty:"); exit(1); }
    return;
}

/// Configures and sets timer and runs timer which waits for additional user kb characters.  Timer needs to be deleted when no longer wanted. */
std::tuple<timer_t &, int, struct sigaction>
enable_inactivity_handler(const int seconds) {
    static sigevent inactivity_event  {};
    inactivity_event.sigev_notify	= SIGEV_SIGNAL;  // Notify via signal.
    inactivity_event.sigev_signo  	= SIGRTMIN;	   // Return number of available real-time signal with highest priority.
    // grostig todo bug? auto [sig_user, action_prior] 	= set_sigaction_for_inactivity( handler_inactivity );
    Sigaction_return result = set_sigaction_for_inactivity( handler_inactivity );

    static timer_t inactivity_timer   {};
    if ( timer_create( CLOCK_REALTIME, &inactivity_event, /*out*/ &inactivity_timer) == POSIX_ERROR) { perror("lib_tty:"); exit(1); }  // todo:  TODO address of a pointer? seriously ptr to ptr?
    set_a_run_inactivity_timer( inactivity_timer, seconds );
    // grostig todo bug? return { inactivity_timer, sig_user, action_prior };
    return { inactivity_timer, result.signal_for_user, result.action_prior };
}

/// todo:verify> stop waiting for additional keyboard characters from the user? delete the CSI/ESC timer
void disable_inactivity_handler(const timer_t inactivity_timer, const int sig_user, const struct sigaction old_action) {
    LOGGERS( "disable inactivity handler on signal: ", sig_user);
    if ( timer_delete( inactivity_timer) 			    == POSIX_ERROR) { perror("lib_tty:disable_inactivity_handler"); exit(1); } // should print out message based on ERRNO // todo: fix this up. TODO this throws in c lang???
    if ( sigaction(    sig_user, &old_action, nullptr) 	== POSIX_ERROR) { perror("lib_tty:disable_inactivity_handler:sigaction"); exit(1); } // should print out message based on ERRNO // todo: fix this up.  TODO __THROW ???
}

/// to show what is happening on standard-in/cin
void print_iostate(const std::istream &stream) {  // Used for debugging. // todo: TODO how do I pass in cin or cout to this?
    LOGGER_( "Is:");
    if (stream.rdstate() == std::ios_base::goodbit) {LOGGER_( "goodbit only, ")};
    if (stream.rdstate() &  std::ios_base::goodbit) {LOGGER_( "goodbit, ")};
    if (stream.rdstate() &  std::ios_base::eofbit)  {LOGGER_( "eofbit, ")};
    if (stream.rdstate() &  std::ios_base::failbit) {LOGGER_( "failbit, ")};
    if (stream.rdstate() &  std::ios_base::badbit)  {LOGGER_( "badbit,")};
}

/// since == doesn't work on structs.
bool check_equality(const Termios &terminal_status, const Termios &terminal_status2){  // Used for debugging using assert().
    /* https://embeddedgurus.com/stack-overflow/2009/12/effective-c-tip-8-structure-comparison/
     * https://isocpp.org/blog/2016/02/a-bit-of-background-for-the-default-comparison-proposal-bjarne-stroustrup
     * https://stackoverflow.com/questions/141720/how-do-you-compare-structs-for-equality-in-c
     * https://stackoverflow.com/questions/46995631/why-are-structs-not-allowed-in-equality-expressions-in-c
     * https://stackoverflow.com/questions/7179174/why-doesnt-c-provide-struct-comparison/47056810#47056810
     * if ( terminal_status.c_iflag != terminal_status2.c_iflag
        && terminal_status.c_oflag  != terminal_status2.c_oflag
        && terminal_status.c_cflag  != terminal_status2.c_cflag
        && terminal_status.c_lflag  != terminal_status2.c_lflag
        && terminal_status.c_line   != terminal_status2.c_line
        && terminal_status.c_ispeed != terminal_status2.c_ispeed
        && terminal_status.c_ospeed != terminal_status2.c_ospeed ) return false;
    for (int i=0; i< NCCS; ++i)
        if (terminal_status.c_cc[i] != terminal_status2.c_cc[i]) return false; */
    // Apparently this is inreliable, but perhaps with this simple datastructure it will work on most/all machines?
    if ( std::memcmp( &terminal_status, &terminal_status2, sizeof terminal_status) != 0) return false;  // does not set ERRNO.  todo:?? Are Termios structures initialized, including the possible padding when defined as below?  How does c++ know the padding that the Linux kernel expects in a system call?
    return true;
}

Termios & termio_get() { // uses POSIX  // todo TODO what are advantages of other version of this function?
    static Termios terminal_status;
    if (auto result = tcgetattr( fileno(stdin), &terminal_status); result == POSIX_ERROR) { // todo: TODO throw() in signature?
        int errno_save = errno;
        LOGGERS( "Standard-in is not a tty keyboard??",errno_save);
        errno = errno_save;
        perror("termio_get()");
        LOGGER_( "We will exit(1) for this error");
        exit(1);
    }
    return terminal_status;  // copy of POD?
}

Termios & termio_set_raw() { // uses POSIX
    cin.sync_with_stdio( false );  									// todo:  iostreams bug?  This is required for timer time-out bug occurs.
    static Termios terminal_status_orig { termio_get() };
    Termios 	   terminal_status_new 	{ terminal_status_orig };
    terminal_status_new.c_lflag 		&= static_cast<tcflag_t>(~ICANON);  // turn off "canonical" or "cooked" mode and go to "non-canonical" or "raw" mode, ie. don't wait for <Enter>. // want this bit OFF for cbreak mode.
    terminal_status_new.c_lflag 		&= static_cast<tcflag_t>(~ECHO);    // turn off "echo" mode, ie. don't automatically show the characters being typed. // want this bit OFF for cbreak mode.
    terminal_status_new.c_lflag 		&= static_cast<tcflag_t>(~IEXTEN);  // turn off ???
    terminal_status_new.c_lflag 		&= static_cast<tcflag_t>(~ISIG);    // turn off ???  // want this bit ON for cbreak mode.
    terminal_status_new.c_iflag 		&= static_cast<tcflag_t>(~BRKINT);  // turn off ???
    terminal_status_new.c_iflag 		&= static_cast<tcflag_t>(~ICRNL);   // turn off ???  // want this bit OFF for cbreak mode.
    terminal_status_new.c_iflag 		&= static_cast<tcflag_t>(~IGNBRK);  // turn off ???
    terminal_status_new.c_iflag 		&= static_cast<tcflag_t>(~IGNCR);   // turn off ???
    terminal_status_new.c_iflag 		&= static_cast<tcflag_t>(~INLCR);   // turn off ???
    terminal_status_new.c_iflag 		&= static_cast<tcflag_t>(~INPCK);   // turn off ???
    terminal_status_new.c_iflag 		&= static_cast<tcflag_t>(~ISTRIP);  // turn off ???
    terminal_status_new.c_iflag 		&= static_cast<tcflag_t>(~IXON);    // turn off ???
    terminal_status_new.c_iflag 		&= static_cast<tcflag_t>(~PARMRK);  // turn off ???
    terminal_status_new.c_oflag 		&= static_cast<tcflag_t>(~OPOST);   // turn off all output processing.
    terminal_status_new.c_cc[VTIME] 	= 0; 								// wait forever to get that char. // http://www.unixwiz.net/techtips/termios-vmin-vtime.html
    terminal_status_new.c_cc[VMIN]  	= 1;  								// get minimun one char
    if (auto result = tcsetattr( fileno(stdin), TCSADRAIN, /*IN*/ &terminal_status_new); result == POSIX_ERROR) {
                                 // https://pubs.opengroup.org/onlinepubs/9699919799/
                                 // todo: Applications that need all of the requested changes made to work properly should follow tcsetattr() with a call to tcgetattr() and compare the appropriate field values.
        int errno_save = errno;
        LOGGERS( "Standard in is not a tty keyboard??",errno_save);
        errno = errno_save;
        perror("termio_set_raw()");
        exit(1);
    }
    Termios terminal_status_actual { termio_get() };
    assert( check_equality( terminal_status_actual, terminal_status_new) && "Termio not fully raw.");
    return terminal_status_orig;
}

void termio_restore(Termios const &terminal_status_orig) { // uses POSIX  // todo: TODO do you like my const 2x, what is effect calling POSIX?
    if (auto result = tcsetattr(fileno(stdin), TCSADRAIN, /*IN*/ &terminal_status_orig); result == POSIX_ERROR) { // restore prior status
        int errno_save = errno;
        LOGGERS( "Standard in is not a tty keyboard??", errno_save);
        errno = errno_save;
        perror("termio_restore()");
        exit(1);
    }
    cin.sync_with_stdio(true);  // todo:  iostreams bug?  This is required for timer time-out bug occurs.
    return;
}
Termios & termio_set_timer(const cc_t time) {  // uses POSIX
    static Termios terminal_status_orig { termio_get() }; // todo: TODO why does this compile with terminal_status and &terminal_status?
    Termios terminal_status_new = terminal_status_orig;
    cin.sync_with_stdio(false);  // todo:  iostreams bug?  This is required for timer time-out bug occurs.
    terminal_status_new.c_cc[VTIME] = time;  // wait some time to get that char
    terminal_status_new.c_cc[VMIN]  = 0;  // no minimum char to get
    if (auto result = tcsetattr(fileno(stdin), TCSADRAIN, /*IN*/ &terminal_status_new); result == POSIX_ERROR) {
        int errno_save = errno;
        LOGGERS( "Standard in is not a tty keyboard??",errno_save);
        errno = errno_save;
        perror("termio_set_timer()");
        exit(1);
    }
    Termios terminal_status_actual { termio_get() };
    assert ( !check_equality( terminal_status_actual, terminal_status_new ) && "Termio_restore() termio not fully raw.");
    return terminal_status_orig;
}

/// give it the string "EOF" and you get back 4 or ^D */
char find_posix_char_from_posix_name(const Ascii_Posix_map &vec, const std::string name) {
    for (auto & ch : vec) {
        if ( ch.posix_name == name )
            return ch.posix_char;
    }
    std::string err_message {"bad_name >"+name+"<"};
    assert ( false && err_message.c_str());
    // we never get here.
}

/// give it "CSI [ A" get back the string name of the hot_key, ie. "right arrow" */
std::optional<Hot_key> find_hot_key(const Hot_keys &hot_keys, const Hot_key_chars this_key) {
    for (auto & hk : hot_keys)
        if ( hk.characters == this_key )
            return hk;
    return {};
}

/// ?
Hotkey_o_errno consider_hot_key( Hot_key_chars const & candidate_hk_chars ) {
    /* https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/termios.h.html
       https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap11.html#tag_11_01_09
       ttp://osr600doc.xinuos.com/en/SDK_sysprog/TDC_SpecialChars.html
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
    static const Ascii_Posix_map ascii_posix_map {  // TODO?? why no constexpr  is it the vector?
    //ascii_id, ascii_name,        posix_id, ascii_ctrl+,  \+, ascii_char, posix_char
        {"NUL", "Null",					"EOL", 		'@',	{},		0,	0},   // [ctl-spacebar does this!] (typically, ASCII NUL) is an additional line delimiter, like the NL character. EOL is not normally used. // If ICANON is set, the EOL character is recognized and acts as a special character on input (see ``Local modes and line disciplines'' in termio(M)).  probably of not used or even available as a char from a PC or any type of keyboard?!
        {"SOH",	"start_of_heading",		"???", 	    'a',	{},		1,	1},   //
        {"STX",	"start_of_text",		"???",	 	'b',	{},		2,	2},   //
        {"ETX",	"end_of_text",			"INTR", 	'c',	{},		3,	3},   // interrupt/stop the program / paste (from copy-paste)?/-SIGINT
        {"EOT",	"end_of_transmission",	"EOF",		'd',  	{}, 	4,	4},   // I perfer to call this "end of file", apparently this is a ascii character char when in POSIX raw mode tty.
        {"ENQ",	"Enquiry",				"???",		'e',  	{}, 	5,	5},   //
        {"ACK",	"Acknowledgement",		"???",		'f',  	{}, 	6,	6},   //
        {"BEL",	"bell",					"BELL?",	'g', 	'a',	7,	7},   // todo: NOT IMPLEMENTED in hot_keys // Ctrl-g = \a  alert?
        {"BS",	"backspace",		 	"BS",		'h',	'b',	8,	8},   // BS alternative, how related to tty erase? todo: NOT IMPLEMENTED in hot_keys // character erase character works on linux and DOS? Backspace on MacOS use DEL for ERASE?
        {"HT",	"horizontal_tab",		"TAB",		'i',	't',	9,	9},
        {"LF",	"line_feed",			"NL-LF",	'j',	'n',	10,	10},  // end the line. newline. NL posix normal line delimiter. On linux with ANSI keyboard this is "Enter" key, was "Return" on typewriters and old PCs? What about "CR"
                                     // EOL/EOL2 from stty is \r\n or 13, 10, or CR,LF  https://www.ni.com/en-us/support/documentation/supplemental/21/labview-termination-characters.html
        {"VT",	"vertical_tab",			"VT?",		'k',	'v',	11,	11},  // todo: NOT IMPLEMENTED in hot_keys  // cursor down one line, like what one would call LF Line feed.
        {"FF",	"form_feed",			"FF?",		'l',	'f',	12,	12},  // todo: NOT IMPLEMENTED in hot_keys // redisplay page.
        {"CR",	"carriage_return",		"CR",		'm',	'r',	13,	13},  // CR note the 'r', end the line. DOS style LF/NL, but is two characters: \n\r?
        {"SO",	"shift_out",			"???",		'n',	{},		14,	14},  //
        {"SI",	"shift_in",				"DISCARD",	'o',	{},		15,	15},  //
        {"DLE",	"data link escape",		"???",		'p',	{},		16,	16},  //
        {"DC1",	"device_control_1/XON",	"START",	'q',	{},		17,	17},  // not usable, todo: NOT IMPLEMENTED in hot_keys // resume tty output  IXON is a tty setting not a character!
        {"DC2",	"device_control_2",		"REPRINT",	'r',	{},		18,	18},  // todo: NOT IMPLEMENTED in hot_keys // redraw the current line.
        {"DC3", "device_control_3/XOFF","STOP",		's',	{},		19,	19},  // not usable, todo: NOT IMPLEMENTED in hot_keys // suspend tty output IXOFF is a tty setting not a character!
        {"DC4", "device_control_4",		"STATUS",	't',	{},		20,	20},  // todo: NOT IMPLEMENTED in hot_keys // on macOS and BSD.
        {"NAK",	"neg. acknowledge",		"KILL",		'u',	{},		21,	21},  // todo: NOT IMPLEMENTED in hot_keys // deletes entire line being typed. todo: "line erase character" kills the current input line when using the POSIX shell?
        {"SYNC","synchronous_idle",		"LNEXT",	'v',	{},		22,	22},  // todo: NOT IMPLEMENTED in hot_keys // paste (from copy-paste)?/
        {"ETB",	"end_of_tranmission_block","WERASE",'w',	{},		23,	23},  // todo: NOT IMPLEMENTED in hot_keys // erase the last word typed.
        {"CAN",	"cancel",				"CANCEL?",	'x',	{},		24,	24},  // todo: NOT IMPLEMENTED in hot_keys // cancel the input line? / cut (from copy-paste)? /
        {"EM",	"end_of_medium",		"???",		'y',	{},		25,	25},  //
        {"SUB",	"substitute",			"SUSP",		'z',	{},		26,	26},  // todo: NOT IMPLEMENTED in hot_keys //send a terminal stop signal. posix job-control: suspend process.  note: use "fg" to bring to foreground, "bg" to make it run in background.  AKA SIGSTOP?-SIGTSTP
        {"ESC",	"escape",				"ESC",		'[',	'e',	27,	27},  // ESC key, the first char of POSIX CSI Control Sequence Introducer
        {"FS",	"file_separator",		"QUIT",		'\\',	{},		28,	28},  // posix - nothing!?!, unix job-control: quit process and create a "core file". todo: TODO WARNING: may include current contents of memory??-SIGQUIT
        {"GS",	"group_separator",		"???",		']',	{},		29,	29},  //
        {"RS",	"record_separator",		"???",		'^',	{},		30,	30},  //
        {"US",	"unit_separator",		"???",		'_',	{},		31,	31},  //
        {"DEL",	"delete",				"ERASE",	'?',	{},	   127,127},  // BS alternative, how related to tty erase?. erase the last character typed. similar to "BS" // todo: NOT IMPLEMENTED in hot_keys //  BS/ERASE on MacOS?
        {" ",	"space",				"???",	    {},		' ',    32, 32},  // simple space character or blank
        {"\\",	"backslash",			"???",		{},		0x5C,   92, 92}, // simple
    };
    static       Hot_keys 		 hot_keys {
        // my_name	     char sequence
        // first the single key char action keys that are the good old Unix shell standard.
        {"escape",		{find_posix_char_from_posix_name(ascii_posix_map, "ESC"), NO_MORE_CHARS}, HotKeyFunctionCat::navigation_esc,			FieldCompletionNav::esc,			 	FieldIntraNav::na},
        {"eof",			{find_posix_char_from_posix_name(ascii_posix_map, "EOF")}, 			HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::eof,			 	FieldIntraNav::na},
        {"quit",		{find_posix_char_from_posix_name(ascii_posix_map, "QUIT")}, 		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::quit_signal,		FieldIntraNav::na},
        {"interrupt",	{find_posix_char_from_posix_name(ascii_posix_map, "INTR")},			HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::quit_signal,		FieldIntraNav::na},

        {"tab",			{find_posix_char_from_posix_name(ascii_posix_map, "TAB")},   		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::skip_one_field, 	FieldIntraNav::na},

        {"CR-CTL-M",    {find_posix_char_from_posix_name(ascii_posix_map, "CR")}, 			HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::down_one_field, 	FieldIntraNav::na},
        {"LF-CTL-J",	{find_posix_char_from_posix_name(ascii_posix_map, "NL-LF")},		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::down_one_field, 	FieldIntraNav::na},

        {"kill",		{find_posix_char_from_posix_name(ascii_posix_map, "KILL")},			HotKeyFunctionCat::nav_intra_field,			FieldCompletionNav::na,		 			FieldIntraNav::kill},
        {"backspace_left_erase", {find_posix_char_from_posix_name(ascii_posix_map, "BS")},	HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na,					FieldIntraNav::backspace_left_erase},
        {"erase_left",  {find_posix_char_from_posix_name(ascii_posix_map, "ERASE")},		HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na,					FieldIntraNav::erase_left},  // todo: is this correct for macOS or Linux or PC??

        // Secondly the multicharacter ESC sequences for the XTERM initially and then VT100 or ANSI.SYS? keyboard, which might be different as in "termcap" on some other hardware.
        // XTERM https://en.wikipedia.org/wiki/ANSI_escape_code#Terminal_input_sequences
        {"f1",			{CSI_ESC,'O','P'}, 				HotKeyFunctionCat::help_popup, 				FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f2",			{CSI_ESC,'O','Q'}, 				HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f3",			{CSI_ESC,'O','R'}, 				HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f4",			{CSI_ESC,'O','S'}, 				HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"up_arrow",	{CSI_ESC,'[','A'}, 				HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::browse_up, 		FieldIntraNav::na},
        {"down_arrow",	{CSI_ESC,'[','B'}, 				HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::browse_down, 	FieldIntraNav::na},
        {"right_arrow",	{CSI_ESC,'[','C'}, 				HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::move_right},
        {"left_arrow",	{CSI_ESC,'[','D'}, 				HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::move_left},
        {"end",			{CSI_ESC,'[','F'}, 				HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::goto_end},
        {"key_pad_5",	{CSI_ESC,'[','G'}, 				HotKeyFunctionCat::na,			 		FieldCompletionNav::na, 			FieldIntraNav::na},
        {"home",		{CSI_ESC,'[','H'}, 				HotKeyFunctionCat::nav_intra_field,			FieldCompletionNav::na,			 	FieldIntraNav::goto_begin},
        {"shift-tab",	{CSI_ESC,'[','Z'}, 				HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::up_one_field, 	FieldIntraNav::na},
        // VT100/220
        {"insert",		{CSI_ESC,'[',    '2','~'}, 			HotKeyFunctionCat::editing_mode,			FieldCompletionNav::na,			 	FieldIntraNav::na},
        {"delete",		{CSI_ESC,'[',    '3','~'}, 			HotKeyFunctionCat::nav_intra_field,			FieldCompletionNav::na, 			FieldIntraNav::delete_char},
        {"pageup",	  	{CSI_ESC,'[',    '5','~'}, 			HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::page_up,		FieldIntraNav::na},
        {"pagedown",	{CSI_ESC,'[',    '6','~'}, 			HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::page_down,		FieldIntraNav::na},
        {"f5",			{CSI_ESC,'[','1','5','~'}, 		HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f6",			{CSI_ESC,'[','1','7','~'}, 		HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 54 '6'
        {"f7",			{CSI_ESC,'[','1','8','~'}, 		HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f8",			{CSI_ESC,'[','1','9','~'}, 		HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f9",			{CSI_ESC,'[','2','0','~'}, 		HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f10",			{CSI_ESC,'[','2','1','~'}, 		HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // todo: is this like ESC or EOF_CHAR?
        {"f11",			{CSI_ESC,'[','2','3','~'}, 		HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 50
        {"f12",			{CSI_ESC,'[','2','4','~'},  	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        // VT100/220, but not on USA PC AT keyboard
        {"f13",			{CSI_ESC,'[','2','5','~'},  	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f14",			{CSI_ESC,'[','2','6','~'},  	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f15",			{CSI_ESC,'[','2','7','~'},  	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f16",			{CSI_ESC,'[','2','8','~'},  	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f17",			{CSI_ESC,'[','3','1','~'},  	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f18",			{CSI_ESC,'[','3','2','~'},  	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f19",			{CSI_ESC,'[','3','3','~'},  	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"f20",			{CSI_ESC,'[','3','4','~'},  	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        // ;2 is shift
        //{"shf-insert",	{CSI_ESC,'[','2',	 ';','2','~'}, 	HotKeyFunctionCat::editing_mode,			FieldCompletionNav::na,			 	FieldIntraNav::na},
        {"shf-delete",		{CSI_ESC,'[','3', 	 ';','2','~'},	HotKeyFunctionCat::nav_intra_field,			FieldCompletionNav::na, 			FieldIntraNav::delete_char},
        //{"shf-pageup",	{CSI_ESC,'[','5',    ';','2','~'}, 	HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::page_up,		FieldIntraNav::na},
        //{"shf-pagedown",	{CSI_ESC,'[','6',    ';','2','~'}, 	HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::page_down,		FieldIntraNav::na},
        {"shf-up_arrow",	{CSI_ESC,'[','1',    ';','2','A'},	HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::browse_up, 		FieldIntraNav::na},
        {"shf-down_arrow",	{CSI_ESC,'[','1',    ';','2','B'},	HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::browse_down, 	FieldIntraNav::na},
        {"shf-right_arrow",	{CSI_ESC,'[','1',    ';','2','C'},	HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::move_right},
        {"shf-left_arrow",	{CSI_ESC,'[','1',    ';','2','D'},	HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::move_left},
        //{"shf-keypad_5",	{'5'},				 				HotKeyFunctionCat::na,			 		FieldCompletionNav::na, 			FieldIntraNav::na},
        //???{"shf-home",	{CSI_ESC,'[','1',    ';','2','H'}}, HotKeyFunctionCat::nav_intra_field,			FieldCompletionNav::na,			 	FieldIntraNav::goto_begin},
        //???{"shf-end",	{CSI_ESC,'[','1',    ';','2','F'}}, HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::goto_end},
        {"sht-f1",			{CSI_ESC,'[','1',    ';','2','P'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"sht-f2",			{CSI_ESC,'[','1',    ';','2','Q'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 54 '6'
        {"sht-f3",			{CSI_ESC,'[','1',    ';','2','R'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"sht-f4",			{CSI_ESC,'[','1',    ';','2','S'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"sht-f5",			{CSI_ESC,'[','1','5',';','2','~'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"sht-f6",			{CSI_ESC,'[','1','7',';','2','~'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 54 '6'
        {"sht-f7",			{CSI_ESC,'[','1','8',';','2','~'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"sht-f8",			{CSI_ESC,'[','1','9',';','2','~'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"sht-f9",			{CSI_ESC,'[','2','0',';','2','~'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        //???{"sht-f10",	{CSI_ESC,'[','2','1',';','2','~'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // todo: is this like ESC or EOF_CHAR?
        {"sht-f11",			{CSI_ESC,'[','2','3',';','2','~'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 50
        {"sht-f12",			{CSI_ESC,'[','2','4',';','2','~'},  HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
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
        {"ctl-keypad_5",	{CSI_ESC,'[','1',    ';','5','E'},		HotKeyFunctionCat::na, 						FieldCompletionNav::na, 			FieldIntraNav::na},
        //???{"ctl-home",	{CSI_ESC,'[','1',    ';','5','H'}}, 					HotKeyFunctionCat::nav_intra_field,			FieldCompletionNav::na,			 	FieldIntraNav::goto_begin},
        //???{"ctl-end",	{CSI_ESC,'[','1',    ';','5','F'}}, 					HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::goto_end},
        {"ctl-f1",			{CSI_ESC,'[','1',    ';','5','P'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"ctl-f2",			{CSI_ESC,'[','1',    ';','5','Q'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 54 '6'
        {"ctl-f3",			{CSI_ESC,'[','1',    ';','5','R'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"ctl-f4",			{CSI_ESC,'[','1',    ';','5','S'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"ctl-f5",			{CSI_ESC,'[','1','5',';','5','~'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"ctl-f6",			{CSI_ESC,'[','1','7',';','5','~'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 54 '6'
        {"ctl-f7",			{CSI_ESC,'[','1','8',';','5','~'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"ctl-f8",			{CSI_ESC,'[','1','9',';','5','~'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        {"ctl-f9",			{CSI_ESC,'[','2','0',';','5','~'}, 	HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        //{"ctl-f10",		{CSI_ESC,'[','2','1',';','5','~'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // todo: is this like ESC or EOF_CHAR?
        {"ctl-f11",			{CSI_ESC,'[','2','3',';','5','~'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 50
        {"ctl-f12",			{CSI_ESC,'[','2','4',';','5','~'},  HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},

        // ;3 is alt
        {"alt-insert",		{CSI_ESC,'[','2',';','3','~'}, 		HotKeyFunctionCat::editing_mode,			FieldCompletionNav::na,			 	FieldIntraNav::na},
        {"alt-delete",		{CSI_ESC,'[','3',';','3','~'}, 		HotKeyFunctionCat::nav_intra_field,			FieldCompletionNav::na, 			FieldIntraNav::delete_char},
        {"alt-pageup",		{CSI_ESC,'[','5',';','3','~'}, 		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::page_up,		FieldIntraNav::na},
        {"alt-pagedown",	{CSI_ESC,'[','6',';','3','~'}, 		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::page_down,		FieldIntraNav::na},
        {"alt-up_arrow",	{CSI_ESC,'[','1',';','3','A'},		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::browse_up, 		FieldIntraNav::na},
        {"alt-down_arrow",	{CSI_ESC,'[','1',';','3','B'},		HotKeyFunctionCat::nav_field_completion,	FieldCompletionNav::browse_down, 	FieldIntraNav::na},
        {"alt-right_arrow",	{CSI_ESC,'[','1',';','3','C'},		HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::move_right},
        {"alt-left_arrow",	{CSI_ESC,'[','1',';','3','D'},		HotKeyFunctionCat::nav_intra_field, 		FieldCompletionNav::na, 			FieldIntraNav::move_left},
        {"alt-keypad_5",	{CSI_ESC,'[','1',';','3','E'},		HotKeyFunctionCat::na, 						FieldCompletionNav::na, 			FieldIntraNav::na},
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
        //{"alt-f10",			{CSI_ESC,'[','2','1',';','3','~'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // todo: is this like ESC or EOF_CHAR?
        //{"alt-f11",			{CSI_ESC,'[','2','3',';','3','~'}, 	HotKeyFunctionCat::na, 					FieldCompletionNav::na, 			FieldIntraNav::na}, // note skipped 50
        //{"alt-f12",			{CSI_ESC,'[','2','4',';','3','~'},  HotKeyFunctionCat::na,					FieldCompletionNav::na, 			FieldIntraNav::na},
        // ;6 is ctl-shf
        // ?? is ctl-alt
        // ?? is shf-alt
         *  todo: where did I get the idea of these next 4?  Not consistent with what I have above.
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
    if ( static bool once {false}; !once) {
        once = true;
        if ( hot_keys.empty() ) {
            assert( false && "consider_hot_key() lib_tty logic error: we don't allow empty hotkeys");
        }
        std::sort( hot_keys.begin(), hot_keys.end() );
        if ( auto dup = std::adjacent_find( hot_keys.begin(), hot_keys.end(), is_hk_chars_equal ); dup != hot_keys.end() ) {
            LOGGERS( "Duplicate hot_key:", dup->my_name);
            assert( false && "Logic error: we don't allow duplicate hotkey character sequences" );
        }
#ifdef GR_DEBUG
        // todo??: make it accept operator<< : LOGGERS( "Run once only now, here are the hot_keys,characters:",hot_keys)
        LOGGER_( "Run once only now, here are the hot_keys,characters:")
        for (auto & i : hot_keys) {
            cerr << std::setw(18) << i.my_name;
            //print_vec(i.characters);
            cerr <<"[" << i.characters <<"]";
            cerr << endl;
        }
        cout << "\b\b. " << endl;
#endif
    }

    LOGGERS( "candidate_hk_chars: ", candidate_hk_chars); //LOGGER_(print_vec(candidate_hk_chars));

    Hot_key const candidate_hk 		{ {}, candidate_hk_chars };
    auto    const lower_bound_itr = std::lower_bound( hot_keys.begin(), hot_keys.end(), candidate_hk );  LOGGERS( "lower_bount_itr:", lower_bound_itr->my_name); LOGGERS( "Characters within above", lower_bound_itr->characters); //print_vec(lower_bound_itr->characters); cerr << "." << endl;

    bool const no_match_end  	{ lower_bound_itr == hot_keys.end() }; // due to the fact we skipped all values, we have one, of several reasons for not having a match.
    bool const partial_match 	{ !no_match_end &&
                                      std::equal( candidate_hk_chars.begin(),	// not the same as == since length of candidate is not length of hot_key
                                                  candidate_hk_chars.end(),
                                                  lower_bound_itr->characters.begin(),
                                                  std::next( lower_bound_itr->characters.begin(), candidate_hk_chars.size() ) )  // check at most the min of both lengths.
                                };
    bool const full_match		{ partial_match &&
                                    lower_bound_itr->characters.size() == candidate_hk_chars.size()
                                };
    // *** NOTE: we still might not have a match.  The if below determines final determination of no_match.
    if      ( full_match )    return *lower_bound_itr;
    else if ( partial_match ) return E_PARTIAL_MATCH;
    return E_NO_MATCH;
}

bool is_usable_char( KbFundamentalUnit const kbc, bool const is_allow_control_chars = false ) {
    // assert( !( is_allow_control_chars && is_password) && "Only one may be true at a time.") ;  //todo: fix this
    int const i	{ static_cast<int>(kbc) };
    return is_allow_control_chars ? ( isprint(i) || iscntrl(i) ) && !( isspace(i) || i==17 || i==19 ) 	// allowing control chars in pw, except: spaces and XON & XOFF,
                                                                                                            // but note some may have been parsed out as hot_keys prior to this test.
                                  :   isprint(i);
}

//Kb_key_a_fstat get_kb_key( [[maybe_unused]] bool const is_strip_control_chars) {
//Kb_key_a_fstat get_kb_key( bool const is_strip_control_chars [[maybe_unused]] ) {
Kb_key_a_fstat get_kb_key( bool const is_strip_control_chars ) {  // todo: use the parameter or get rid of it????
    Hot_key_chars hkc {};
    for ( Simple_key_char first_skc {};
          first_skc = 0, cin.get( first_skc ), hkc.push_back( first_skc == CSI_ALT ? CSI_ESC : first_skc ), true;
        ) {
        File_status file_status { File_status::other };
        if ( cin.eof() || first_skc == 0) {
            assert( (cin.eof() || first_skc == 0) && "we probably don't handle eof well."); // todo: more eof handling needed
            file_status = File_status::eof_file_descriptor;
            return { hkc, file_status};
        };
        if ( first_skc == CSI_ESC ) {
            Termios terminal_status_orig { termio_set_timer( VTIME_ESC ) }; // return with no char within timer, if not a multicharacter ESC sequence
            Simple_key_char timed_test_char {} ;
            cin.get( timed_test_char );  				// see if we get chars too quickly to come from a human, but instead is a multibyte sequence.
            /* if ( cin.eof() ) {  // todo: this appears to be triggered by ESC alone, ie. the time expires.  Had thought that just the char would be 0.
                assert( (cin.eof()) && "post timer, we probably don't handle eof well."); // todo: more eof handling needed
                file_status = File_status::eof_file_descriptor;
                termio_restore( terminal_status_orig );
                return { hkc, file_status};
            }; */
            termio_restore( terminal_status_orig );
            if ( timed_test_char == TIMED_NULL_GET ) {  // todo: magic number // no kbc immediately available within waiting time. NOTE: Must do this check first! if we didn't get another char within prescribed time, it is just a single ESC!
                hkc.push_back( NO_MORE_CHARS );  // todo: magic number // add a flag value to show a singular ESC
                cin.clear();  // todo: required after a timer failure has been triggered? Seems to be, why? // note: we have no char to "putback"!
            } else
                cin.putback( timed_test_char ); 		// part of an ESC multibyte sequence, so we will need it next loop iteration!  The CSI_ESC will be a partial match and later we pick up the other characters.
        }
        Hotkey_o_errno const k = consider_hot_key( hkc );  // todo: consider using ref for speed?
        if ( std::holds_alternative<Hot_key>(k) )
            return { std::get<Hot_key>(k),         File_status::other };
        else {
            assert ( std::holds_alternative<Lt_errno>(k) && "We have an Lt_errno." );
            switch ( std::get<Lt_errno>(k) ) {
            case E_PARTIAL_MATCH:
                continue;
            case E_NO_MATCH:
                if ( hkc.size() == 1 )
                    return { hkc[0] , File_status::other };
                // **** this is the Hot_key_chars case of the variant return value  // todo: should we throw away or putback?
                // std::for_each( hkc.rend(), std::prev(hkc.rbegin()), [](Simple_key_char i){cin.putback(i);});  // all except first one.  todo: how many can I putback in this implementation?  Is it even a good idea?
                // hkc.clear();
                // Hot_key_chars const hot_key_chars_unfound { hkc.begin(), hkc.end() };
                return { hkc, File_status::unexpected_data };
            }
        }
      } // * end loop *
    assert(false && "We should never get here.");
}

bool is_ignore_key_file_status( File_status const file_status ) { // **** CASE on File Status
    switch (file_status) {
    case File_status::other : LOGGER_( "File_status is: other."); //
        return false;
    case File_status::eof_simple_key_char : LOGGER_( "File_status is: keyboard eof, which is a hotkey."); //
        return false;
    case File_status::timed_out :
        cout << "\ais_ignore_key_file_status: keyboard timeout, try again.";
        break;
    case File_status::unexpected_data :
        cout << "\ais_ignore_key_file_status: bad keyboard character sequence, try again."; // we throw away bad character sequence or char // todo: handle scrolling and dialog
        break;
    case File_status::eof_file_descriptor :
        assert( false && "is_ignore_key_file_status: file descriptor is at eof.");  // todo: is this correct, or should we not ignore it?
        break;
    }
    return true;  // we add back the input character that we have decided to ignore.
}

bool is_ignore_hotkey_function_cat( HotKeyFunctionCat const hot_key_function_cat ) {
                                            //LOGGER_( "get_kb_keys_raw(): Got a Hot_key: "<<hot_key_rv.my_name<<endl;
        // **** CASE on hot key Function Category
        switch ( hot_key_function_cat ) {
        case HotKeyFunctionCat::other :
            LOGGER_( "HotKeyFunctionCat::other");
            cout << "\aLast key press not handled here, so ignored.\n";
            assert(false && "\aHotKeyFunctionCat::other not sure about what cases are and how to handle." );  // todo:
            return false;
            break;
        case HotKeyFunctionCat::nav_field_completion :
            LOGGER_("Navigation completion.");
            break;
        case HotKeyFunctionCat::editing_mode :
                                            // todo: how do we use this?  do we ignore the character?? is_editing_mode_insert = ! is_editing_mode_insert;
            LOGGER_("get_kb_keys_raw(): Editing mode toggled.");
            break;
        // All subsequent cases appear to be exactly the same except for debugging messages.
        case HotKeyFunctionCat::navigation_esc :
            LOGGER_("get_kb_keys_raw(): Navigation ESC");
            break;
        case HotKeyFunctionCat::nav_intra_field :
            LOGGER_( "Nav_intra_field mode");
            // todo:  clarify comment: only done for get_value_raw() above, here we let the caller do it: nav_intra_field( hot_key, value, value_index, echo_skc_to_tty);
            break;
        case HotKeyFunctionCat::job_control :
            LOGGER_("Job control");
            break;
        case HotKeyFunctionCat::help_popup :
            LOGGER_("Help Pop-Up");
            break;
        case HotKeyFunctionCat::na :
            LOGGER_("HotKeyFunctionCat::na");
            break;
        }
    return false;
}

// true if we disallow the character
bool is_ignore_key_skchar( Simple_key_char const skc,
                           bool const is_echo_skc_to_tty = true,
                           bool const is_allow_control_chars = false,
                           bool const is_ring_bell = true) {
    LOGGERS("get_kb_keys_raw(): Got a Simple_key_char: ", static_cast<int>(skc));
    if ( is_usable_char( skc, is_allow_control_chars )) {
        if ( is_echo_skc_to_tty )
            cout << skc <<endl;
        //value_rv += skc; //value_index++; // todo: index would have to be used if value is not a string, but instead an vector<KbFundamentalUnit>
        return false;
    }
    else {
        if ( is_ring_bell )
            cout << "\a";
        cout << "Last key press invalid/unusable here, so ignored."<<endl;
        return true;
    }
    assert(false && "we never get here.");
}

Kb_value_plus get_kb_keys_raw(size_t const length_in_simple_key_chars,
                              bool const   is_require_field_completion_key,
                              bool const   is_echo_skc_to_tty,
                              bool const   is_strip_control_chars,
                              bool const   is_password ) {
    assert( length_in_simple_key_chars > 0 && "too small get length." );   // todo: can debug n>1 case later.
    Kb_regular_value 	value_rv 				{}; //  *** need to load the 3 "_rv" vars below
    Hot_key		 		hot_key_rv				{};
    File_status  		file_status_rv			{File_status::other};
    size_t 		 		additional_skc 			{ length_in_simple_key_chars };  // todo: we presume that bool is worth one and it is added for the CR we require to end the value of specified length.
    HotKeyFunctionCat   hot_key_function_cat  	{HotKeyFunctionCat::na};			// reset some variables from prior loop if any, specifically old/prior hot_key.
    //unsigned 			int value_index			{0}; // Note: Points to the character beyond the current character (presuming zero origin), like an STL iterator it.end(), hence 0 == empty field.
    //bool 		 		is_editing_mode_insert {true};
    Termios const terminal_status_orig 	{ termio_set_raw() };
    do {  // *** begin loop *** 	// Gather char(s) to make a value until we get a "completion" Hot_key, or number of chars, or error.
        hot_key_rv 			 = {};  							// reset some variables from prior loop if any, specifically old/prior hot_key.
        hot_key_function_cat = {HotKeyFunctionCat::na};			// reset some variables from prior loop if any, specifically old/prior hot_key.
        bool is_ignore_key_skc {false}, is_ignore_key_hk {false}, is_ignore_key_fd {false};  // todo: don't seem to need these variables, but think I might.

        Kb_key_a_fstat kb_key_a_fstat { get_kb_key( is_strip_control_chars ) }; //--additional_skc, additional_skc > 0 ? kb_key_a_fstat = get_kb_key( false ), nullptr : nullptr //--additional_skc, additional_skc > 0 && static_cast<bool>( ( kb_key_a_fstat = get_kb_key( false ) ).second ) //--additional_skc, additional_skc > 0 ? kb_key_a_fstat = get_kb_key( false ), nullptr : nullptr
        file_status_rv 		  		= kb_key_a_fstat.second;
        if ( ! (is_ignore_key_fd = is_ignore_key_file_status( file_status_rv )) )
        {
            if      ( std::holds_alternative< Simple_key_char >( kb_key_a_fstat.first )) {
                Simple_key_char const skc { std::get < Simple_key_char >( kb_key_a_fstat.first ) };
                if ( ! ( is_ignore_key_skc = is_ignore_key_skchar( skc, is_echo_skc_to_tty, is_password )) )
                    value_rv += skc;
            }
            else if ( std::holds_alternative< Hot_key > ( kb_key_a_fstat.first )) {
                hot_key_rv 			 = std::get < Hot_key >( kb_key_a_fstat.first );  // TODO:?? is this a copy?
                hot_key_function_cat = hot_key_rv.function_cat;
                //                if ( ( hot_key_function_cat = hot_key_rv.function_cat ) == HotKeyFunctionCat::editing_mode ) {
                //                    is_editing_mode_insert 	= ! is_editing_mode_insert;  // todo: Do we use this value here, or down the call stack?
                //                    cerr << "get_kb_keys_raw() function_cat: Editing mode is insert: "<<is_editing_mode_insert << endl;
                //                }
                is_ignore_key_hk     = is_ignore_hotkey_function_cat( hot_key_function_cat );
            }
            else  assert( false && "ERROR:Not sure why we got here, we require that either a Simple_key_char or a Hot_key entered."); // todo:  I think this else clause is not needed. cerr << "\aget_kb_keys_raw(): throwing away this key stroke, trying again to get one." << endl;
        }
        if ( !is_ignore_key_fd || !is_ignore_key_skc || !is_ignore_key_hk )
            --additional_skc;
    } while ( additional_skc >  0 && // *** end loop ***
              file_status_rv != File_status::eof_simple_key_char &&
              file_status_rv != File_status::eof_file_descriptor &&
              hot_key_function_cat == HotKeyFunctionCat::na ); // todo: also NEED TO HANDLE hot_key_chars alone?  eof of both types?  intrafield?  editing mode? monostate alone

    while ( is_require_field_completion_key &&
            file_status_rv          != File_status::eof_file_descriptor   &&
            hot_key_rv.function_cat != HotKeyFunctionCat::nav_field_completion &&  // todo: may need more cats like intra_field, editing_mode?
            hot_key_rv.function_cat != HotKeyFunctionCat::navigation_esc ) { // prior: hot_key_rv.f_completion_nav != FieldCompletionNav::down_one_field ) {
        Kb_key_a_fstat const kb_key_a_fstat = get_kb_key( is_strip_control_chars );
        Kb_key_optvariant const k 			= kb_key_a_fstat.first;
        file_status_rv                      = kb_key_a_fstat.second;
        if ( std::holds_alternative< Hot_key >( k ) )
            hot_key_rv = std::get< Hot_key >( k );
    }
    termio_restore( terminal_status_orig );
    return { value_rv, hot_key_rv, file_status_rv };  // NOTE: we designed to copy OUT these variables.
}

}
