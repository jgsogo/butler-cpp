
#pragma once

#include <memory>
#include <functional>
#include <tgbot/tgbot.h>

namespace telegram {
    class Bot {
    public:
        Bot(const char* token);
        ~Bot();

        void register_command(const std::string &command, std::function<void(TgBot::Message::Ptr message)> func,
                              const std::string &help_message);

        void run();

        void send_message(int32_t chat, const std::string& message) const;
        void delegate(int32_t chat, std::function<void(TgBot::Message::Ptr message)> func);
        void hold_back(int32_t chat);

    protected:
        void on_any_message(TgBot::Message::Ptr message);

    protected:
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };
}
