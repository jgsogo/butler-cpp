
#pragma once

#include "db/table_user.h"

namespace db {
    class Database {
    public:
        static Database& instance();
        postgresql::table::UserManager& users();

    protected:
        Database();
        ~Database();

        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };
}