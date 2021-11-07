//-----------------------------------------------------------------------------
/** @file libboardgame_gtp/Arguments.h
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

#ifndef LIBBOARDGAME_GTP_ARGUMENTS_H
#define LIBBOARDGAME_GTP_ARGUMENTS_H

#if defined __GNUC__ && __has_include(<cxxabi.h>)
#include <cxxabi.h>
#endif
#include <sstream>
#include "CmdLine.h"
#include "Failure.h"

namespace libboardgame_gtp {

using namespace std;

//-----------------------------------------------------------------------------

/** Access arguments of command line. */
class Arguments
{
public:
    /** Constructor.
        @param line The command line. The lifetime of this parameter
        must exceed the lifetime of the class instance. */
    explicit Arguments(const CmdLine& line) : m_line(line) { }

    /** Get argument.        
        @param i Argument index starting with 0
        @return Argument value        
        @throws Failure If no such argument
        @tparam T The type the argument should be converted to. The type
        must implement operator<< */
    template<typename T = string_view>
    T get(unsigned i) const;

    /** Get single argument.        
        Like get(unsigned) but throws if there is not exactly one argument. */
    template<typename T = string_view>
    T get() const;

    /** Get argument converted to lowercase.
        @param i Argument index starting with 0
        @return Copy of argument value converted to lowercase
        @throws Failure If no such argument */
    string get_tolower(unsigned i) const;

    /** Get argument and check against a minimum value.
        Like get(unsigned) but throws if the argument is less than the minimum
        value. */
    template<typename T>
    T get_min(unsigned i, T min) const;

    /** Check that command has no arguments.
        @throws Failure If command has arguments
    */
    void check_empty() const { check_size(0); }

    /** Check number of arguments.
        @param n Expected number of arguments
        @throws Failure If command has a different number of arguments */
    void check_size(unsigned n) const;

    /** Check maximum number of arguments.
        @param n Expected maximum number of arguments
        @throws Failure If command has more arguments */
    void check_size_less_equal(unsigned n) const;

    /** Get argument line.
        Get all arguments as a line.
        No modifications to the line were made apart from trimmimg leading
        and trailing white spaces. */
    string_view get_line() const;

    /** Get number of arguments. */
    unsigned get_size() const;

    /** Return remaining line after argument.
        @param i Argument index starting with 0
        @return The remaining line after the given argument, unmodified apart
        from leading and trailing whitespaces, which are trimmed. Quotation
        marks are not handled.
        @throws Failure If no such argument */
    string_view get_remaining_line(unsigned i) const;

private:
    const CmdLine& m_line;

    template<typename T>
    static string get_type_name();
};

inline string_view Arguments::get_line() const
{
    return m_line.get_trimmed_line_after_elem(m_line.get_idx_name());
}

inline unsigned Arguments::get_size() const
{
    return
        static_cast<unsigned>(m_line.get_elements().size())
        - m_line.get_idx_name() - 1;
}

template<typename T>
string Arguments::get_type_name()
{
#if defined __GNUC__ && __has_include(<cxxabi.h>)
    int status;
    auto name_ptr =
        abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
    if (status == 0)
    {
        string result(name_ptr);
        free(name_ptr);
        return result;
    }
#endif
    return typeid(T).name();
}

template<typename T>
T Arguments::get() const
{
    check_size(1);
    return get<T>(0);
}

template<>
string_view Arguments::get(unsigned i) const;

template<>
string Arguments::get(unsigned i) const;

template<typename T>
T Arguments::get(unsigned i) const
{
    auto s = get<string>(i);
    istringstream in(s);
    T result;
    in >> result;
    if (! in)
    {
        ostringstream msg;
        msg << "argument " << (i + 1) << " ('" << s
            << "') has invalid type (expected " << get_type_name<T>() << ")";
        throw Failure(msg.str());
    }
    return result;
}

template<typename T>
T Arguments::get_min(unsigned i, T min) const
{
    auto result = get<T>(i);
    if (result < min)
    {
        ostringstream msg;
        msg << "argument " << (i + 1) << " must be greater or equal " << min;
        throw Failure(msg.str());
    }
    return result;
}

//-----------------------------------------------------------------------------

} // namespace libboardgame_gtp

#endif // LIBBOARDGAME_GTP_ARGUMENTS_H
