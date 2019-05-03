
#pragma once

#include <string>


namespace utils {

    bool validate_date(const std::string&);
    bool validate_hour(const std::string&);
    time_t timestamp(const std::string& date, const std::string& hour);
    std::string humanize(time_t);
}
