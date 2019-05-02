
#include "telegram/bot.h"

#include "tgbot/tgbot.h"
#include "spdlog/spdlog.h"


namespace telegram {

    struct Bot::Impl {
        Impl(const char* token) : _bot(token) {}
        TgBot::Bot _bot;
        std::map<std::string, std::string> _help_messages;
    };

    Bot::Bot(const char* token) : pImpl(std::make_unique<Impl>(token)) {}
    Bot::~Bot() {}

    void Bot::register_command(const std::string &command, std::function<void(TgBot::Message::Ptr message)> func,
                               const std::string &help_message) {
        spdlog::debug("Register command '{}': {}", command, help_message);
        pImpl->_bot.getEvents().onCommand(command, func);
        pImpl->_help_messages[command] = help_message;
    }

    void Bot::run() {
        pImpl->_bot.getEvents().onCommand("help", [this](TgBot::Message::Ptr message) {
            std::stringstream ss;
            ss << "This is the bot. Commands:\n";
            for (auto& [command, help_text]: pImpl->_help_messages) {
                ss << " /" << command << ": " << help_text << "\n";
            }
            pImpl->_bot.getApi().sendMessage(message->chat->id, ss.str());
        });

        spdlog::info("Launching Telegram bot");
        TgBot::TgLongPoll longPoll(pImpl->_bot);
        while (true) {
            longPoll.start();
        }
    }

    void Bot::send_message(int32_t chat, const std::string& message) const {
        pImpl->_bot.getApi().sendMessage(chat, message);
    }
}
