#pragma once

#include <string>
#include <sstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>


template<typename Iterator>
std::string join(Iterator first, const Iterator last,
        const std::string& separator)
{
    std::string str;
    for (; first != last; ++first)
    {
        str.append(*first);
        if (first != (last - 1))
        {
            str.append(separator);
        }

    }
    return str;
}

std::string uuid()
{
    boost::uuids::random_generator generator;
    boost::uuids::uuid uuid(generator());

    std::stringstream sstr;
    sstr<<boost::uuids::to_string(uuid);
    return sstr.str();
}
