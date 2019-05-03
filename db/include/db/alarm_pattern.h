
#pragma once

#include <tuple>
#include <pqxx/pqxx>

namespace db {

    enum class DatePattern : short {EVERY_DAY, LABOUR_DAYS, FREE_DAYS, ON_WEEKDAY, ON_MONTH_DAY, ON_YEAR_DAY};
    typedef std::tuple<int, int, short, time_t, time_t, time_t, std::string> AlarmPattern;  // id, user, pattern, hour, date_init, date_end, message

    class AlarmPatternManager {
    public:
        AlarmPatternManager(pqxx::connection&);

        void create();
        void remove();

        std::vector<AlarmPattern> all();
        AlarmPattern insert(int user_id, DatePattern, time_t hour, time_t date_init, time_t date_end, const std::string& message);
        void update(const AlarmPattern& user);
        void remove(AlarmPattern& user);
        AlarmPattern get(int id);

    protected:
        pqxx::connection& _connection;
    };


}