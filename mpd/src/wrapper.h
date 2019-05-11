
#pragma once

#include <memory>

namespace mpd {

    class Mpd {
    public:
        ~Mpd();

        static Mpd& instance();

        void play();
        void stop();
        void volume(short value);
        void next();
        void prev();
        std::string status();
        std::vector<std::string> playlists();
        void playlist(const std::string&);
    protected:
        Mpd();

    protected:
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };
}