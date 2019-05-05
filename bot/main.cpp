
#include <iostream>
#include <sstream>
#include <stdio.h>

#include "spdlog/spdlog.h"

#include "telegram/bot.h"
#include "telegram/datetime.h"
#include "create_alarm.h"

#include "alarm_utils.h"
#include "db/database.h"
#include "mpd/mpd_telegram.h"

const std::string butler_bot_token_var = "BUTLER_BOT_TOKEN";

int main(int argc,char** argv) {
    const char* token = std::getenv(butler_bot_token_var.c_str());
    if (!token) {
        std::cerr << "Provide env variable '" << butler_bot_token_var <<"'\n";
        return 1;
    }

    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%H:%M:%S %z] [%^%8l%$] [%n] [thread %t] %v");

    // Database
    auto& database = db::Database::instance();
    database.users().create();
    database.alarms_fixed().create();
    database.alarms_pattern().create();

    // Bot
    telegram::Bot bot{token};
    //telegram::commands::datetime(bot);
    bot::CreateAlarm::register_command(bot);
    mpd::telegram_commands(bot);

    bot.run();
}