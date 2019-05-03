
#pragma once

#include "telegram/bot.h"


namespace bot {
    class CreateAlarm {
    public:
        CreateAlarm(telegram::Bot&);

        void init(TgBot::Message::Ptr message);
        void list_alarms(TgBot::Message::Ptr message);
        void create_fixed(TgBot::Message::Ptr message);
        void create_recurrent(TgBot::Message::Ptr message);

        void on_any_message(TgBot::Message::Ptr message);
        void on_callback_query(TgBot::CallbackQuery::Ptr query);
        void end(TgBot::Message::Ptr message);
        void end_task(TgBot::Message::Ptr message, const std::string&);
        static void register_command(telegram::Bot&);

    protected:
        void db_create_alarm_fixed(telegram::Bot::chat_id_t user_id, std::string date, std::string hour,
                                   std::string message);
        std::vector<std::string> db_list_alarms(telegram::Bot::chat_id_t user_id, int count = 10, int init = 0);


    protected:
        telegram::Bot& _bot;
        std::function<void (TgBot::Message::Ptr)> _dispatch;
        std::function<void (TgBot::CallbackQuery::Ptr)> _dispatch_query;
    };
}