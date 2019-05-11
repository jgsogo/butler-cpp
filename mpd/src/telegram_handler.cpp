
#include "telegram_handler.h"

#include <boost/lexical_cast.hpp>
#include "spdlog/spdlog.h"
#include "wrapper.h"

namespace mpd {

    namespace {
        const std::string command = "mpd";
        std::map<telegram::Bot::chat_id_t, MpdHandler> _users;
        const std::string back_to_mpd = "<< Back to MPD";
        const std::string back_to_main = "<< Main menu";

        // subcommands
        const std::string play_command = "Play";
        const std::string stop_command = "Stop";
        const std::string volume_command = "Volume";
        const std::string next_command = "Next";
        const std::string prev_command = "Prev";
        const std::string status_command = "Info";
    }

    MpdHandler::MpdHandler(telegram::Bot& bot) : _bot(bot) {};

    void MpdHandler::register_command(telegram::Bot& bot){
        spdlog::info("Register commands: /{}", command);

        bot.register_command(command, [&bot](TgBot::Message::Ptr message) {
            auto [it, inserted] = _users.insert(std::make_pair(message->chat->id, MpdHandler{bot}));
            if (!inserted) {
                spdlog::error("MpdHandler for '{}' was not previously removed!", message->chat->id);
            }
            MpdHandler& mpd_handler = it->second;
            bot.delegate(message->chat->id, [&mpd_handler](TgBot::Message::Ptr message) {
                             mpd_handler.on_any_message(message);
                         },
                         [&mpd_handler](TgBot::CallbackQuery::Ptr query) {
                             mpd_handler.on_callback_query(query);
                         });
            mpd_handler.init(message->chat->id);
        }, "Manage MPD");
    }

    void MpdHandler::init(telegram::Bot::chat_id_t chat_id) {
        std::vector<std::vector<std::string>> options = {{play_command, stop_command}, {prev_command, next_command}, {status_command, volume_command}, {back_to_main}};
        _bot.send_message(chat_id, "Choose option from the list below (/help):", options);
        _dispatch_query = [this](TgBot::CallbackQuery::Ptr query) {
            const std::string& input = query->data;
            if (input == play_command) {
                this->play(query->from->id, query->message->chat->id);
            }
            else if (input == stop_command) {
                this->stop(query->from->id, query->message->chat->id);
            }
            else if (input == volume_command) {
                this->volume(query->from->id, query->message->chat->id);
            }
            else if (input == status_command) {
                this->status(query->from->id, query->message->chat->id);
            }
            else if (input == next_command) {
                this->next(query->from->id, query->message->chat->id);
            }
            else if (input == prev_command) {
                this->prev(query->from->id, query->message->chat->id);
            }
            else {
                _bot.send_message(query->message->chat->id, "I cannot understand you, use the menus.");
                this->init(query->message->chat->id);
            }
        };
    }

    void MpdHandler::on_callback_query(TgBot::CallbackQuery::Ptr query) {
        spdlog::info("MpdHandler::on_callback_query(chat_id='{}', data='{}', message='{}')", query->message->chat->id, query->data, query->message->text);
        const std::string& input = query->data;
        if (input == back_to_mpd) {
            this->init(query->message->chat->id);
        }
        else if (input == back_to_main) {
            this->end(query->message->chat->id);
        }
        else {
            _dispatch_query(query);
        }
    }

    void MpdHandler::on_any_message(TgBot::Message::Ptr message) {
        spdlog::info("MpdHandler::on_any_message(chat_id='{}', message='{}')", message->chat->id, message->text);
        const std::string& input = message->text;
        if (input == back_to_mpd) {
            this->init(message->chat->id);
        }
        else if (input == back_to_main) {
            this->end(message->chat->id);
        }
        else {
            _dispatch(message);
        }
    }

    void MpdHandler::end(telegram::Bot::chat_id_t chat_id) {
        spdlog::debug("MpdHandler::end(chat_id='{}')", chat_id);
        _users.erase(_users.find(chat_id));
        _bot.send_message(chat_id, "Waiting for your order. /help");
        _bot.hold_back(chat_id);
    }

    void MpdHandler::end_task(telegram::Bot::chat_id_t chat_id, const std::string& task) {
        _bot.send_message(chat_id, task);
        this->init(chat_id);
    }

    void MpdHandler::play(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id) {
        Mpd::instance().play();
        this->end_task(chat_id, Mpd::instance().status()); // TODO: get info about the song
    }

    void MpdHandler::stop(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id) {
        Mpd::instance().stop();
        this->end_task(chat_id, "Stop!");
    }

    void MpdHandler::next(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id) {
        Mpd::instance().next();
        this->end_task(chat_id, Mpd::instance().status());
    }

    void MpdHandler::prev(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id) {
        Mpd::instance().prev();
        this->end_task(chat_id, Mpd::instance().status());
    }

    void MpdHandler::status(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id) {
        this->end_task(chat_id, Mpd::instance().status());
    }

    void MpdHandler::volume(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id) {
        std::vector<std::vector<std::string>> options = {{"0", "20"}, {"50", "100"}, {back_to_mpd, back_to_main}};
        _bot.send_message(chat_id, "Select a value or enter one:", options);

        auto on_value = [this, user_id](telegram::Bot::chat_id_t chat_id, const std::string& value) {
            try
            {
                auto volume_value = boost::lexical_cast<short>(value);
                Mpd::instance().volume(volume_value);
                this->end_task(chat_id, Mpd::instance().status());
            }
            catch(boost::bad_lexical_cast &)
            {
                _bot.send_message(chat_id, "Invalid volume value.");
                this->volume(user_id, chat_id);
            }
        };

        _dispatch = [this, user_id, on_value](TgBot::Message::Ptr message) {
            assert(user_id == message->from->id);
            on_value(message->chat->id, message->text);
        };
        _dispatch_query = [this, user_id, on_value](TgBot::CallbackQuery::Ptr query) {
            assert(user_id == query->from->id);
            on_value(query->message->chat->id, query->data);
        };
    }


}