//-----------------------------------------------------------------------------
/** @file libboardgame_gtp/CmdLine.h
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

#ifndef LIBBOARDGAME_GTP_CMDLINE_H
#define LIBBOARDGAME_GTP_CMDLINE_H

#include <cassert>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace libboardgame_gtp {

using namespace std;

//-----------------------------------------------------------------------------

/** Parsed GTP command line.
    Only used internally by libboardgame_gtp::Engine. GTP command handlers
    query arguments of the command line through the instance of class Arguments
    given as a function argument by class Engine to the command handler. */
class CmdLine
{
public:
    /** Construct empty command.
        @warning An empty command cannot be used, before init() was called.
        This constructor exists only to reuse instances. */
    CmdLine() = default;

    /** Construct with a command line.
        @see init() */
    explicit CmdLine(const string& line);

    ~CmdLine();

    void init(const string& line);

    const string& get_line() const { return m_line; }

    /** Get command name. */
    string_view get_name() const { return m_elem[m_idx_name]; }


    void write_id(ostream& out) const;

    string_view get_trimmed_line_after_elem(unsigned i) const;

    const vector<string_view>& get_elements() const { return m_elem; }


    const string_view& get_element(unsigned i) const;

    unsigned get_idx_name() const { return m_idx_name; }

private:
    unsigned m_idx_name;

    /** Full command line. */
    string m_line;

    vector<string_view> m_elem;

    void add_elem(string::const_iterator begin, string::const_iterator end);

    void find_elem();

    void parse_id();
};

inline const string_view& CmdLine::get_element(unsigned i) const
{
    assert(i < m_elem.size());
    return m_elem[i];
}

inline void CmdLine::write_id(ostream& out) const
{
    if (m_idx_name != 0)
        out << m_elem[0];
}

//-----------------------------------------------------------------------------

} // namespace libboardgame_gtp

#endif // LIBBOARDGAME_GTP_CMDLINE_H
