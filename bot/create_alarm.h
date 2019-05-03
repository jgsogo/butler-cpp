
#pragma once

#include "telegram/bot.h"


namespace bot {
    class CreateAlarm {
    public:
        CreateAlarm(telegram::Bot&);

        void on_any_message(TgBot::Message::Ptr message);

        static void register_command(telegram::Bot&);
    protected:
        telegram::Bot& _bot;
    };
}