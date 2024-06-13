//-----------------------------------------------------------------------------
/** @file twogtp/Output.h
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

#ifndef TWOGTP_OUTPUT_H
#define TWOGTP_OUTPUT_H

#include <string>
#include <map>
#include <mutex>
#include "OutputTree.h"
#include "libboardgame_base/Timer.h"
#include "libboardgame_base/WallTimeSource.h"

using libboardgame_base::Timer;
using libboardgame_base::WallTimeSource;

//-----------------------------------------------------------------------------

/** Handles the output files of TwoGtp and their concurrent access. */
class Output
{
public:
    Output(Variant variant, const string& prefix, bool create_tree);

    ~Output();

    void set_save_interval(double seconds) { m_save_interval = seconds; }

    void add_result(unsigned n, float result, const Board& bd,
                    unsigned first_player, vector<double> cpu,
                    const string& sgf,
                    const array<bool, Board::max_moves>& is_real_move);

    unsigned get_next();

    bool check_sentinel();

    bool generate_fast_open_move(bool is_first_player, const Board& bd,
                                 Color to_play, Move& mv);

private:
    bool m_create_tree;

    unsigned m_next = 0;

    int m_lock_fd;

    string m_prefix;

    mutex m_mutex;

    map<unsigned, string> m_games;

    OutputTree m_output_tree;

    ostringstream m_sgf_buffer;

    WallTimeSource m_time_source;

    Timer m_timer;

    double m_save_interval = 60;

    void save();
};

//-----------------------------------------------------------------------------

#endif // TWOGTP_OUTPUT_H
