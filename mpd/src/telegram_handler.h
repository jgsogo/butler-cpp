
#pragma once

#include "telegram/bot.h"
#include "wrapper.h"

namespace mpd {
    class MpdHandler {
    public:
        explicit MpdHandler(telegram::Bot&);

        static void register_command(telegram::Bot&);
        void init(telegram::Bot::chat_id_t chat_id);
        void on_any_message(TgBot::Message::Ptr message);
        void on_callback_query(TgBot::CallbackQuery::Ptr query);
        void end(telegram::Bot::chat_id_t chat_id);
        void end_task(telegram::Bot::chat_id_t chat_id, const std::string&);

        void play(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id);
        void stop(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id);
        void volume(telegram::Bot::chat_id_t user_id, telegram::Bot::chat_id_t chat_id);

    protected:

    protected:
        telegram::Bot& _bot;
        std::function<void (TgBot::Message::Ptr)> _dispatch;
        std::function<void (TgBot::CallbackQuery::Ptr)> _dispatch_query;
    };
}
