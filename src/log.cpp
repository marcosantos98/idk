#include "log.hpp"

void Log::log_terr(const char *msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    (void)vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);
    printf("\u001b[1m\u001b[31m[%s]\u001b[0m ", m_tag.c_str());
    printf("%s", buffer);
    exit(1);
}

void Log::log_err(const char *msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    (void)vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);
    printf("\u001b[1m\u001b[31m%s\u001b[0m", buffer);
}

void Log::log_tdbg(const char *msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    (void)vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);

    printf("\u001b[1m\u001b[36m[%s]\u001b[0m ", m_tag.c_str());
    printf("%s", buffer);
}

void Log::log_dbg(const char *msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    (void)vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);
    printf("\u001b[1m\u001b[36m%s\u001b[0m", buffer);
}

void Log::log_tok(const char *msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    (void)vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);

    printf("\u001b[1m\u001b[32m[%s]\u001b[0m %s", m_tag.c_str(), buffer);
}

void Log::log_ok(const char *msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    (void)vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);
    printf("\u001b[1m\u001b[32m%s\u001b[0m", buffer);
}

void Log::log_twarn(const char *msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    (void)vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);

    printf("\u001b[1m\u001b[33m[%s]\u001b[0m %s", m_tag.c_str(), buffer);
}

void Log::log_warn(const char *msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    (void)vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);
    printf("\u001b[1m\u001b[33m%s\u001b[0m", buffer);
}

void Log::log_info(const char *msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    (void)vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);
    printf("%s", buffer);
}