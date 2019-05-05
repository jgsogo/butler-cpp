
#include "wrapper.h"

#include <mpd/client.h>
#include <mpd/status.h>
#include <mpd/entity.h>
#include <mpd/search.h>
#include <mpd/tag.h>
#include <mpd/message.h>

#include "spdlog/spdlog.h"

namespace mpd {

    namespace {
        int handle_error(struct mpd_connection *c) {
            assert(mpd_connection_get_error(c) != MPD_ERROR_SUCCESS);

            fprintf(stderr, "%s\n", mpd_connection_get_error_message(c));
            // mpd_connection_free(c);
            return EXIT_FAILURE;
        }

        void run_commands(mpd_connection* conn, std::function<void ()> commands) {
            mpd_command_list_begin(conn, true);
            commands();
            mpd_command_list_end(conn);

            // TODO: I consume all the pairs, why? Investigate!!
            struct mpd_pair *pair;
            while ((pair = mpd_recv_pair(conn)) != NULL) {
                mpd_return_pair(conn, pair);
            }


            auto status = mpd_recv_status(conn);
            if (status == nullptr) {
                handle_error(conn);
                spdlog::error("Error fatal. {}", mpd_connection_get_error_message(conn));
                return;
            }

            if (mpd_status_get_error(status) != nullptr) {
                spdlog::error(mpd_status_get_error(status));
            }

            mpd_status_free(status);
        }
    }

    struct Mpd::Impl {
        struct mpd_connection *conn;
    };

    Mpd::Mpd() : pImpl(std::make_unique<Impl>()) {
        pImpl->conn = mpd_connection_new(NULL, 0, 30000);

        if (mpd_connection_get_error(pImpl->conn) != MPD_ERROR_SUCCESS) {
            handle_error(pImpl->conn);
            pImpl->conn = nullptr;
        }
    }

    Mpd::~Mpd() {
        if (pImpl->conn != nullptr) {
            mpd_connection_free(pImpl->conn);
        }
    }

    Mpd& Mpd::instance() {
        static Mpd mpd;
        return mpd;
    }

    void Mpd::play() {
        spdlog::debug("Mpd::play()");
        run_commands(pImpl->conn, [this](){
            mpd_send_play(pImpl->conn);
        });
    }

    void Mpd::stop() {
        spdlog::debug("Mpd::stop()");
        run_commands(pImpl->conn, [this](){
            mpd_send_stop(pImpl->conn);
        });
    }

    void Mpd::volume(short value) {
        spdlog::debug("Mpd::volume(value='{}')", value);
        run_commands(pImpl->conn, [this, value](){
            mpd_send_set_volume(pImpl->conn, value);
        });
    }
}