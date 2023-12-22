/* Copyright (c) 2019 Grant Rostig all rights reserved,  grantrostig.com
 *  WARNING enable this main.cpp file in qmake ONLY if you want to run this test, but to build the libary DON'T enable this file to be linked into the *.so
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
#include <any>
#include <gsl/gsl>
//#include <bits/stdc++.h>
//#include <bits/stdc++>  // TODO??: why is .h needed?
using std::cin; using std::cout; using std::cerr; using std::clog; using std::endl; using std::string;  // using namespace std;
using namespace std::string_literals;;
//#define LOGGER_( );
//#define LOGGERX( );
//#define LOGGER_R( );
//#define LOGGERXR( );
/* //string source_loc() {
//    using loc = std::source_location;
//    //using ts = std::to_string;  // TODO??: why not?  alternative approach?
//    string result {"\n"s+loc::current().file_name() +":"s +std::to_string(loc::current().line()) +std::to_string(loc::current().column())+"]`"s +loc::current().function_name()+"`."s};
//    return result;
//}

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

/// define if asserts are NOT to be checked.
//#define 	NDEBUG
/// define I'm Debugging LT.
//#define  	GR_DEBUG
//#undef  	GR_DEBUG

//auto crash_tracer(int const signal_number) ->void {
//    // TODO??: set tty to sane mode.
//    cout << "CRASH_ERROR: signal#, stack trace:{" << signal_number << "},{" << std::stacktrace::current() << "}<<<END CRASH_ERROR STACK_TRACE.\n"; // We want the user to see this error
//    string reply; cout << "CRASH_ERROR: q for exit(1) or CR to continue:"; cout.flush(); cin.clear(); getline(cin, reply); if ( reply == "q") exit(1);
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
//} */
/** User indicates their completion of this input field. */
bool is_completed_char_input( Lib_tty::Kb_keys_result & kb_keys_result, Lib_tty::I18n_key_chars Q, Lib_tty::I18n_key_chars QQQ ) {
    if ( auto ptr = std::get_if< Lib_tty::Key_char_singular >( & kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant); *ptr == *Q.begin() ) return true;
    if ( auto ptr = std::get_if< Lib_tty::Hot_key_chars >    ( & kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant); *ptr == QQQ )        return true;
    if ( auto ptr = std::get_if< Lib_tty::I18n_key_chars >   ( & kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant); *ptr == QQQ )        return true;
    return false;
};
/* As above but as: Lambda TODO??: We need to consider which is better for which situation.
    auto is_completed_char_input = []( Lib_tty::Kb_keys_result & kb_keys_result, string Q, Lib_tty::Hot_key_chars HHH) -> bool {
    if ( auto ptr = std::get_if< Lib_tty::Key_char_singular >( & kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant); *ptr == *Q.begin() ) return true;
    if ( auto ptr = std::get_if< Lib_tty::Hot_key_chars >    ( & kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant); *ptr == HHH )        return true;
    if ( auto ptr = std::get_if< Lib_tty::I18n_key_chars >   ( & kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant); *ptr == HHH )        return true;
    return false;
};*/
/* Speculation ONLY: struct Visitee_fns { //differentiated by parameters ONLY:
    // Members in one! parameter
        // data_type in
        // strategy_fns ?
};
struct Visitor_target_variant {
    // a datatype
    // strategy_fns ?
};
struct Visitee_return {
    // not a variant, but a struct with all types, perhaps same as/similar to target
};
*/
struct Visitee_string_kb_key_variant_name {
    auto operator() (std::monostate             const& variant_visitor_target ) { return "std::monostate"; }
    auto operator() (Lib_tty::Key_char_singular const& variant_visitor_target ) { return "Lib_tty::Key_char_singular"; }
    auto operator() (Lib_tty::I18n_key_chars    const& variant_visitor_target ) { return "Lib_tty::I18n_key_chars"; } // currently same as Lib_tty::Hot_key_chars.
    auto operator() (Lib_tty::I18n_key_table_row const&variant_visitor_target ) { return "Lib_tty::I18n_key_row"; }
    auto operator() (Lib_tty::Hot_key_table_row const& variant_visitor_target ) { return "Lib_tty::Hot_key_table_row"; }
};
/** Used for debugging in main() */
auto get_kb_key_variant_ptr( Lib_tty::Kb_key_variant const & variant_visitor_target ) -> std::pair<string, std::any const >  // $ void const * works
{   if ( auto ptr{std::get_if< std::monostate >(            & variant_visitor_target )}; ptr ) { return {"std::monostate",              ptr}; };
    if ( auto ptr{std::get_if< Lib_tty::Key_char_singular>( & variant_visitor_target )}; ptr ) { return {"Lib_tty::Key_char_singular",  ptr}; };
    if ( auto ptr{std::get_if< Lib_tty::I18n_key_chars>(    & variant_visitor_target )}; ptr ) { return {"Lib_tty::I18n_key_chars",     ptr}; };
    if ( auto ptr{std::get_if< Lib_tty::I18n_key_table_row>(& variant_visitor_target )}; ptr ) { return {"Lib_tty::I18n_key_row",       ptr}; };
    if ( auto ptr{std::get_if< Lib_tty::Hot_key_table_row>( & variant_visitor_target )}; ptr ) { return {"Lib_tty::Hot_key_table_row",  ptr}; };
    assert( false && "Logic Error: Should have found one.");
};
/** Used for debugging in main() */
auto get_kb_key_variant_value( Lib_tty::Kb_key_variant const & variant_visitor_target ) -> std::pair<string, std::any const >  // $ void const * works
{   try { auto var{std::get< std::monostate >(              variant_visitor_target )}; { return {"std::monostate",             var}; } } catch (...){ };
    try { auto var{std::get< Lib_tty::Key_char_singular>(   variant_visitor_target )}; { return {"Lib_tty::Key_char_singular", var}; } } catch (...){ };
    try { auto var{std::get< Lib_tty::I18n_key_chars>(      variant_visitor_target )}; { return {"Lib_tty::I18n_key_chars",    var}; } } catch (...){ };
    try { auto var{std::get< Lib_tty::I18n_key_table_row>(  variant_visitor_target )}; { return {"Lib_tty::I18n_key_row",      var}; } } catch (...){ };
    try { auto var{std::get< Lib_tty::Hot_key_table_row>(   variant_visitor_target )}; { return {"Lib_tty::Hot_key_table_row", var}; } } catch (...){ };
    assert( false && "Logic Error: Should have found one.");
};
class Get_row_fo {  // Function object acting like lambda
        Lib_tty::Kb_keys_result kb_keys_result_;
public: Get_row_fo( Lib_tty::Kb_keys_result const & kb_keys_result ): kb_keys_result_{kb_keys_result} {};
        Lib_tty::Kb_key_a_stati_row operator()() {
            return *kb_keys_result_.kb_key_a_stati_rows.begin();
        }
};
/** Detail helper api function to get variable from parameter. pre( nth must be valid ) */
Lib_tty::Kb_key_variant
Get_row_kb_key_vrnt( Lib_tty::Kb_keys_result kb_keys_result, size_t nth ) {
        Expects( kb_keys_result.kb_key_a_stati_rows.size() != 0 && "Error Message" );
        //static auto nth_{nth};
        //static auto itr_ { kb_keys_result.kb_key_a_stati_rows.begin() };
        auto itr2 { kb_keys_result.kb_key_a_stati_rows.begin() };
        std::advance( itr2, nth );
        Lib_tty::Kb_key_variant varnt;
        varnt = itr2->kb_key_variant;
        Ensures( kb_keys_result.kb_key_a_stati_rows.size() != 0 );
        return varnt;
};
Lib_tty::Kb_key_variant
Get_next_row_kb_key_vrnt( Lib_tty::Kb_key_a_stati_row kb_key_a_stati_row ) {
        return kb_key_a_stati_row.kb_key_variant;
}
/** Detail helper api function to get next variable from parameter. pre( next must be valid ) */
class
Get_next_row_kb_key_vrnt_fo {  // Function object
    Lib_tty::Kb_keys_result     kb_keys_result_{};

    using My_iterator = decltype(kb_keys_result_.kb_key_a_stati_rows.begin());
    My_iterator         itr { kb_keys_result_.kb_key_a_stati_rows.begin() };                          // TODO??: what does {} do here if anything beyond prior line?  Beaks what it we want?
                                                    // alternative code: decltype(kb_keys_result_.kb_key_a_stati_rows.begin()) itr1{};
  //std::iterator itr1 { kb_keys_result_.kb_key_a_stati_rows.begin() };  TODO??: How to make this compile?
                                                    //Lib_tty::Kb_key_variant varnt{};
                                                    //varnt = my_iterator->kb_key_variant;  // TODO??: compile error:must say type of variant with <>  How to fix that to be a "generic" variant
public:
                            Get_next_row_kb_key_vrnt_fo( Lib_tty::Kb_keys_result kb_keys_result ): kb_keys_result_ {kb_keys_result} {};
    Lib_tty::Kb_key_variant operator()() {
        My_iterator     save_iterator{itr};
        itr++;
        position++;
        Lib_tty::Kb_key_variant result {Get_next_row_kb_key_vrnt( *save_iterator )};
        return result;
    }
    size_t                      position{0};
};
/** Detail helper api function to get first variable from parameter. pre( next must be valid ) */
Lib_tty::Kb_key_variant detail_get_1( Lib_tty::Kb_keys_result const & keys ) {
    // *** Increased Abstraction Approach *** // TODO??: This would be nice: $ using simple = Lib_tty::Kb_keys.kb_key_a_stati_rows.begin();
    auto simple1c =                                 [&keys] () { return *keys.kb_key_a_stati_rows.begin(); }();
    Lib_tty::Kb_key_variant kb_key_variant1         { simple1c.kb_key_variant };
    Lib_tty::Kb_key_variant kb_key_variant5         { Get_row_fo{keys}().kb_key_variant };
    // *** The Explicit Approach ***
  //Lib_tty::Kb_key_variant kb_key_variant2         {keys.kb_key_a_stati_rows.begin()->kb_key_variant};
  //Lib_tty::Kb_key_variant kb_key_variant3         {keys.kb_key_a_stati_rows.at(0).kb_key_variant};
  //Lib_tty::Kb_key_variant kb_key_variant4         {keys.kb_key_a_stati_rows[0].kb_key_variant};

    char                    kb_key1                 { std::get<1>(keys.kb_key_a_stati_rows.at(0).kb_key_variant) };
  //auto                    kb_key2                 { std::get<2>(keys.kb_key_a_stati_rows.at(0).kb_key_variant) };
    int const i{1}; // TODO??: why must i be const for next line or could I do it with templates?
    char                    kb_key3                 { std::get<i>(keys.kb_key_a_stati_rows.at(0).kb_key_variant) };
    char                    kb_key4                 { std::get<char>(keys.kb_key_a_stati_rows.at(0).kb_key_variant) };
    LOGGERXR("Name of variant type:", std::visit( Visitee_string_kb_key_variant_name {}, kb_key_variant1) );
  //LOGGERXR("Int index() of variant type:", (int)kb_key_variant5);
    LOGGERXR("Int index() of variant type:",      kb_key_variant5.index());

    int16_t                 is_failed_match_chars   {keys.kb_key_a_stati_rows.begin()->is_failed_match_chars};
    Lib_tty::File_status    file_status             {keys.kb_key_a_stati_rows.begin()->file_status};
    return {keys.kb_key_a_stati_rows.begin()->kb_key_variant};
}

/** #################### This main() is used solely to test our linked shared library: lib_tty.o
 *   WARNING enable this main.cpp file in qmake ONLY if you want to run this test, but to build the libary DON'T enable this file to be linked into the *.so
 */
int main ( int argc, char* arv[] ) { string my_arv { *arv}; cout << ":~~~ argc,argv:"<<argc<<","<<my_arv<<"."<<endl; //using namespace Lib_tty; //cin.exceptions( std::istream::failbit);  // throw on fail of cin. //crash_signals_register();
    string                 STRING_H            {"h"};
    string                 STRING_I            {"i"};
    string                 STRING_Q            {"q"};
    string                 STRING_HHH          {"hhh"};
    string                 STRING_III          {"iii"};
    string                 STRING_QQQ          {"qqq"};
    Lib_tty::I18n_key_chars     Q                   {STRING_Q.begin(),  STRING_Q.end()};
    Lib_tty::I18n_key_chars     QQQ                 {STRING_QQQ.begin(),STRING_QQQ.end()};
    Lib_tty::Hot_key_chars      H                   {STRING_H.begin(),  STRING_H.end()};
    Lib_tty::Hot_key_chars      HHH                 {STRING_HHH.begin(),STRING_HHH.end()};
    Lib_tty::I18n_key_chars     I                   {STRING_I.begin(),  STRING_I.end()};
    Lib_tty::I18n_key_chars     III                 {STRING_III.begin(),STRING_III.end()};

/*Lib_tty::Kb_key_variant key_variant_kcs  { Lib_tty::Key_char_singular    {'A'} };
Lib_tty::Kb_key_variant key_variant_hkcs { Lib_tty::Hot_key_chars        {'C','C','C','C',} };
Lib_tty::Kb_key_variant key_variant_hkcs2{ Lib_tty::Hot_key_chars        {HHH} };
Lib_tty::Kb_key_variant key_variant_hkcs3{ Lib_tty::Hot_key_chars        {} };
Lib_tty::Kb_key_variant key_variant_hkr  { Lib_tty::Hot_key_table_row          { "my_hot_key_table_row",  {STRING_H.cbegin(),STRING_H.cend() },
                             Lib_tty::HotKeyFunctionCat::initial_state, Lib_tty::InteractionIntentNav::na, Lib_tty::FieldIntraNav::na } };
Lib_tty::Kb_key_variant key_variant_ikc { Lib_tty::I18n_key_chars       {III} }; //{'h','I','I','I',} };
Lib_tty::Kb_key_variant key_variant_ikr { Lib_tty::I18n_key_row         { "my_i18n_key_row", {STRING_I.cbegin(),STRING_I.cend() } } };
Lib_tty::Kb_key_variant my_monostate    { std::monostate                {} };
LOGGERXR("                           key_variant_kcs.index()", key_variant_kcs.index());
LOGGERXR("get_0_kb_key_variant_value:key_variant_kcs/string", std::get<0>( get_kb_key_variant_value( key_variant_kcs )) );
LOGGERXR("get_1_kb_key_variant_value:key_variant_kcs/value", std::any_cast<Lib_tty::KbFundamentalUnit> ( std::get<1>( get_kb_key_variant_value( key_variant_kcs ))) );
LOGGERXR("get_0_kb_key_variant_value:key_variant_hkcs", std::get<0>( get_kb_key_variant_value( key_variant_hkcs )) );
//LOGGERXR("get_1_kb_key_variant_value:key_variant_hkcs", std::get<1>( get_kb_key_variant_value( key_variant_hkcs )));
LOGGERXR("get_1_kb_key_variant_value:key_variant_hkcs", std::any_cast<Lib_tty::Hot_key_chars> ( std::get<1>( get_kb_key_variant_value( key_variant_hkcs ))) );
LOGGERXR("get_0_kb_key_variant_value:key_variant_hkr", std::get<0>( get_kb_key_variant_value( key_variant_hkr )) );
LOGGERXR("get_0_kb_key_variant_value:key_variant_ikc", std::get<0>( get_kb_key_variant_value( key_variant_ikc )) );
LOGGERXR("get_1_kb_key_variant_value:key_variant_ikr", std::get<0>( get_kb_key_variant_value( key_variant_ikr )) );
*/

    // *** Test raw character input, grabbing individual keyboard key presses, including multi-character sequences like F1 and Insert keys.
    std::monostate                  ms                  {};
    std::monostate *                ms_p                {};
    Lib_tty::Key_char_singular      hks                 {};
    Lib_tty::Key_char_singular *    hks_p               {};
    Lib_tty::Hot_key_chars          hkcs                {};
    Lib_tty::Hot_key_chars *        hkcs_p              {};
    Lib_tty::I18n_key_chars         i18ns               {};
    Lib_tty::I18n_key_chars *       i18ns_p             {};

    Lib_tty::Hot_key_table_row      hk_table_row        {};
    Lib_tty::Hot_key_table_row *    hk_table_row_p      {};
    Lib_tty::I18n_key_table_row     i18n_table_row      {};
    Lib_tty::I18n_key_table_row *   i18n_table_row_p    {};

    Lib_tty::HotKeyFunctionCat      nav                 {};
    Lib_tty::File_status            fs                  {};
    string                          user_ack            {};
    Lib_tty::Kb_keys_result         kb_keys_result      {};
    do { cout << ">ENTER a single keyboard key press now! (q or F4 for next test):"; cout.flush();
        Lib_tty::Kb_keys_result kb_keys_result      { Lib_tty::get_kb_keystrokes_raw( 1, false, true, true) };

        Lib_tty::Kb_key_variant kb_key_variant2     { kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant}; // Ugly way to get the first key value
        Lib_tty::Kb_key_variant kb_key_variant1     { detail_get_1(kb_keys_result) };
        Lib_tty::Kb_key_variant kb_key_variant3     { Get_row_fo { kb_keys_result } ().kb_key_variant };

        for ( Lib_tty::Kb_key_a_stati_row const & ks : kb_keys_result.kb_key_a_stati_rows ) { // Print the typename of all elements of all keys.
            auto a = std::visit( Visitee_string_kb_key_variant_name {}, ks.kb_key_variant);    // Visiting, where the visitee fn will recieve the variant visitor var.  OR visit the visitee with the variant.
            LOGGERXR("Key type:", a );
        };

        nav = kb_keys_result.hot_key_nav_final;
        LOGGERXR("navigation enum:", (int)nav);
        fs  = kb_keys_result.file_status_final;
        LOGGERXR("file_status enum:",(int)fs);

        ms_p =              std::get_if<std::monostate>(                &kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant );
        hks_p =             std::get_if<Lib_tty::Key_char_singular>(    &kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant );
        hkcs_p =            std::get_if<Lib_tty::Hot_key_chars>(        &kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant );
        i18ns_p =           std::get_if<Lib_tty::I18n_key_chars>(       &kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant );
        hk_table_row_p =    std::get_if<Lib_tty::Hot_key_table_row>(    &kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant );
        i18n_table_row_p =  std::get_if<Lib_tty::I18n_key_table_row>(   &kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant );
        if (ms_p != nullptr )           cout <<  "nullptr";
        if (hks_p != nullptr )          cout <<  *hks_p;
        if (hkcs_p != nullptr )         cout <<  *hkcs_p;
        if (i18ns_p != nullptr )        cout <<  *i18ns_p;
        if (hk_table_row_p != nullptr ) cout <<  hk_table_row_p->to_string();
        if (i18n_table_row_p != nullptr)cout <<  i18n_table_row_p->to_string();
        if (ms_p != nullptr )           ms =                *ms_p;
        if (hks_p != nullptr )          hks =               *hks_p;
        if (hkcs_p != nullptr )         hkcs =              *hkcs_p;
        if (i18ns_p != nullptr )        i18ns =             *i18ns_p;
        if (hk_table_row_p != nullptr ) hk_table_row =      *hk_table_row_p;
        if (i18n_table_row_p != nullptr)i18n_table_row =    *i18n_table_row_p;

      /*ms =                std::get<std::monostate>(                 kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant );
        hks =               std::get<Lib_tty::Key_char_singular>(     kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant );
        hkcs =              std::get<Lib_tty::Hot_key_chars>(         kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant );
        i18ns =             std::get<Lib_tty::I18n_key_chars>(        kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant );
        hk_table_row =      std::get<Lib_tty::Hot_key_table_row>(     kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant );
        i18n_table_row =    std::get<Lib_tty::I18n_key_table_row>(    kb_keys_result.kb_key_a_stati_rows.begin()->kb_key_variant ); */

        LOGGER_R("We got this in 3 variables below:>" );
        cout<<":MAIN():i18ns,length:{"<< i18ns<<","<<i18ns.size()<<"}, hot_key:{"<< hk_table_row.my_name << "}, file_status:{"<< (int)fs <<"}."<<endl;
        LOGGER_R(">Press RETURN to continue (q to exit(0)):>");
        getline( cin, user_ack ); cin.clear(); cout <<":Got this from continue:"<<user_ack<<endl; if ( user_ack == "q") exit(EXIT_SUCCESS);
    } while ( nav != Lib_tty::HotKeyFunctionCat::nav_field_completion &&
              nav != Lib_tty::HotKeyFunctionCat::navigation_esc       &&
              hk_table_row.my_name != "f4"                       &&        // This line is redundant with above 2 lines, but shown for example completeness.
              not is_completed_char_input(kb_keys_result, Q, QQQ )
              //i18ns != Q                                            &&
            );

    /* do { cout << "ENTER a sequence of 3 key strokes, including possibly some function_keys INTERSPERSED. (qqq or ??F4 for next test):"; cout.flush();
        Lib_tty::Kb_keys kvp { Lib_tty::get_kb_keystrokes_raw( 3, false, true, true)};
        //i18ns = kvp.key_chars_i18n;
        //hk  = kvp.hot_key;
        //fs  = kvp.file_status;
        LOGGER_R("" ); LOGGER_R("We got this in 3 variables below:" );
        //cout<<"MAIN():kb_regular_value,length:{"<< i18ns<<","<<i18ns.size()<<"}, hot_key:"<< kvp.hot_key.my_name << ", file_status:"<< (int) fs <<"."<<endl;
        //cout << "Press RETURN to continue (q to exit(0)):"; getline( cin, user_ack); cin.clear(); cout <<"got this from continue:"<<user_ack<<endl; if ( user_ack == "q") exit(0);
    //} while ( i18ns != QQQ && hk.my_name != "f4");

    do { cout << "ENTER a sequence of 3 key strokes, including possibly some function_keys, ENDING with a field_completion key stroke. (qqq or ??F4 for next test):"; cout.flush();
        Lib_tty::Kb_keys kvp { Lib_tty::get_kb_keystrokes_raw( 3, true, true, true)};
        //i18ns = kvp.key_chars_i18n;
        //hk  = kvp.hot_key;
        //fs  = kvp.file_status;
        LOGGER_R("~~~" ); LOGGER_R("We got this in 3 variables below:" );
        //cout<<"MAIN():kb_regular_value,length:{"<< i18ns<<","<<i18ns.size()<<"}, hot_key:"<< kvp.hot_key.my_name << ", file_status:"<< (int) fs <<"."<<endl;
        //cout << "Press RETURN to continue (q to exit(0)):"; getline( cin, user_ack); cin.clear(); cout <<"got this from continue:"<<user_ack<<endl; if ( user_ack == "q") exit(0);
    //} while ( i18ns != QQQ && hk.my_name != "f4"); */

    // TODO: Test other use cases of get_kb_keystrokes_raw().
    // TODO: Test other use cases of the library.
    cout << ":All tests have been run.\n";
    cout << "###" << endl;
    return EXIT_SUCCESS;
}
