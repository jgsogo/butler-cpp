
#pragma once

#include <tuple>
#include <pqxx/pqxx>

namespace postgresql {
    namespace table {

        typedef std::tuple<int, std::string, int> User;

        class UserManager {
        public:

            static void create(pqxx::connection&);
            static void remove(pqxx::connection&);

            static std::vector<User> all(pqxx::connection&);
            static User insert(pqxx::connection&, const std::string& name, int chat_id);
            static void update(pqxx::connection&, const User& user);
            static void remove(pqxx::connection&, User& user);
            static User get(pqxx::connection&, int id);
        };

    }
}