
#include "db/alarm_pattern.h"

#include "spdlog/spdlog.h"
#include <fmt/format.h>


namespace db {

    namespace {
        const std::string table_name = "alarm_pattern";
        const std::vector<std::string> fields = {"id", "user_id", "date_pattern", "hour", "date_init", "date_end", "message"};

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

    DatePattern pattern_from_str(const std::string& str) {
        int i = 0;
        for (auto& it: DatePatterStr) {
            if (it == str) return static_cast<DatePattern>(i);
            ++i;
        }
        throw std::runtime_error(fmt::format("Invalid DatePattern '{}'", str));
    }

    AlarmPatternManager::AlarmPatternManager(pqxx::connection& connection) : _connection(connection) {}

    void AlarmPatternManager::create() {
        auto result = run_query(_connection, fmt::format("SELECT to_regclass('public.{}');", table_name));
        if (result[0][0].is_null()) {
            run_query(_connection, fmt::format("CREATE TABLE {} ("
                                               "    {} SERIAL,"
                                               "    {} integer,"
                                               "    {} integer,"  // TODO: maybe short
                                               "    {} bigint,"
                                               "    {} bigint,"
                                               "    {} bigint,"
                                               "    {} varchar)", table_name, fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6]));
        }
    }

    void AlarmPatternManager::remove() {
        run_query(_connection, fmt::format("DROP TABLE IF EXISTS {}", table_name));
    }

    std::vector<AlarmPattern> AlarmPatternManager::all() {
        auto result = run_query(_connection, fmt::format("SELECT {}, {}, {}, {}, {}, {}, {} FROM {}", fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6], table_name));
        std::vector<AlarmPattern> ret;
        for (auto item: result) {
            ret.emplace_back(std::make_tuple(item[0].as<int>(), item[1].as<int>(), item[2].as<short>(), item[3].as<time_t>(), item[4].as<time_t>(), item[5].as<time_t>(), item[6].as<std::string>()));
        }
        return ret;
    }

    AlarmPattern AlarmPatternManager::insert(int user_id, DatePattern pattern, time_t hour, time_t date_init, time_t date_end, const std::string& message) {
        spdlog::debug("AlarmFixedManager::insert(user_id='{}', pattern='{}', hour='{}', date_init='{}', date_end='{}', message='{}')", user_id, static_cast<short>(pattern), hour, date_init, date_end, message);
        auto r = run_query(_connection, fmt::format("INSERT INTO {} ({}, {}, {}, {}, {}, {}) values ('{}', '{}', '{}', '{}', '{}', '{}') RETURNING id",
                                                    table_name, fields[1], fields[2], fields[3], fields[4], fields[5], fields[6], user_id, static_cast<short>(pattern), hour, date_init, date_end, message));
        return AlarmPattern{r[0][0].as<int>(), user_id, static_cast<short>(pattern), hour, date_init, date_end, message};
    }

    void AlarmPatternManager::update(const AlarmPattern& alarm) {
        spdlog::debug("AlarmFixedManager::update(id='{}', user_id='{}', pattern='{}', timestamp='{}', message='{}')", std::get<0>(alarm), std::get<1>(alarm), std::get<2>(alarm), std::get<3>(alarm), std::get<4>(alarm));
        auto r = run_query(_connection, fmt::format("UPDATE {} SET {}='{}', {}='{}', {}='{}', {}='{}', {}='{}', {}='{}' WHERE {}={}", table_name,
                                                    fields[1], std::get<1>(alarm), fields[2], std::get<2>(alarm), fields[3], std::get<3>(alarm), fields[4], std::get<4>(alarm), fields[5], std::get<5>(alarm), fields[6], std::get<6>(alarm), fields[0], std::get<0>(alarm)));
    }

    void AlarmPatternManager::remove(AlarmPattern& alarmFixed) {
        spdlog::debug("AlarmFixedManager::remove(id='{}', user_id='{}', pattern='{}', timestamp='{}', message='{}')", std::get<0>(alarmFixed), std::get<1>(alarmFixed), std::get<2>(alarmFixed), std::get<3>(alarmFixed), std::get<4>(alarmFixed));
        run_query(_connection, fmt::format("DELETE FROM {} WHERE {}={}", table_name, fields[0], std::get<0>(alarmFixed)));
    }

    AlarmPattern AlarmPatternManager::get(int id) {
        spdlog::debug("AlarmFixedManager::remove(id='{}')", id);
        auto r = run_query(_connection, fmt::format("SELECT {}, {}, {}, {}, {}, {}, {} FROM {} WHERE {}={}",
                                                    fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6], table_name, fields[0], id));
        return AlarmPattern{r[0][0].as<int>(), r[0][1].as<int>(), r[0][2].as<short>(), r[0][3].as<time_t>(), r[0][4].as<time_t>(), r[0][5].as<time_t>(), r[0][6].as<std::string>()};
    }

}
