
#pragma once

#include <tuple>
#include <pqxx/pqxx>

namespace db {

    typedef std::tuple<int, int, int64_t, std::string> AlarmFixed; // id, user, seconds, message

    class AlarmFixedManager {
    public:
        AlarmFixedManager(pqxx::connection&);

        void create();
        void remove();

        std::vector<AlarmFixed> all();
        AlarmFixed insert(int user_id, int64_t timestamp, const std::string& message);
        void update(const AlarmFixed& user);
        void remove(AlarmFixed& alarmFixed);
        AlarmFixed get(int id);

    protected:
        pqxx::connection& _connection;
    };

}