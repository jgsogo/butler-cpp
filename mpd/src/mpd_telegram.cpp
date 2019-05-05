
#include "mpd/mpd_telegram.h"

#include "spdlog/spdlog.h"
#include "telegram_handler.h"

namespace mpd {

    void telegram_commands(telegram::Bot &bot) {
        MpdHandler::register_command(bot);
    }
}