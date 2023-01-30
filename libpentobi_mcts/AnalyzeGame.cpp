//-----------------------------------------------------------------------------
/** @file libpentobi_mcts/AnalyzeGame.cpp
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

#include "AnalyzeGame.h"

#include "Search.h"
#include "libboardgame_base/WallTimeSource.h"

namespace libpentobi_mcts {

using libboardgame_base::SgfError;
using libboardgame_base::WallTimeSource;
using libpentobi_base::BoardUpdater;

//-----------------------------------------------------------------------------

void AnalyzeGame::clear()
{
    m_moves.clear();
    m_values.clear();
}

void AnalyzeGame::run(const Game& game, Search& search, size_t nu_simulations,
                      const function<void()>& progress_callback)
{
    m_variant = game.get_variant();
    m_moves.clear();
    m_values.clear();
    auto& tree = game.get_tree();
    unique_ptr<Board> bd(new Board(m_variant));
    BoardUpdater updater;
    auto& root = game.get_root();
    auto node = &root;
    WallTimeSource time_source;
    node = &root;
    auto tie_value = Search::SearchParamConst::tie_value;
    const auto max_count = Float(nu_simulations);
    double max_time = 0;
    // Set min_simulations to a reasonable value because nu_simulations can be
    // reached without having that many value updates if a subtree from a
    // previous search is reused (which re-initializes the value and value
    // count of the new root from the best child)
    size_t min_simulations = min(size_t(100), nu_simulations);
    Move dummy;
    do
    {
        auto mv = tree.get_move(*node);
        if (! mv.is_null())
        {
            if (! node->has_parent())
            {
                // Root shouldn't contain moves in SGF files
                m_moves.push_back(mv);
                m_values.push_back(static_cast<double>(tie_value));
            }
            else
            {
                progress_callback();
                try
                {
                    updater.update(*bd, tree, node->get_parent());
                    LIBBOARDGAME_LOG("Analyzing move ", bd->get_nu_moves());
                    search.search(dummy, *bd, mv.color, max_count,
                                  min_simulations, max_time, time_source);
                    if (search.was_aborted())
                        break;
                    m_moves.push_back(mv);
                    m_values.push_back(static_cast<double>(
                                           search.get_root_val().get_mean()));
                }
                catch (const SgfError&)
                {
                    break;
                }
            }
        }
        if (! node->has_children())
        {
            updater.update(*bd, tree, *node);
            LIBBOARDGAME_LOG("Analyzing last position");
            Color c;
            if (bd->is_game_over() && ! m_moves.empty())
                // If game is over, analyze last position from viewpoint of
                // color that played the last move to avoid using a color that
                // might have run out of moves much earlier.
                c = m_moves.back().color;
            else
                c = bd->get_effective_to_play();
            search.search(dummy, *bd, c, max_count, min_simulations, max_time,
                          time_source);
            if (search.was_aborted())
                break;
            m_moves.emplace_back(c, Move::null());
            m_values.push_back(static_cast<double>(
                                   search.get_root_val().get_mean()));
        }
        node = node->get_first_child_or_null();
    }
    while (node != nullptr);
}

void AnalyzeGame::set(Variant variant, const vector<ColorMove>& moves,
                      const vector<double>& values)
{
    LIBBOARDGAME_ASSERT(moves.size() == values.size());
    m_variant = variant;
    m_moves = moves;
    m_values = values;
}

//-----------------------------------------------------------------------------

} // namespace libpentobi_mcts
