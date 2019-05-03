
#include "telegram/bot.h"

#include "tgbot/tgbot.h"
#include "spdlog/spdlog.h"


namespace telegram {

    struct Bot::Impl {
        Impl(const char* token) : _bot(token) {}
        TgBot::Bot _bot;
        std::map<std::string, std::string> _help_messages;
        std::map<Bot::chat_id_t, std::pair<std::function<void(TgBot::Message::Ptr)>, std::function<void(TgBot::CallbackQuery::Ptr)>>> _delegations;
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

        pImpl->_bot.getEvents().onCallbackQuery([this](TgBot::CallbackQuery::Ptr query) {
            this->on_callback_query(query);
        });

        try {
            spdlog::info("Launching Telegram bot");
            pImpl->_bot.getApi().deleteWebhook();
            TgBot::TgLongPoll longPoll(pImpl->_bot);
            while (true) {
                longPoll.start();
            }
        }
        catch (TgBot::TgException& e) {
            spdlog::error("TgBot::TgLongPoll error: {}", e.what());
        }
    }

    void Bot::send_message(chat_id_t chat, const std::string &message) const {
        pImpl->_bot.getApi().sendMessage(chat, message, false, 0, std::make_shared<TgBot::ReplyKeyboardRemove>());
    }

    void Bot::send_message(chat_id_t chat, const std::string &message,
                           const std::vector<std::vector<std::string>> &options) const {
        TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
        // keyboard->oneTimeKeyboard = true;

        for (auto& options_row: options) {
            std::vector<TgBot::InlineKeyboardButton::Ptr> row;
            for (auto& options_cell: options_row) {
                TgBot::InlineKeyboardButton::Ptr item(new TgBot::InlineKeyboardButton);
                item->text = options_cell;
                item->callbackData = options_cell;
                row.push_back(item);
            }
            keyboard->inlineKeyboard.push_back(row);
        }

        pImpl->_bot.getApi().sendMessage(chat, message, false, 0, keyboard);
    }

    void Bot::on_any_message(TgBot::Message::Ptr message) {
        spdlog::debug("Bot::on_any_message(message/chat/id='{}', message='{}')", message->chat->id, message->text);
        // If it is a command, hold back control (it it was delegated
        if (StringTools::startsWith(message->text, "/")) {
            this->hold_back(message->chat->id);
            return;
        }

        auto it = pImpl->_delegations.find(message->chat->id);
        if (it != pImpl->_delegations.end()) {
            spdlog::debug("Bot::on_any_message 'chat={}' (delegated): msg='{}'", message->chat->id, message->text);
            it->second.first(message);
            return;
        }
        spdlog::debug("Bot::on_any_message 'chat={}' (lost): msg='{}'", message->chat->id, message->text);
        this->send_message(message->chat->id, "Waiting for your order. /help");
    }

    void Bot::on_callback_query(TgBot::CallbackQuery::Ptr query) {
        spdlog::debug("Bot::on_callback_query(query/chat/id='{}')", query->message->chat->id);
        auto it = pImpl->_delegations.find(query->message->chat->id);
        if (it != pImpl->_delegations.end()) {
            spdlog::debug("Bot::on_callback_query 'chat={}' (delegated): data='{}', msg='{}'", query->message->chat->id, query->data, query->message->text);
            it->second.second(query);
            return;
        }
        spdlog::debug("Bot::on_callback_query 'chat={}' (lost): data='{}', msg='{}'", query->message->chat->id, query->data, query->message->text);
        this->send_message(query->message->chat->id, "Waiting for your order. /help");
    }


    void Bot::delegate(chat_id_t chat, std::function<void(TgBot::Message::Ptr message)> f1,
                       std::function<void(TgBot::CallbackQuery::Ptr message)> f2) {
        spdlog::debug("Bot::delegate '{}'", chat);
        pImpl->_delegations[chat] = std::make_pair(std::move(f1), std::move(f2));
    }

    void Bot::hold_back(chat_id_t chat) {
        spdlog::debug("Bot::hold_back '{}'", chat);
        auto it = pImpl->_delegations.find(chat);
        if (it != pImpl->_delegations.end()) {
            pImpl->_delegations.erase(it);
        }
    }

}
