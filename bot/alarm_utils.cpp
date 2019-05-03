
#include "alarm_utils.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


namespace utils {
    bool validate_date(const std::string &date) {
        std::istringstream time_stream{date};  // Construct a stream from string
        std::tm time;  // Time structure
        time_stream >> std::get_time(&time, "%Y-%m-%d");  // Fill time struct
        return !time_stream.fail();
    }

    bool validate_hour(const std::string& hour) {
        std::istringstream time_stream{hour};  // Construct a stream from string
        std::tm time;  // Time structure
        time_stream >> std::get_time(&time, "%H:%M");  // Fill time struct
        return !time_stream.fail();
    }

    time_t timestamp(const std::string &date, const std::string &hour) {
        // Parse date
        std::istringstream date_stream{date};
        std::tm time_date{};
        date_stream >> std::get_time(&time_date, "%Y-%m-%d");
        assert(!date_stream.fail());

        // Parse hour
        std::istringstream hour_stream{hour};
        std::tm time_hour{};
        hour_stream >> std::get_time(&time_hour, "%H:%M");
        assert(!hour_stream.fail());

        time_date.tm_hour = time_hour.tm_hour;
        time_date.tm_min = time_hour.tm_min;
        return timegm(&time_date);
    }

    std::string humanize(time_t tm) {
        char buf[sizeof "2011-10-08 07:07:09"];
        strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", gmtime(&tm));
        return std::string(buf);
    }
}