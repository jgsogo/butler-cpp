
#include "db/database.h"

#include <pqxx/pqxx>

namespace db {

    struct Database::Impl {
        pqxx::connection _connection;
    };

    Database::Database() : pImpl(std::make_unique<Impl>()) {}
    Database::~Database() {}

    Database& Database::instance() {
        static Database instance;
        return instance;
    }

    postgresql::table::UserManager& Database::users() {
        static postgresql::table::UserManager user_manager(pImpl->_connection);
        return user_manager;
    }
}
