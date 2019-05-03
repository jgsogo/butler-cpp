
#include "create_fixed_alarm.h"

#include "spdlog/spdlog.h"

namespace bot {
    namespace {
        std::map<int, CreateAlarm> _users_creating;
    }

    CreateAlarm::CreateAlarm(telegram::Bot& bot) : _bot(bot) {};

    void CreateAlarm::register_command(telegram::Bot& bot) {
        spdlog::info("Register command: /fixed_alarm");
        bot.register_command("fixed_alarm", [&bot](TgBot::Message::Ptr message) {
            auto [it, inserted] = _users_creating.insert(std::make_pair(message->chat->id, CreateAlarm{bot}));
            if (!inserted) {
                spdlog::error("CreateAlarm for 'message->chat->id' was not previously removed!");
            }
            CreateAlarm& create_fixed_alarm = it->second;

            bot.delegate(message->chat->id, [&create_fixed_alarm](TgBot::Message::Ptr message){
                create_fixed_alarm.on_any_message(message);
            });

        }, "Create a fixed alarm");
    }

    void CreateAlarm::on_any_message(TgBot::Message::Ptr message) {
        spdlog::info("CreateAlarm::on_any_message(chat_id='{}', message='{}')", message->chat->id, message->text);

        if (message->text == "end") {
            _users_creating.erase(_users_creating.find(message->chat->id));
            _bot.hold_back(message->chat->id);
        }
    }
}