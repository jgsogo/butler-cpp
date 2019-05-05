
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

    protected:
        Mpd();

    protected:
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };
}