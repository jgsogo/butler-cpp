#include <iostream>
#include <pqxx/pqxx>

#include "spdlog/spdlog.h"
#include "fmt/format.h"
#include "postgresql/table_user.h"


const std::vector<std::string> postgresql_url_vars = {"PGDATABASE", "PGHOST", "PGPORT", "PGUSER", "PGPASSWORD"};

int main()
{
    for (auto& env_var: postgresql_url_vars) {
        const char* value = std::getenv(env_var.c_str());
        if (!value) {
            std::cerr << "Provide env variable '" << env_var <<"'\n";
            return 1;
        }
    }

    try
    {
        pqxx::connection connection;
        std::cout << "Connected to " << connection.dbname() << std::endl;

        postgresql::table::UserManager::remove(connection);
        postgresql::table::UserManager::create(connection);
        auto u1 = postgresql::table::UserManager::insert(connection, "javi", 23);
        auto u2 = postgresql::table::UserManager::insert(connection, "laura", 42);

        auto list_all = [&](){
            std::cout << "List all\n";
            auto all = postgresql::table::UserManager::all(connection);
            for (auto& it: all) {
                std::cout << fmt::format(" id='{}', name='{}', chat_id='{}'\n", std::get<0>(it), std::get<1>(it), std::get<2>(it));
            }
        };
        list_all();

        std::get<2>(u1) = 54;
        postgresql::table::UserManager::update(connection, u1);
        list_all();

        postgresql::table::UserManager::remove(connection, u1);
        list_all();

        auto uu = postgresql::table::UserManager::get(connection, std::get<0>(u2));
        std::cout << fmt::format("GET: id='{}', name='{}', chat_id='{}'\n", std::get<0>(uu), std::get<1>(uu), std::get<2>(uu));
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}