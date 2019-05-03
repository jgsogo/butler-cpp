
#pragma once

#include "telegram/bot.h"


namespace bot {
    class CreateAlarm {
    public:
        CreateAlarm(telegram::Bot&);

        void init(TgBot::Message::Ptr message);
        void create_fixed(TgBot::Message::Ptr message);
        void create_recurrent(TgBot::Message::Ptr message);

        void on_any_message(TgBot::Message::Ptr message);
        void on_callback_query(TgBot::CallbackQuery::Ptr query);
        void end(TgBot::Message::Ptr message);
        static void register_command(telegram::Bot&);

    protected:
        void create_alarm_fixed(telegram::Bot::chat_id_t user_id, std::string date, std::string hour, std::string message);

    protected:
        telegram::Bot& _bot;
        std::function<void (TgBot::Message::Ptr)> _dispatch;
        std::function<void (TgBot::CallbackQuery::Ptr)> _dispatch_query;
    };
}