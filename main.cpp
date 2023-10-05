/* Copyright (c) 2019 Grant Rostig all rights reserved,  grantrostig.com
 *
 *  Used to test lib_tty.so
 *  Testing should be expanded considerably.
 *  We test raw character input, grabbing individual keyboard key presses, including multi-character sequences like F1 and Insert keystrokes_raw.
 */

#include "lib_tty_internal.hpp"
#include "lib_tty.hpp"
#include <stacktrace>
#include <source_location>
#include <iostream>
#include <variant>
#include <deque>
//#include <bits/stdc++.h>
//#include <bits/stdc++>  // TODO??: why is .h needed?
using std::cin; using std::cout; using std::cerr; using std::clog; using std::endl; using std::string;  // using namespace std;
using namespace std::string_literals;;

string source_loc() {
    using loc = std::source_location;
    //using ts = std::to_string;  // TODO??: why not?  alternative approach?
    std::string result {"\n"s+loc::current().file_name() +":"s +std::to_string(loc::current().line()) +std::to_string(loc::current().column())+"]`"s +loc::current().function_name()+"`."s};
    return result;
}

//template<typename Container>            // utility f() to print vectors
//    requires Insertable<Container>
//std::ostream&
//operator<<( std::ostream & out, Container const & c) {
//    if ( not c.empty()) {
//        out << "[";   // TODO??: add std::setw(3)
//        //out.width(9);
//        //out << std::setw(9);
//        std::copy(c.begin(), c.end(), std::ostream_iterator< typename Container::value_type >(out, ","));
//        //out.width();
//        //out << std::setw();
//        out << "\b]";
//    } else {
//        out << "[CONTAINTER IS EMPTY]";
//    }
//    return out;
//}

/// define if asserts are NOT to be checked.  Put in *.h file not *.CPP
//#define 	NDEBUG
/// define I'm Debugging LT.  Put in *.h file not *.CPP
#define  	GR_DEBUG
//#undef  	GR_DEBUG
//#ifdef   	GR_DEBUG
//#endif  # GR_DEBUG

//auto crash_tracer(int const signal_number) ->void {
//    // TODO??: set tty to sane mode.
//    cout << "CRASH_ERROR: signal#, stack trace:{" << signal_number << "},{" << std::stacktrace::current() << "}<<<END CRASH_ERROR STACK_TRACE.\n"; // We want the user to see this error
//    std::string reply; cout << "CRASH_ERROR: q for exit(1) or CR to continue:"; cout.flush(); cin.clear(); getline(cin, reply); if ( reply == "q") exit(1);
//}

//auto crash_signals_register() -> void {    // signals that cause "terminate" and sometimes "core dump"  https://en.wikipedia.org/wiki/Signal_(IPC)
//    //std::signal( SIGABRT, crash_tracer );
//    std::signal( SIGALRM, crash_tracer );
//    std::signal( SIGBUS,  crash_tracer );
//    std::signal( SIGFPE,  crash_tracer );
//    std::signal( SIGHUP,  crash_tracer );
//    std::signal( SIGILL,  crash_tracer );
//    std::signal( SIGINT,  crash_tracer );
//    std::signal( SIGKILL, crash_tracer );
//    std::signal( SIGPIPE, crash_tracer );
//    std::signal( SIGPOLL, crash_tracer );
//    std::signal( SIGPROF, crash_tracer );
//    std::signal( SIGQUIT, crash_tracer );
//    std::signal( SIGSEGV, crash_tracer );
//    std::signal( SIGSYS,  crash_tracer );
//    std::signal( SIGTERM, crash_tracer );
//    std::signal( SIGTRAP, crash_tracer );
//    std::signal( SIGUSR1, crash_tracer );
//    std::signal( SIGUSR2, crash_tracer );
//    std::signal( SIGVTALRM, crash_tracer );
//    std::signal( SIGXCPU, crash_tracer );
//    std::signal( SIGXFSZ, crash_tracer );
//    std::signal( SIGVTALRM, crash_tracer );
//}

struct Print_visitor_fn_objects {
    int operator() ( std::monostate const & variant_target_acceptor ) {
        cout << "Here is the std::monostate.\n";
        return EXIT_SUCCESS;
    }
    int operator() ( Lib_tty::Key_char_singular const & variant_target_acceptor ) {
        cout << "Here is the Lib_tty::Key_char_singular:" << variant_target_acceptor <<".\n";
        return EXIT_SUCCESS;
    }
    int operator() ( Lib_tty::I18n_key_chars const & variant_target_acceptor ) {
        cout << "Here is the Lib_tty::I18n_key_chars:" << variant_target_acceptor <<".\n";
        return EXIT_SUCCESS;
    }
    int operator() ( Lib_tty::I18n_key_row const & variant_target_acceptor ) {
        cout << "Here is the Lib_tty::I18n_key_row:" << variant_target_acceptor.my_name <<".\n";
        return EXIT_SUCCESS;
    }
    int operator() ( Lib_tty::Hot_key_row const & variant_target_acceptor ) {
        cout << "Here is the Lib_tty::Hot_key_row:" << variant_target_acceptor.my_name <<".\n";
        return EXIT_SUCCESS;
    }
};

/**  This main() is used solely to test our linked shared library: lib_tty.o
 *   WARNING enable this main.cpp file in qmake ONLY if you want to run this test, but to build the libary DON'T enable this file to be linked into the *.so
 */
int main ( int argc, char* arv[] ) { string my_arv { *arv}; cout << "~~~ argc,argv:"<<argc<<","<<my_arv<<"."<<endl;
    //using namespace Lib_tty;
    // Test raw character input, grabbing individual keyboard key presses, including multi-character sequences like F1 and Insert keys.
    cout << "Prepare for several tests of lib_tty:\n";
    //cin.exceptions( std::istream::failbit);  // throw on fail of cin.
    //crash_signals_register();

    std::string                 STRING_Q            {"q"};
    Lib_tty::I18n_key_chars     Q                   {STRING_Q.begin(),STRING_Q.end()};

    std::string                 STRING_QQQ          {"qqq"};
    Lib_tty::I18n_key_chars     QQQ                 {STRING_QQQ.begin(),STRING_QQQ.end()};

    Lib_tty::I18n_key_chars     i18ns               {};
    Lib_tty::Hot_key_row        * hot_key_row_p     {};
    Lib_tty::Hot_key_row        hot_key_row         {};
    Lib_tty::HotKeyFunctionCat  nav                 {};
    Lib_tty::File_status        fs          {};
    string                      user_ack    {};
    // Test raw character input, grabbing individual keyboard key presses, including multi-character sequences like F1 and Insert keys.
    do { cout << "ENTER a single keyboard key press now! (q or F4 for next test):"; cout.flush();
        Lib_tty::Kb_keys        keys                {Lib_tty::get_kb_keystrokes_raw( 1, false, true, true)};
        Lib_tty::Kb_key_variant my_kb_key_variant   = keys.kb_key_a_stati_rows.begin()->kb_key_variant;
        //hot_key_row                                 = std::get<    Lib_tty::Hot_key_row >(   my_kb_key_variant );
        //hot_key_row_p                               = std::get_if< Lib_tty::Hot_key_row >( & my_kb_key_variant );
        for ( Lib_tty::Kb_key_a_stati const & ks : keys.kb_key_a_stati_rows ) {
            std::visit( Print_visitor_fn_objects {}, ks.kb_key_variant);
        };
        nav = keys.hot_key_nav_final;
        LOGGER_("nav:"<<nav);
        fs  = keys.file_status_final;
        LOGGER_("nav:"<<fs);

        LOGGER_("We got this in 3 variables below:" );
        cout<<"MAIN():kb_regular_value,length:{"<< i18ns<<","<<i18ns.size()<<"}, hot_key:"<< hot_key_row.my_name << ", file_status:"<< (int) fs <<"."<<endl;
        cout << "Press RETURN to continue (q to exit(0)):"; getline( cin, user_ack); cin.clear(); cout <<"got this from continue:"<<user_ack<<endl; if ( user_ack == "q") exit(0);
    } while ( i18ns != Q && hot_key_row.my_name != "f4");

    /* do { cout << "ENTER a sequence of 3 key strokes, including possibly some function_keys INTERSPERSED. (qqq or ??F4 for next test):"; cout.flush();
        Lib_tty::Kb_keys kvp { Lib_tty::get_kb_keystrokes_raw( 3, false, true, true)};
        //i18ns = kvp.key_chars_i18n;
        //hk  = kvp.hot_key;
        //fs  = kvp.file_status;
        LOGGER_("~~~" ); LOGGER_("We got this in 3 variables below:" );
        //cout<<"MAIN():kb_regular_value,length:{"<< i18ns<<","<<i18ns.size()<<"}, hot_key:"<< kvp.hot_key.my_name << ", file_status:"<< (int) fs <<"."<<endl;
        //cout << "Press RETURN to continue (q to exit(0)):"; getline( cin, user_ack); cin.clear(); cout <<"got this from continue:"<<user_ack<<endl; if ( user_ack == "q") exit(0);
    //} while ( i18ns != QQQ && hk.my_name != "f4");

    do { cout << "ENTER a sequence of 3 key strokes, including possibly some function_keys, ENDING with a field_completion key stroke. (qqq or ??F4 for next test):"; cout.flush();
        Lib_tty::Kb_keys kvp { Lib_tty::get_kb_keystrokes_raw( 3, true, true, true)};
        //i18ns = kvp.key_chars_i18n;
        //hk  = kvp.hot_key;
        //fs  = kvp.file_status;
        LOGGER_("~~~" ); LOGGER_("We got this in 3 variables below:" );
        //cout<<"MAIN():kb_regular_value,length:{"<< i18ns<<","<<i18ns.size()<<"}, hot_key:"<< kvp.hot_key.my_name << ", file_status:"<< (int) fs <<"."<<endl;
        //cout << "Press RETURN to continue (q to exit(0)):"; getline( cin, user_ack); cin.clear(); cout <<"got this from continue:"<<user_ack<<endl; if ( user_ack == "q") exit(0);
    //} while ( i18ns != QQQ && hk.my_name != "f4"); */

    // TODO: Test other use cases of get_kb_keystrokes_raw().
    // TODO: Test other use cases of the library.
    cout << "All tests have been run.\n";
    cout << "###" << endl;
    return EXIT_SUCCESS;
}
