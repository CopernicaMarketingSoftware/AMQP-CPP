/**
 *  numeric_cast.h
 *
 * safety cast from int64 and int32
 */

#pragma once

#include <limits>
#include <stdexcept>
#include <sstream>

namespace AMQP {

/**
 *  returns the result of converting a value of type Source to a value of type Target. If out-of-range is detected, an exception is thrown
 *
 *  @param  source          value to be cast
 */
template<typename Target, typename Source>
Target numeric_cast(Source source) {
    if (source > std::numeric_limits<Target>::max()
        || source < std::numeric_limits<Target>::min()
        ) {
        std::stringstream info;
        info << sizeof(Source) << "bytes value (" << source << ") does not fit in " << sizeof(Target) << " bytes" << sizeof(Source);
        throw std::overflow_error(info.str());
    }
    return (Target)source;
}

/**
 *  Executes numeric cast from source to target. Deduces Target type from target.
 *
 *  @param  source          value to be cast
 */
template<typename Target, typename Source>
void numeric_cast(Target &target, Source source) {
    target = numeric_cast<Target>(source);
}

/**
 *  End of namespace
 */
}

