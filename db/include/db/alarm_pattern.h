
#pragma once

#include <tuple>
#include <pqxx/pqxx>

namespace db {

    typedef std::tuple<int, int, std::string, int, std::string> AlarmPattern;  // id, user, pattern, seconds, message

    class AlarmPatternManager {
    public:
        AlarmPatternManager(pqxx::connection&);

        void create();
        void remove();

        std::vector<AlarmPattern> all();
        AlarmPattern insert(int user_id, const std::string& pattern, int timestamp, const std::string& message);
        void update(const AlarmPattern& user);
        void remove(AlarmPattern& user);
        AlarmPattern get(int id);

    protected:
        pqxx::connection& _connection;
    };


}