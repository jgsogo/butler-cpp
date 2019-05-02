
#include "db/alarm_fixed.h"

#include "spdlog/spdlog.h"
#include <fmt/format.h>


namespace db {

    namespace {
        const std::string table_name = "alarm_fixed";
        const std::vector<std::string> fields = {"id", "user_id", "seconds", "message"};

        pqxx::result run_query(pqxx::connection &connection, const std::string &query) {
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

    AlarmFixedManager::AlarmFixedManager(pqxx::connection& connection) : _connection(connection) {}

    void AlarmFixedManager::create() {
        auto result = run_query(_connection, fmt::format("SELECT to_regclass('public.{}');", table_name));
        if (result[0][0].is_null()) {
            run_query(_connection, fmt::format("CREATE TABLE {} ("
                                               "    {} SERIAL,"
                                               "    {} integer,"
                                               "    {} bigint,"
                                               "    {} varchar)", table_name, fields[0], fields[1], fields[2], fields[3]));
        }
    }

    void AlarmFixedManager::remove() {
        run_query(_connection, fmt::format("DROP TABLE IF EXISTS {}", table_name));
    }

    std::vector<AlarmFixed> AlarmFixedManager::all() {
        auto result = run_query(_connection, fmt::format("SELECT {}, {}, {}, {} FROM {}", fields[0], fields[1], fields[2], fields[3], table_name));
        std::vector<AlarmFixed> ret;
        for (auto item: result) {
            ret.emplace_back(std::make_tuple(item[0].as<int>(), item[1].as<int>(), item[2].as<int64_t>(), item[3].as<std::string>()));
        }
        return ret;
    }

    AlarmFixed AlarmFixedManager::insert(int user_id, int64_t timestamp, const std::string& message) {
        spdlog::debug("AlarmFixedManager::insert(user_id='{}', timestamp='{}', message='{}')", user_id, timestamp, message);
        auto r = run_query(_connection, fmt::format("INSERT INTO {} ({}, {}, {}) values ('{}', '{}', '{}') RETURNING id",
                                                    table_name, fields[1], fields[2], fields[3], user_id, timestamp, message));
        return AlarmFixed{r[0][0].as<int>(), user_id, timestamp, message};
    }

    void AlarmFixedManager::update(const AlarmFixed& alarm) {
        spdlog::debug("AlarmFixedManager::update(id='{}', user_id='{}', timestamp='{}', message='{}')", std::get<0>(alarm), std::get<1>(alarm), std::get<2>(alarm), std::get<3>(alarm));
        auto r = run_query(_connection, fmt::format("UPDATE {} SET {}='{}', {}='{}', {}='{}' WHERE {}={}", table_name,
                                                    fields[1], std::get<1>(alarm), fields[2], std::get<2>(alarm), fields[3], std::get<3>(alarm), fields[0], std::get<0>(alarm)));
    }

    void AlarmFixedManager::remove(AlarmFixed& alarmFixed) {
        spdlog::debug("AlarmFixedManager::remove(id='{}', user_id='{}', timestamp='{}', message='{}')", std::get<0>(alarmFixed), std::get<1>(alarmFixed), std::get<2>(alarmFixed), std::get<3>(alarmFixed));
        run_query(_connection, fmt::format("DELETE FROM {} WHERE {}={}", table_name, fields[0], std::get<0>(alarmFixed)));
    }

    AlarmFixed AlarmFixedManager::get(int id) {
        spdlog::debug("AlarmFixedManager::remove(id='{}')", id);
        auto r = run_query(_connection, fmt::format("SELECT {}, {}, {}, {} FROM {} WHERE {}={}",
                                                    fields[0], fields[1], fields[2], fields[3], table_name, fields[0], id));
        return AlarmFixed{r[0][0].as<int>(), r[0][1].as<int>(), r[0][2].as<__int64_t >(), r[0][3].as<std::string>()};
    }

}
