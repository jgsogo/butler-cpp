
#include "db/user.h"

#include "spdlog/spdlog.h"
#include <fmt/format.h>


namespace db {

    namespace {
        const std::string table_name = "users";
        const std::vector<std::string> fields = {"id", "name", "chat_id"};

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

    UserManager::UserManager(pqxx::connection& connection) : _connection(connection) {}

    void UserManager::create() {
        auto result = run_query(_connection, fmt::format("SELECT to_regclass('public.{}');", table_name));
        if (result[0][0].is_null()) {
            run_query(_connection, fmt::format("CREATE TABLE {} ("
                                              "    {} SERIAL,"
                                              "    {} varchar(120),"
                                              "    {} integer)", table_name, fields[0], fields[1], fields[2]));
        }
    }

    void UserManager::remove() {
        run_query(_connection, fmt::format("DROP TABLE IF EXISTS {}", table_name));
    }

    std::vector<User> UserManager::all() {
        auto result = run_query(_connection, fmt::format("SELECT {}, {}, {} FROM {}", fields[0], fields[1], fields[2], table_name));
        std::vector<User> ret;
        for (auto item: result) {
            ret.emplace_back(std::make_tuple(item[0].as<int>(), item[1].as<std::string>(), item[2].as<int>()));
        }
        return ret;
    }

    User UserManager::insert(const std::string& name, int chat_id) {
        spdlog::debug("UserManager::insert(name='{}', chat_id='{}')", name, chat_id);
        auto r = run_query(_connection, fmt::format("INSERT INTO {} ({}, {}) values ('{}', '{}') RETURNING id",
                table_name, fields[1], fields[2], name, chat_id));
        return User{r[0][0].as<int>(), name, chat_id};
    }

    void UserManager::update(const User& user) {
        spdlog::debug("UserManager::update(id='{}', name='{}', chat_id='{}')", std::get<0>(user), std::get<1>(user), std::get<2>(user));
        auto r = run_query(_connection, fmt::format("UPDATE {} SET {}='{}', {}='{}' WHERE {}={}", table_name,
                fields[1], std::get<1>(user), fields[2], std::get<2>(user), fields[0], std::get<0>(user)));
    }

    void UserManager::remove(User& user) {
        spdlog::debug("UserManager::remove(id='{}', name='{}', chat_id='{}')", std::get<0>(user), std::get<1>(user), std::get<2>(user));
        run_query(_connection, fmt::format("DELETE FROM {} WHERE {}={}", table_name, fields[0], std::get<0>(user)));
    }

    User UserManager::get(int id) {
        spdlog::debug("UserManager::remove(id='{}')", id);
        auto r = run_query(_connection, fmt::format("SELECT {}, {}, {} FROM {} WHERE {}={}",
                fields[0], fields[1], fields[2], table_name, fields[0], id));
        return User{r[0][0].as<int>(), r[0][1].as<std::string>(), r[0][2].as<int>()};
    }

}
