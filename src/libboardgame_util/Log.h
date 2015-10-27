//-----------------------------------------------------------------------------
/** @file libboardgame_util/Log.h
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

#ifndef LIBBOARDGAME_UTIL_LOG_H
#define LIBBOARDGAME_UTIL_LOG_H

#include <sstream>
#include <string>

namespace libboardgame_util {

using namespace std;

//-----------------------------------------------------------------------------

/** The log stream.
    Initialized with a platform-dependent default value. Set to nullptr to
    disable logging. */
extern ostream* log_stream;

inline void flush_log()
{
    if (log_stream)
        log_stream->flush();
}

/** Initializes the logging functionality.
    This is necessary to call on some platforms at the start of the program
    before any calls to log().
    @see LogInitializer */
void log_init();

/** Closes the logging functionality.
    This is necessary to call on some platforms before the program exits.
    @see LogInitializer */
void log_close();

/** Write a string to the log stream.
    Appends a newline if the output has no newline at the end. */
void log(const string& s);

/** Helper function needed for log(const Ts&...) */
template<typename T>
void log_buffered(ostream& buffer, const T& t)
{
    buffer << t;
}

/** Helper function needed for log(const Ts&...) */
template<typename T, typename... Ts>
void log_buffered(ostream& buffer, const T& first, const Ts&... rest)
{
    buffer << first;
    log_buffered(buffer, rest...);
}

/** Write a number of arguments to the log stream.
    Writes to a buffer first so there is only a single write to the log
    stream. Appends a newline if the output has no newline at the end. */
template<typename... Ts>
void log(const Ts&... args)
{
    if (! log_stream)
        return;
    ostringstream buffer;
    log_buffered(buffer, args...);
    log(buffer.str());
}

//-----------------------------------------------------------------------------

/** Automatically calls log_init() and log_close() at beginning and end
    of lifetime. */
class LogInitializer
{
public:
    LogInitializer()
    {
        log_init();
    }

    ~LogInitializer()
    {
        log_close();
    }
};

//-----------------------------------------------------------------------------

} // namespace libboardgame_util

#endif // LIBBOARDGAME_UTIL_LOG_H
