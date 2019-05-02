
#pragma once

#include "db/user.h"
#include "db/alarm_fixed.h"
#include "db/alarm_pattern.h"

namespace db {
    class Database {
    public:
        static Database& instance();
        db::UserManager& users();
        db::AlarmFixedManager& alarms_fixed();
        db::AlarmPatternManager& alarms_pattern();

    protected:
        Database();
        ~Database();

        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };
}