#pragma once

#include "nava.hpp"

class Log {
    public:
    Log(String tag) 
        : m_tag(tag) {}

    void log_terr(const char*, ...);
    void log_err(const char*, ...);
    void log_tdbg(const char*, ...);
    void log_dbg(const char*, ...);
    void log_tok(const char*, ...);
    void log_ok(const char*, ...);
    void log_twarn(const char*, ...);
    void log_warn(const char*, ...);
    void log_info(const char*, ...);

    inline void enable_location()
    {
        location = true;
    }

    inline void enable_timestamp()
    {
        time = true;
    }

    private:
        String m_tag;
        bool time = false;
        bool location = false;
};