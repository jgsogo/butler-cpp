
#include "db/alarm_pattern.h"

#include "spdlog/spdlog.h"
#include <fmt/format.h>


namespace db {

    namespace {
        const std::string table_name = "alarm_pattern";
        const std::vector<std::string> fields = {"id", "user_id", "pattern", "timestamp", "message"};

        pqxx::result run_query(pqxx::connection& connection, const std::string& query) {
            pqxx::work work(connection);
            try {
                auto r = work.exec(query);
                work.commit();
                return r;
            } catch (const std::exception &e) {
                work.abort();
                throw;
            }
        }
    }

    AlarmPatternManager::AlarmPatternManager(pqxx::connection& connection) : _connection(connection) {}

    void AlarmPatternManager::create() {
        auto result = run_query(_connection, fmt::format("SELECT to_regclass('public.{}');", table_name));
        if (result[0][0].is_null()) {
            run_query(_connection, fmt::format("CREATE TABLE {} ("
                                               "    {} SERIAL,"
                                               "    {} integer,"
                                               "    {} varchar(40),"
                                               "    {} int,"
                                               "    {} varchar)", table_name, fields[0], fields[1], fields[2], fields[3], fields[4]));
        }
    }

    void AlarmPatternManager::remove() {
        run_query(_connection, fmt::format("DROP TABLE IF EXISTS {}", table_name));
    }

    std::vector<AlarmPattern> AlarmPatternManager::all() {
        auto result = run_query(_connection, fmt::format("SELECT {}, {}, {}, {}, {} FROM {}", fields[0], fields[1], fields[2], fields[3], fields[4], table_name));
        std::vector<AlarmPattern> ret;
        for (auto item: result) {
            ret.emplace_back(std::make_tuple(item[0].as<int>(), item[1].as<int>(), item[2].as<std::string>(), item[3].as<int>(), item[4].as<std::string>()));
        }
        return ret;
    }

    AlarmPattern AlarmPatternManager::insert(int user_id, const std::string& pattern, int timestamp, const std::string& message) {
        spdlog::debug("AlarmFixedManager::insert(user_id='{}', pattern='{}', timestamp='{}', message='{}')", user_id, pattern, timestamp, message);
        auto r = run_query(_connection, fmt::format("INSERT INTO {} ({}, {}, {}, {}) values ('{}', '{}', '{}', '{}') RETURNING id",
                                                    table_name, fields[1], fields[2], fields[3], fields[4], user_id, timestamp, message));
        return AlarmPattern{r[0][0].as<int>(), user_id, pattern, timestamp, message};
    }

    void AlarmPatternManager::update(const AlarmPattern& alarm) {
        spdlog::debug("AlarmFixedManager::update(id='{}', user_id='{}', pattern='{}', timestamp='{}', message='{}')", std::get<0>(alarm), std::get<1>(alarm), std::get<2>(alarm), std::get<3>(alarm), std::get<4>(alarm));
        auto r = run_query(_connection, fmt::format("UPDATE {} SET {}='{}', {}='{}', {}='{}', {}='{}' WHERE {}={}", table_name,
                                                    fields[1], std::get<1>(alarm), fields[2], std::get<2>(alarm), fields[3], std::get<3>(alarm), fields[4], std::get<4>(alarm), fields[0], std::get<0>(alarm)));
    }

    void AlarmPatternManager::remove(AlarmPattern& alarmFixed) {
        spdlog::debug("AlarmFixedManager::remove(id='{}', user_id='{}', pattern='{}', timestamp='{}', message='{}')", std::get<0>(alarmFixed), std::get<1>(alarmFixed), std::get<2>(alarmFixed), std::get<3>(alarmFixed), std::get<4>(alarmFixed));
        run_query(_connection, fmt::format("DELETE FROM {} WHERE {}={}", table_name, fields[0], std::get<0>(alarmFixed)));
    }

    AlarmPattern AlarmPatternManager::get(int id) {
        spdlog::debug("AlarmFixedManager::remove(id='{}')", id);
        auto r = run_query(_connection, fmt::format("SELECT {}, {}, {}, {}, {} FROM {} WHERE {}={}",
                                                    fields[0], fields[1], fields[2], fields[3], fields[4], table_name, fields[0], id));
        return AlarmPattern{r[0][0].as<int>(), r[0][1].as<int>(), r[0][2].as<std::string >(), r[0][3].as<int>(), r[0][4].as<std::string>()};
    }

}
