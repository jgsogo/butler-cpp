
#pragma once

#include <tuple>
#include <pqxx/pqxx>

namespace db {

    typedef std::tuple<int, std::string, int> User;

    class UserManager {
    public:
        UserManager(pqxx::connection&);

        void create();
        void remove();

        std::vector<User> all();
        User insert(const std::string& name, int chat_id);
        void update(const User& user);
        void remove(User& user);
        User get(int id);

    protected:
        pqxx::connection& _connection;
    };

}