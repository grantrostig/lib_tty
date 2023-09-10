/*
 * Copyright (c) 2019 Grant Rostig all rights reserved,  grantrostig.com
 * BOOST 1.0 license
 *
 *  Used to test lib_tty.so
 *  Testing should be expanded considerably.
 *  We test raw character input, grabbing individual keyboard key presses, including multi-character sequences like F1 and Insert keys.
 */

//#include <bits/stdc++>  // todo??: why is .h needed?
//#include <bits/stdc++.h>
#include "lib_tty.h"
#include <stacktrace>
#include <source_location>
#include <iostream>

using std::cin; using std::cout; using std::cerr; using std::clog; using std::endl; using std::string;  // using namespace std;
using namespace std::string_literals;;

string source_loc() {
    using loc = std::source_location;
    //using ts = std::to_string;  // todo??: why not?  alternative approach?
    std::string result {"\n"s+loc::current().file_name() +":"s +std::to_string(loc::current().line()) +std::to_string(loc::current().column())+"]`"s +loc::current().function_name()+"`."s};
    return result;
}

/// define if asserts are NOT to be checked.  Put in *.h file not *.CPP
//#define 	NDEBUG
/// define I'm Debugging LT.  Put in *.h file not *.CPP
#define  	GR_DEBUG
//#undef  	GR_DEBUG
//#ifdef   	GR_DEBUG
//#endif  # GR_DEBUG
#define LOGGER_( msg )   using loc = std::source_location;std::cerr<<"\n\r["<<loc::current().file_name()<<':'<<std::setw(3)<<loc::current().line()<<','<<std::setw(2)<<loc::current().column()<<"]`"<<loc::current().function_name()<<"`:" <<#msg<<".\n";
#define LOGGERS( msg, x )using loc = std::source_location;std::cerr<<"\n\r["<<loc::current().file_name()<<':'<<std::setw(3)<<loc::current().line()<<','<<std::setw(2)<<loc::current().column()<<"]`"<<loc::current().function_name()<<"`:" <<#msg<<",{"<<x<<"}.\n";

auto crash_tracer(int const signal_number) ->void {
    cout << "CRASH_ERROR: signal#, stack trace:{" << signal_number << "},{" << std::stacktrace::current() << "}<<<END CRASH_ERROR STACK_TRACE.\n"; // We want the user to see this error
    std::string reply; cout << "q for exit(1) or CR to continue:"; cout.flush(); cin.clear(); getline(cin, reply); if ( reply == "q") exit(1);
}

auto crash_signals_register() -> void {    // signals that cause "terminate" and sometimes "core dump"  https://en.wikipedia.org/wiki/Signal_(IPC)
    std::signal( SIGABRT, crash_tracer );
    std::signal( SIGALRM, crash_tracer );
    std::signal( SIGBUS,  crash_tracer );
    std::signal( SIGFPE,  crash_tracer );
    std::signal( SIGHUP,  crash_tracer );
    std::signal( SIGILL,  crash_tracer );
    std::signal( SIGINT,  crash_tracer );
    std::signal( SIGKILL, crash_tracer );
    std::signal( SIGPIPE, crash_tracer );
    std::signal( SIGPOLL, crash_tracer );
    std::signal( SIGPROF, crash_tracer );
    std::signal( SIGQUIT, crash_tracer );
    std::signal( SIGSEGV, crash_tracer );
    std::signal( SIGSYS,  crash_tracer );
    std::signal( SIGTERM, crash_tracer );
    std::signal( SIGTRAP, crash_tracer );
    std::signal( SIGUSR1, crash_tracer );
    std::signal( SIGUSR2, crash_tracer );
    std::signal( SIGVTALRM, crash_tracer );
    std::signal( SIGXCPU, crash_tracer );
    std::signal( SIGXFSZ, crash_tracer );
    std::signal( SIGVTALRM, crash_tracer );
}

/**  This main() is used solely to test our linked shared library: lib_tty.so
 *   WARNING enable this main.cpp file in qmake ONLY if you want to run this test, but to build the libary DON'T enable this file to be linked into the *.so
 */
int main ( int argc, char* arv[] ) { string my_arv { *arv}; cout << "~~~ argc,argv:"<<argc<<","<<my_arv<<"."<<endl;
    // Test raw character input, grabbing individual keyboard key presses, including multi-character sequences like F1 and Insert keys.
    cout << "Prepare for several tests of lib_tty:\n";
    cin.exceptions( std::istream::failbit);  // throw on fail of cin.
    crash_signals_register();
    Lib_tty::Kb_regular_value   kbrv {};
    Lib_tty::Hot_key            hk {};
    Lib_tty::File_status        fs {};
    string                      user_ack {};
    // Test raw character input, grabbing individual keyboard key presses, including multi-character sequences like F1 and Insert keys.
    do { cout << "Enter a single keyboard key press now! (q or F4 for next test):"; cout.flush();
        Lib_tty::Kb_value_plus kvp {Lib_tty::get_kb_keys_raw( 1, false, true, true)};
        kbrv = std::get< Lib_tty::Kb_regular_value >( kvp );
        hk   = std::get< Lib_tty::Hot_key >         ( kvp );
        fs   = std::get< Lib_tty::File_status >     ( kvp );
        LOGGER_("~~~" ); LOGGER_("We got this in 3 variables below:" );
        LOGGERS("kb_regular_value:", kbrv); LOGGERS("hot_key:", hk.my_name); LOGGERS("file_status:", (int) fs);
        cout << "Press return to continue (q to exit(0)):"; getline( cin, user_ack); cin.clear(); cout <<"got this from continue:"<<user_ack<<endl; if ( user_ack == "q") exit(0);
    } while ( kbrv != "q" && hk.my_name != "f4");

    do { cout << "Enter a sequence of 3 key strokes, including possibly some function_keys. (qqq or ??F4 for next test):"; cout.flush();
        Lib_tty::Kb_value_plus kvp { Lib_tty::get_kb_keys_raw( 3, false, true, true)};
        kbrv = std::get< Lib_tty::Kb_regular_value >( kvp );
        hk   = std::get< Lib_tty::Hot_key >         ( kvp );
        fs   = std::get< Lib_tty::File_status >     ( kvp );
        LOGGER_("~~~" ); LOGGER_("We got this in 3 variables below:" );
        LOGGERS("kb_regular_value:", kbrv); LOGGERS("hot_key:", hk.my_name); LOGGERS("file_status:", (int) fs);
        cout << "Press return to continue (q to exit(0)):"; getline( cin, user_ack); cin.clear(); cout <<"got this from continue:"<<user_ack<<endl; if ( user_ack == "q") exit(0);
    } while ( kbrv != "qqq" && hk.my_name != "f4");

    do { cout << "Enter a sequence of 3 key strokes, including possibly some function_keys, ending with a field_completion key stroke. (qqq or ??F4 for next test):"; cout.flush();
        Lib_tty::Kb_value_plus kvp { Lib_tty::get_kb_keys_raw( 3, true, true, true)};
        kbrv = std::get< Lib_tty::Kb_regular_value >( kvp );
        hk   = std::get< Lib_tty::Hot_key >         ( kvp );
        fs   = std::get< Lib_tty::File_status >     ( kvp );
        LOGGER_("~~~" ); LOGGER_("We got this in 3 variables below:" );
        LOGGERS("kb_regular_value:", kbrv); LOGGERS("hot_key:", hk.my_name); LOGGERS("file_status:", (int) fs);
        cout << "Press return to continue (q to exit(0)):"; getline( cin, user_ack); cin.clear(); cout <<"got this from continue:"<<user_ack<<endl; if ( user_ack == "q") exit(0);
    } while ( kbrv != "qqq" && hk.my_name != "f4");

    // todo: Test other use cases of get_kb_keys_raw().
    // todo: Test other use cases of the library.
    cout << "All tests have been run.\n";
    cout << "###" << endl;
}
