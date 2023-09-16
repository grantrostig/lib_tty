/* copyright grant rostig grantrostig.com
 */
#include "lib_tty_internal.h"
#include <deque>
#include <string>

//template<typename Container>            // utility f() to print vectors
//    requires Insertable<Container>
//std::ostream&
//operator<<( std::ostream & out, Container const & c) {
//    if ( not c.empty()) {
//        out << "[";   // todo??: add std::setw(3)
//        out.width(9);
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

