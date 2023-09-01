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
#define LOGGER_( msg )   using loc = std::source_location;std::cerr<<"["<<loc::current().file_name()<<':'<<loc::current().line()<<','<<loc::current().column()<<"]`"<<loc::current().function_name()<<"`:" <<#msg<<".\n";
#define LOGGERS( msg, x )using loc = std::source_location;std::cerr<<"["<<loc::current().file_name()<<':'<<loc::current().line()<<','<<loc::current().column()<<"]`"<<loc::current().function_name()<<"`:" <<#msg<<",{"<<x<<"}.\n";

///  This main() is used to test our linked shared library: lib_tty.so
int main ( int argc, char* arv[] ) {
    string my_arv { *arv};
    cout << "argc,argv:"<<argc<<","<<my_arv<<endl;

    string c {};

    // Next we test raw character input, grabbing individual keyboard key presses, including multi-character sequences like F1 and Insert keys.
    do {
        cout << "Enter a single keyboard key press now! (q to quit):"; cout.flush();
        auto [kb_regular_value, hot_key, file_status] = Lib_tty::get_kb_keys_raw( 1, false, true, true, false );

        //Lib_tty::Kb_value_plus key {Lib_tty::get_kb_keys_raw( 1, true, true, true, false )};
        //Lib_tty::Kb_regular_value krv   = std::get<0>( key );
        //Lib_tty::Hot_key hk             = std::get<1>( key );
        //Lib_tty::File_status fs         = std::get<2>( key );

        LOGGER_("" );
        LOGGER_("We got this in 3 variables below:" );
        LOGGERS("kb_regular_value:", kb_regular_value);
        LOGGERS("hot_key:", hot_key.my_name);
        LOGGERS("file_status:", (int) file_status);

        c = kb_regular_value;

    } while ( c != "q");

    // todo: Test other use cases of lib_tty.

    cout << "###" << endl;
}
