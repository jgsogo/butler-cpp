
#include "telegram/bot.h"

#include "tgbot/tgbot.h"
#include "spdlog/spdlog.h"


namespace telegram {

    struct Bot::Impl {
        Impl(const char* token) : _bot(token) {}
        TgBot::Bot _bot;
        std::map<std::string, std::string> _help_messages;
        std::map<int32_t, std::function<void(TgBot::Message::Ptr message)>> _delegations;
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

        pImpl->_bot.getEvents().onAnyMessage([this](TgBot::Message::Ptr message){
            this->on_any_message(message);
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

    void Bot::on_any_message(TgBot::Message::Ptr message) {
        // If it is a command, hold back control (it it was delegated
        if (StringTools::startsWith(message->text, "/")) {
            this->hold_back(message->chat->id);
            return;
        }

        auto it = pImpl->_delegations.find(message->chat->id);
        if (it != pImpl->_delegations.end()) {
            spdlog::debug("Bot::on_any_message 'chat={}' (delegated): msg='{}'", message->chat->id, message->text);
            it->second(message);
            return;
        }
        spdlog::debug("Bot::on_any_message 'chat={}' (lost): msg='{}'", message->chat->id, message->text);
    }

    void Bot::delegate(int32_t chat, std::function<void(TgBot::Message::Ptr message)> func) {
        spdlog::debug("Bot::delegate '{}'", chat);
        pImpl->_delegations[chat] = std::move(func);
    }

    void Bot::hold_back(int32_t chat) {
        spdlog::debug("Bot::hold_back '{}'", chat);
        auto it = pImpl->_delegations.find(chat);
        if (it != pImpl->_delegations.end()) {
            pImpl->_delegations.erase(it);
        }
    }

}
