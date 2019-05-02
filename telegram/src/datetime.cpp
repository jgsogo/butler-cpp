
#include "telegram/datetime.h"

#include <ctime>
#include "spdlog/spdlog.h"

namespace telegram {
    namespace commands {

        std::string get_datetime() {
            time_t now;
            time(&now);
            char buf[sizeof "2011-10-08T07:07:09Z"];
            strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
            return std::string(buf);
        }

        std::string get_date() {
            time_t now;
            time(&now);
            char buf[sizeof "2011-10-08"];
            strftime(buf, sizeof buf, "%Y-%m-%d", gmtime(&now));
            return std::string(buf);
        }

        std::string get_time() {
            time_t now;
            time(&now);
            char buf[sizeof "07:07:09Z"];
            strftime(buf, sizeof buf, "%H:%M:%SZ", gmtime(&now));
            return std::string(buf);
        }

        void datetime(Bot &bot) {
            spdlog::info("Register commands: datetime, date, time");
            bot.register_command("datetime", [&bot](TgBot::Message::Ptr message) {
                bot.send_message(message->chat->id, get_datetime());
            }, "Get datetime");
            bot.register_command("date", [&bot](TgBot::Message::Ptr message) {
                bot.send_message(message->chat->id, get_date());
            }, "Get date");
            bot.register_command("time", [&bot](TgBot::Message::Ptr message) {
                bot.send_message(message->chat->id, get_time());
            }, "Get time");
        }
    }
}