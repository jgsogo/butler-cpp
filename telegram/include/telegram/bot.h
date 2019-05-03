
#pragma once

#include <memory>
#include <functional>
#include <tgbot/tgbot.h>

namespace telegram {
    class Bot {
    public:
        typedef decltype(TgBot::Chat::id) chat_id_t;

        Bot(const char* token);
        ~Bot();

        void register_command(const std::string &command, std::function<void(TgBot::Message::Ptr message)> func,
                              const std::string &help_message);

        void run();

        void send_message(chat_id_t chat, const std::string &message) const;
        void send_message(chat_id_t chat, const std::string &message,
                          const std::vector<std::vector<std::string>> &keyboard) const;

        void delegate(chat_id_t chat, std::function<void(TgBot::Message::Ptr message)> f1,
                      std::function<void(TgBot::CallbackQuery::Ptr message)> f2);
        void hold_back(chat_id_t chat);

    protected:
        void on_any_message(TgBot::Message::Ptr message);
        void on_callback_query(TgBot::CallbackQuery::Ptr query);

    protected:
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };
}
