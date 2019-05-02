
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

    db::UserManager& Database::users() {
        static db::UserManager manager(pImpl->_connection);
        return manager;
    }

    db::AlarmFixedManager& Database::alarms_fixed() {
        static db::AlarmFixedManager manager(pImpl->_connection);
        return manager;
    }

    db::AlarmPatternManager& Database::alarms_pattern() {
        static db::AlarmPatternManager manager(pImpl->_connection);
        return manager;
    }
}
