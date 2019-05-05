
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

    void CreateAlarm::db_create_alarm_recurrent(telegram::Bot::chat_id_t user_id, db::DatePattern date_pattern, std::string hour,
                                   std::string date_end, std::string date_init, std::string message) {
        spdlog::info("CreateAlarm::db_create_alarm_recurrent(user='{}', date_pattern='{}', hour='{}', date_end='{}', "
                     "date_init='{}', message='{}')", user_id, db::DatePatterStr.at(static_cast<short>(date_pattern)),
                     hour, date_end, date_init, message);
        auto timestamp_end = utils::timestamp(date_end.empty() ? "1970-01-01" : date_end, "00:00");
        auto timestamp_init = utils::timestamp(date_init.empty() ? "1970-01-01" : date_end, "00:00");
        auto timestamp_hour = utils::timestamp("1970-01-01", hour);
        db::Database::instance().alarms_pattern().insert(user_id, date_pattern, timestamp_hour, timestamp_init, timestamp_end, message);
    }

    std::vector<std::string> CreateAlarm::db_list_alarms(telegram::Bot::chat_id_t user_id, int count, int init) {
        auto fixed_alarms = db::Database::instance().alarms_fixed().all();  // TODO: Filter by user
        auto recurrent_alarms = db::Database::instance().alarms_pattern().all(); // TODO: Filter by user

        std::vector<std::string> ret;
        int counter = 0;
        for (auto& it: fixed_alarms) {
            if (std::get<1>(it) != user_id) continue;
            if (ret.size() == count) {
                break;
            }
            if (counter >= init) {
                ret.push_back(fmt::format("{} {}", utils::humanize(std::get<2>(it)), std::get<3>(it)));
            }
            ++counter;
        }
        for (auto& it: recurrent_alarms) {
            if (std::get<1>(it) != user_id) continue;
            if (ret.size() == count) {
                break;
            }
            if (counter >= init) {
                ret.push_back(fmt::format("{} {} {}", utils::humanize(std::get<3>(it)), db::DatePatterStr.at(std::get<2>(it)), std::get<6>(it)));
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

        }, "Create alarms, fixed and recurrent");
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
                this->create_recurrent(query->from->id, query->message->chat->id);
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
        spdlog::debug("CreateAlarm::create_fixed(user_id='{}', chat_id='{}')", user_id, chat_id);
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
        spdlog::debug("CreateAlarm::create_recurrent(user_id='{}', chat_id='{}')", user_id, chat_id);

        _bot.send_message(chat_id, "Ok. Let's create a recurrent alarm");

        // Select recursion pattern
        std::vector<std::vector<std::string>> options;
        std::vector<std::string> row;
        int i = 0;
        for (auto& it: db::DatePatterStr) {
            row.push_back(it);
            if (++i % 2 == 0) {
                options.push_back(row);
                row.clear();
            }
        }
        if (row.size()) options.push_back(row);
        _bot.send_message(chat_id, "Select recursion:", options);
        _dispatch_query = [this, user_id](TgBot::CallbackQuery::Ptr query) {
            assert(user_id == query->from->id);
            db::DatePattern date_pattern = db::pattern_from_str(query->data);
            _bot.send_message(query->message->chat->id, "Enter a valid hour (format 18:50):");
            _dispatch = [this, user_id, date_pattern](TgBot::Message::Ptr message) {
                assert(user_id == message->from->id);
                std::string hour = message->text;
                auto valid = utils::validate_hour(hour);  // TODO: validate hour
                if (valid) {
                    std::vector<std::vector<std::string>> options = {{"Forever"}};
                    _bot.send_message(message->chat->id, "Last alarm date (format 2019-08-12)?", options);

                    auto on_date_end = [this, user_id, date_pattern, hour](const std::string& date_end, telegram::Bot::chat_id_t chat_id) {
                        std::vector<std::vector<std::string>> options = {{"Today"}};
                        _bot.send_message(chat_id, "First alarm date (format 2019-08-12)?", options);

                        auto on_date_init = [this, user_id, date_pattern, hour, date_end](const std::string& date_init, telegram::Bot::chat_id_t chat_id) {
                            std::vector<std::vector<std::string>> options = {{"Just create the alarm"}};
                            _bot.send_message(chat_id, "Write a message for this alarm (optional)", options);

                            _dispatch = [this, user_id, date_pattern, hour, date_end, date_init](TgBot::Message::Ptr message) {
                                assert(user_id == message->from->id);
                                this->db_create_alarm_recurrent(user_id, date_pattern, hour, date_end, date_init, message->text);
                                this->end_task(message->chat->id, "Recurrent alarm created!");
                            };

                            _dispatch_query = [this, user_id, date_pattern, hour, date_end, date_init](TgBot::CallbackQuery::Ptr query) {
                                assert(user_id == query->from->id);
                                assert(query->data == "Just create the alarm");
                                this->db_create_alarm_recurrent(user_id, date_pattern, hour, date_end, date_init, "");
                                this->end_task(query->message->chat->id, "Recurrent alarm created!");
                            };
                        };

                        _dispatch_query = [this, user_id, on_date_init](TgBot::CallbackQuery::Ptr query) {
                            assert(user_id == query->from->id);
                            on_date_init("", query->message->chat->id);
                        };
                        _dispatch = [this, user_id, on_date_init](TgBot::Message::Ptr message) {
                            assert(user_id == message->from->id);
                            std::string date_init = message->text;
                            auto valid = utils::validate_date(date_init);
                            if (valid) {
                                on_date_init(date_init, message->chat->id);
                            }
                            else {
                                _bot.send_message(message->chat->id, "Invalid date format, enter a valid date (format 2019-08-12):");
                            }
                        };
                    };

                    _dispatch_query = [this, user_id, on_date_end](TgBot::CallbackQuery::Ptr query) {
                        assert(user_id == query->from->id);
                        on_date_end("", query->message->chat->id);
                    };
                    _dispatch = [this, user_id, on_date_end](TgBot::Message::Ptr message) {
                        assert(user_id == message->from->id);
                        std::string date_end = message->text;
                        auto valid = utils::validate_date(date_end);
                        if (valid) {
                            on_date_end(date_end, message->chat->id);
                        }
                        else {
                            _bot.send_message(message->chat->id, "Invalid date format, enter a valid date (format 2019-08-12):");
                        }
                    };
                }
                else {
                    _bot.send_message(message->chat->id, "Invalid hour format, enter a valid hour (format 18:50):");
                }
            };
        };
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