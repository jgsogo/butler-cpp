
#pragma once

#include "telegram/bot.h"


namespace bot {
    class CreateAlarm {
    public:
        CreateAlarm(telegram::Bot&);

        void init(telegram::Bot::chat_id_t chat_id);
        void list_alarms(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id);
        void create_fixed(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id);
        void create_recurrent(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id);

        void on_any_message(TgBot::Message::Ptr message);
        void on_callback_query(TgBot::CallbackQuery::Ptr query);
        void end(telegram::Bot::chat_id_t chat_id);
        void end_task(telegram::Bot::chat_id_t chat_id, const std::string&);
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