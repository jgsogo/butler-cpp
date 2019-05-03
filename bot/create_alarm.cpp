
#include "create_alarm.h"

#include "spdlog/spdlog.h"

namespace bot {
    namespace {
        std::map<int, CreateAlarm> _users_creating;
        const std::string back_to_alarms = "<< Back to alarms";
        const std::string back_to_main = "<< Main menu";
    }

    CreateAlarm::CreateAlarm(telegram::Bot& bot) : _bot(bot) {};

    void CreateAlarm::create_alarm_fixed(int64_t user_id, std::string date, std::string hour, std::string message) {
        spdlog::info("CreateAlarm::create_alarm_fixed(user='{}', date='{}', hour='{}', message='{}')", user_id, date, hour, message);
    }

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
            create_fixed_alarm.init(message);

        }, "Create a fixed alarm");
    }

    void CreateAlarm::init(TgBot::Message::Ptr message) {
        std::vector<std::vector<std::string>> options = {{"Create fixed", "Create recurrent"}, {"List alarms", back_to_main}};
        _bot.send_message(message->chat->id, "Choose option from the list below:", options);
        _dispatch = [this](TgBot::Message::Ptr message) {
            const std::string& input = message->text;
            if (input == "Create fixed") {
                this->create_fixed(message);
            }
            else if (input == "Create recurrent") {
                _bot.send_message(message->chat->id, "Not implemented yet --");
                // TODO: create recurrent
            }
            else if (input == "List alarms") {
                _bot.send_message(message->chat->id, "These are all your alarms:");
                // TODO: allow select + delete
            }
            else {
                _bot.send_message(message->chat->id, "I cannot understand you, use the menus.");
                this->init(message);
            }
        };
    }

    void CreateAlarm::create_fixed(TgBot::Message::Ptr message) {
        _bot.send_message(message->chat->id, "Ok. Let's create a fixed alarm");
        _bot.send_message(message->chat->id, "Enter a valid date (format 2019-08-12):");
        _dispatch = [this](TgBot::Message::Ptr message) {
            const std::string& input = message->text;
            std::string date = message->text;
            auto valid = true;  // TODO: validate date
            if (valid) {
                _bot.send_message(message->chat->id, "Enter a valid hour (format 18:50):");
                _dispatch = [this, date](TgBot::Message::Ptr message) {
                    const std::string& input = message->text;
                    std::string hour = message->text;
                    auto valid = true;  // TODO: validate hour
                    if (valid) {
                        std::vector<std::vector<std::string>> options = {{"Just create the alarm"}};
                        _bot.send_message(message->chat->id, "Write a message for this alarm (optional)", options);
                        _dispatch = [this, date, hour](TgBot::Message::Ptr message) {
                            std::string alarm_message = "";
                            if (message->text != "Just create the alarm") {
                                alarm_message = message->text;
                            }

                            const std::string& input = message->text;
                            this->create_alarm_fixed(message->chat->id, date, hour, alarm_message);

                            std::vector<std::vector<std::string>> options = {{back_to_alarms, back_to_main}};
                            _bot.send_message(message->chat->id, "Success! Fixed alarm created. /help", options);
                            _dispatch = [this](TgBot::Message::Ptr message) {
                                const std::string& input = message->text;
                                _bot.send_message(message->chat->id, "I cannot understand you, use the menus.");
                                this->init(message);
                            };

                        };
                    }
                    else { // Invalid hour
                        _bot.send_message(message->chat->id, "Invalid hour format, enter a valid hour (format 18:50):");
                    }
                };
            }
            else { // Invalid date
                _bot.send_message(message->chat->id, "Invalid date format, enter a valid date (format 2019-08-12):");
            }
        };
    }

    void CreateAlarm::create_recurrent(TgBot::Message::Ptr message) {

    }


    void CreateAlarm::on_any_message(TgBot::Message::Ptr message) {
        spdlog::info("CreateAlarm::on_any_message(chat_id='{}', message='{}')", message->chat->id, message->text);
        const std::string& input = message->text;
        if (input == back_to_alarms) {
            this->init(message);
        }
        else if (input == back_to_main) {
            this->end(message);
        }
        else {
            _dispatch(message);
        }
    }

    void CreateAlarm::end(TgBot::Message::Ptr message) {
        _users_creating.erase(_users_creating.find(message->chat->id));
        _bot.hold_back(message->chat->id);
    }
}