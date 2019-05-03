
#include "create_alarm.h"

#include "spdlog/spdlog.h"
#include "alarm_utils.h"
#include "db/database.h"

namespace bot {
    namespace {
        std::map<telegram::Bot::chat_id_t, CreateAlarm> _users_creating;
        const std::string command = "alarms";
        const std::string back_to_alarms = "<< Back to alarms";
        const std::string back_to_main = "<< Main menu";
    }

    CreateAlarm::CreateAlarm(telegram::Bot& bot) : _bot(bot) {};

    void CreateAlarm::db_create_alarm_fixed(telegram::Bot::chat_id_t user_id, std::string date, std::string hour,
                                            std::string message) {
        spdlog::info("CreateAlarm::create_alarm_fixed(user='{}', date='{}', hour='{}', message='{}')", user_id, date, hour, message);
        auto timestamp = utils::timestamp(date, hour);
        db::Database::instance().alarms_fixed().insert(user_id, timestamp, message);
    }

    std::vector<std::string> CreateAlarm::db_list_alarms(telegram::Bot::chat_id_t user_id, int count, int init) {
        auto all_alarms = db::Database::instance().alarms_fixed().all();  // TODO: Filter!
        // TODO: add pattern alarms
        std::vector<std::string> ret;
        int counter = 0;
        for (auto& it: all_alarms) {
            if (std::get<1>(it) != user_id) continue;
            if (counter >= init) {
                ret.push_back(fmt::format("{} {}", utils::humanize(std::get<2>(it)), std::get<3>(it)));
            }
            if (ret.size() == count) {
                break;
            }
            ++counter;
        }
        return ret;
    }

    void CreateAlarm::register_command(telegram::Bot& bot) {
        spdlog::info("Register command: /{}", command);
        bot.register_command(command, [&bot](TgBot::Message::Ptr message) {
            auto [it, inserted] = _users_creating.insert(std::make_pair(message->chat->id, CreateAlarm{bot}));
            if (!inserted) {
                spdlog::error("CreateAlarm for 'message->chat->id' was not previously removed!");
            }
            CreateAlarm& create_fixed_alarm = it->second;

            bot.delegate(message->chat->id, [&create_fixed_alarm](TgBot::Message::Ptr message) {
                             create_fixed_alarm.on_any_message(message);
                         },
                         [&create_fixed_alarm](TgBot::CallbackQuery::Ptr query) {
                             create_fixed_alarm.on_callback_query(query);
                         });
            create_fixed_alarm.init(message->chat->id);

        }, "Create a fixed alarm");
    }

    void CreateAlarm::init(telegram::Bot::chat_id_t chat_id) {
        std::vector<std::vector<std::string>> options = {{"Create fixed", "Create recurrent"}, {"List alarms", back_to_main}};
        _bot.send_message(chat_id, "Choose option from the list below:", options);
        _dispatch_query = [this](TgBot::CallbackQuery::Ptr query) {
            const std::string& input = query->data;
            if (input == "Create fixed") {
                this->create_fixed(query->from->id, query->message->chat->id);
            }
            else if (input == "Create recurrent") {
                _bot.send_message(query->message->chat->id, "Not implemented yet --");
                // TODO: create recurrent
            }
            else if (input == "List alarms") {
                this->list_alarms(query->from->id, query->message->chat->id);
            }
            else {
                _bot.send_message(query->message->chat->id, "I cannot understand you, use the menus.");
                this->init(query->message->chat->id);
            }
        };
    }

    void CreateAlarm::list_alarms(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id) {
        auto alarms = this->db_list_alarms(user_id);
        // TODO: handle start and count
        // TODO: allow select + delete
        std::stringstream ss;
        ss << "These are all your alarms:\n";
        for (auto& it: alarms) {
            ss << it << "\n";
        }
        _bot.send_message(chat_id, ss.str());
        this->end_task(chat_id, "What's next?");
    }

    void CreateAlarm::create_fixed(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id) {
        _bot.send_message(chat_id, "Ok. Let's create a fixed alarm");
        _bot.send_message(chat_id, "Enter a valid date (format 2019-08-12):");
        _dispatch = [this, user_id](TgBot::Message::Ptr message) {
            assert(user_id == message->from->id);
            std::string date = message->text;
            auto valid = utils::validate_date(date);
            if (valid) {
                _bot.send_message(message->chat->id, "Enter a valid hour (format 18:50):");
                _dispatch = [this, user_id, date](TgBot::Message::Ptr message) {
                    assert(user_id == message->from->id);
                    std::string hour = message->text;
                    auto valid = utils::validate_hour(hour);  // TODO: validate hour
                    if (valid) {
                        std::vector<std::vector<std::string>> options = {{"Just create the alarm"}};
                        _bot.send_message(message->chat->id, "Write a message for this alarm (optional)", options);

                        _dispatch = [this, user_id, date, hour](TgBot::Message::Ptr message) {
                            assert(user_id == message->from->id);
                            this->db_create_alarm_fixed(user_id, date, hour, message->text);
                            this->end_task(message->chat->id, "Fixed alarm created!");
                        };

                        _dispatch_query = [this, user_id, date, hour](TgBot::CallbackQuery::Ptr query) {
                            assert(user_id == query->from->id);
                            assert(query->data == "Just create the alarm");
                            this->db_create_alarm_fixed(user_id, date, hour, "");
                            this->end_task(query->message->chat->id, "Fixed alarm created!");
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

    void CreateAlarm::create_recurrent(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id) {

    }

    void CreateAlarm::on_callback_query(TgBot::CallbackQuery::Ptr query) {
        spdlog::info("CreateAlarm::on_callback_query(chat_id='{}', data='{}', message='{}')", query->message->chat->id, query->data, query->message->text);
        const std::string& input = query->data;
        if (input == back_to_alarms) {
            this->init(query->message->chat->id);
        }
        else if (input == back_to_main) {
            this->end(query->message->chat->id);
        }
        else {
            _dispatch_query(query);
        }
    }

    void CreateAlarm::on_any_message(TgBot::Message::Ptr message) {
        spdlog::info("CreateAlarm::on_any_message(chat_id='{}', message='{}')", message->chat->id, message->text);
        const std::string& input = message->text;
        if (input == back_to_alarms) {
            this->init(message->chat->id);
        }
        else if (input == back_to_main) {
            this->end(message->chat->id);
        }
        else {
            _dispatch(message);
        }
    }

    void CreateAlarm::end(telegram::Bot::chat_id_t chat_id) {
        spdlog::debug("CreateAlarm::end(chat_id='{}')", chat_id);
        _users_creating.erase(_users_creating.find(chat_id));
        _bot.send_message(chat_id, "Waiting for your order. /help");
        _bot.hold_back(chat_id);
    }

    void CreateAlarm::end_task(telegram::Bot::chat_id_t chat_id, const std::string& task) {
        std::vector<std::vector<std::string>> options = {{back_to_alarms, back_to_main}};
        _bot.send_message(chat_id, fmt::format("{} /help", task), options);
        _dispatch_query = [this](TgBot::CallbackQuery::Ptr query) {
            _bot.send_message(query->message->chat->id, "I cannot understand you, use the menus.");
            this->init(query->message->chat->id);
        };
    }

}