
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

        void run_commands(mpd_connection* , std::function<void (mpd_connection *conn)> commands, std::function<void(mpd_connection *conn)> listen = [](mpd_connection *conn){}) {
            mpd_connection* conn = mpd_connection_new(NULL, 0, 30000);
            if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
                handle_error(conn);
                return;
            }


            mpd_command_list_begin(conn, true);
            mpd_send_status(conn);
            commands(conn);
            mpd_command_list_end(conn);

            // TODO: I consume all the pairs, why? Investigate!!
            //struct mpd_pair *pair;
            //while ((pair = mpd_recv_pair(conn)) != NULL) {
            //    mpd_return_pair(conn, pair);
            //}


            auto status = mpd_recv_status(conn);
            if (status == nullptr) {
                handle_error(conn);
                spdlog::error("Error fatal. {}", mpd_connection_get_error_message(conn));
                return;
            }

            //if (mpd_status_get_error(status) != nullptr) {
            //    spdlog::error(mpd_status_get_error(status));
            //}

            mpd_status_free(status);

            if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
                handle_error(conn);
                return;
            }

            mpd_response_next(conn);

            listen(conn);

            if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
                handle_error(conn);
                return;
            }

            mpd_connection_free(conn);
        }
    }

    struct Mpd::Impl {
        struct mpd_connection *conn;
    };

    Mpd::Mpd() : pImpl(std::make_unique<Impl>()) {
        //pImpl->conn = mpd_connection_new(NULL, 0, 30000);

        //if (mpd_connection_get_error(pImpl->conn) != MPD_ERROR_SUCCESS) {
        //    handle_error(pImpl->conn);
        //    pImpl->conn = nullptr;
        //}
    }

    Mpd::~Mpd() {
        //if (pImpl->conn != nullptr) {
        //    mpd_connection_free(pImpl->conn);
        //}
    }

    Mpd& Mpd::instance() {
        static Mpd mpd;
        return mpd;
    }

    void Mpd::play() {
        spdlog::debug("Mpd::play()");
        run_commands(pImpl->conn, [](mpd_connection *conn){
            mpd_send_play(conn);
        });
    }

    void Mpd::stop() {
        spdlog::debug("Mpd::stop()");
        run_commands(pImpl->conn, [](mpd_connection *conn){
            mpd_send_stop(conn);
        });
    }

    void Mpd::volume(short value) {
        spdlog::debug("Mpd::volume(value='{}')", value);
        run_commands(pImpl->conn, [value](mpd_connection *conn){
            mpd_send_set_volume(conn, value);
        });
    }

    void Mpd::next() {
        spdlog::debug("Mpd::next()");
        run_commands(pImpl->conn, [](mpd_connection *conn){
            mpd_send_next(conn);
        });
    }

    void Mpd::prev() {
        spdlog::debug("Mpd::prev()");
        run_commands(pImpl->conn, [](mpd_connection *conn){
            mpd_send_previous(conn);
        });
    }

    std::string Mpd::status() {
        spdlog::debug("Mpd::status()");
        std::string ret = "No song being played right now";
        run_commands(pImpl->conn, [](mpd_connection *conn){
                        mpd_send_current_song(conn);
                    },
                    [&ret](mpd_connection *conn){
                        struct mpd_song *song;
                        while ((song = mpd_recv_song(conn)) != NULL) {
                            const char* artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
                            const char* album = mpd_song_get_tag(song, MPD_TAG_ALBUM, 0);
                            const char* title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
                            ret = fmt::format("{} | {} ({})", artist, title, album);
                            mpd_song_free(song);
                        }
                    });
        return ret;
    }

    std::vector<std::string> Mpd::playlists() {
        spdlog::debug("Mpd::playlists()");
        std::vector<std::string> ret;
        run_commands(pImpl->conn, [](mpd_connection *conn){
                         mpd_send_list_playlists(conn);
                     },
                     [&ret](mpd_connection *conn){
                         struct mpd_playlist *playlist;
                         while ((playlist = mpd_recv_playlist(conn)) != NULL) {
                             const char* it = mpd_playlist_get_path(playlist);
                             ret.push_back(it);
                             mpd_playlist_free(playlist);
                         }
                     });
        return ret;
    }

    void Mpd::playlist(const std::string& playlist) {
        spdlog::debug("Mdp::playlist({})", playlist);
        run_commands(pImpl->conn, [&playlist](mpd_connection *conn){
            mpd_send_clear(conn);
            mpd_send_load(conn, playlist.c_str());
            mpd_send_play(conn);
        });
    }
}