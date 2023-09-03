/*  Used to test lib_tty.so
 *  Testing should be expanded considerably.
 *  We test raw character input, grabbing individual keyboard key presses, including multi-character sequences like F1 and Insert keys.
 */

//#include <bits/stdc++>  // todo??: why is .h needed?
#include <bits/stdc++.h>
#include "lib_tty.h"

using std::endl, std::cin, std::cout, std::cerr, std::string;


/// define if asserts are NOT to be checked.  Put in *.h file not *.CPP
//#define 	NDEBUG
/// define I'm Debugging LT.  Put in *.h file not *.CPP
#define  	GR_DEBUG
//#undef  	GR_DEBUG
//#ifdef   	GR_DEBUG
//#endif  # GR_DEBUG
#define LOGGER_( msg )   using loc = std::source_location;std::cerr<<"["<<loc::current().file_name()<<':'<<std::setw(3)<<loc::current().line()<<','<<std::setw(2)<<loc::current().column()<<"]`"<<loc::current().function_name()<<"`:" <<#msg<<".\n";
#define LOGGERS( msg, x )using loc = std::source_location;std::cerr<<"["<<loc::current().file_name()<<':'<<std::setw(3)<<loc::current().line()<<','<<std::setw(2)<<loc::current().column()<<"]`"<<loc::current().function_name()<<"`:" <<#msg<<",{"<<x<<"}.\n";

///  This main() is used solely to test our linked shared library: lib_tty.so
int main ( int argc, char* arv[] ) { string my_arv { *arv}; cout << "~~~ argc,argv:"<<argc<<","<<my_arv<<"."<<endl;
    Lib_tty::Kb_regular_value   kbrv {};
    Lib_tty::Hot_key            hk {};
    Lib_tty::File_status        fs {};
    // Test raw character input, grabbing individual keyboard key presses, including multi-character sequences like F1 and Insert keys.
    do { cout << "Enter a single keyboard key press now! (q or F4 to quit):"; cout.flush();
        //auto [kb_regular_value, hot_key, file_status] = Lib_tty::get_kb_keys_raw( 1, false, true, true, false );
        Lib_tty::Kb_value_plus kvp {Lib_tty::get_kb_keys_raw( 1, false, true, true, false )};
        kbrv = std::get< Lib_tty::Kb_regular_value >( kvp );
        hk   = std::get< Lib_tty::Hot_key >         ( kvp );
        fs   = std::get< Lib_tty::File_status >     ( kvp );
        LOGGER_("" ); LOGGER_("We got this in 3 variables below:" );
        LOGGERS("kb_regular_value:", kbrv); LOGGERS("hot_key:", hk.my_name); LOGGERS("file_status:", (int) fs);
    //} while ( kbrv != "q" && hk.my_name != "f4");
    } while ( kbrv != "q" && hk.my_name != "f4");
    do { cout << "Enter a sequence of 3 key strokes, including possibly some function_keys. (qqq or ??F4):"; cout.flush();
        Lib_tty::Kb_value_plus kvp { Lib_tty::get_kb_keys_raw( 3, false, true, true, false )};
        kbrv = std::get< Lib_tty::Kb_regular_value >( kvp );
        hk   = std::get< Lib_tty::Hot_key >         ( kvp );
        fs   = std::get< Lib_tty::File_status >     ( kvp );
        LOGGER_("" ); LOGGER_("We got this in 3 variables below:" );
        LOGGERS("kb_regular_value:", kbrv); LOGGERS("hot_key:", hk.my_name); LOGGERS("file_status:", (int) fs);
    } while ( kbrv != "qqq" && hk.my_name != "f4");

    // todo: Test other use cases of get_kb_keys_raw().
    // todo: Test other use cases of the library.
    // test
    cout << "###" << endl;
}
