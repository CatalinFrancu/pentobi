//-----------------------------------------------------------------------------
/** @file pentobi_gtp/GtpEngine.cpp
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

#include "GtpEngine.h"

#include <fstream>
#include "libboardgame_base/Writer.h"
#include "libpentobi_mcts/Util.h"

using libboardgame_base::Writer;
using libboardgame_gtp::Failure;
using libpentobi_base::Board;
using libpentobi_mcts::Float;

//-----------------------------------------------------------------------------

GtpEngine::GtpEngine(
        Variant variant, unsigned level, bool use_book,
        const string& books_dir, unsigned nu_threads)
    : libpentobi_gtp::GtpEngine(variant)
{
    create_player(variant, level, books_dir, nu_threads);
    get_mcts_player().set_use_book(use_book);
    add("get_value", &GtpEngine::cmd_get_value);
    add("name", &GtpEngine::cmd_name);
    add("param", &GtpEngine::cmd_param);
    add("move_values", &GtpEngine::cmd_move_values);
    add("save_tree", &GtpEngine::cmd_save_tree);
    add("selfplay", &GtpEngine::cmd_selfplay);
    add("version", &GtpEngine::cmd_version);
}

GtpEngine::~GtpEngine() = default; // Non-inline to avoid GCC -Winline warning

void GtpEngine::cmd_get_value(Response& response)
{
    response << get_search().get_tree().get_root().get_value();
}

void GtpEngine::cmd_move_values(Response& response)
{
    auto children = get_search().get_tree().get_root_children();
    if (children.empty())
        return;
    auto& bd = get_board();
    vector<const Search::Node*> sorted_children;
    sorted_children.reserve(children.size());
    for (auto& i : children)
        sorted_children.push_back(&i);
    sort(sorted_children.begin(), sorted_children.end(), libpentobi_mcts::compare_node);
    response << fixed;
    for (auto node : sorted_children)
        response << setprecision(0) << node->get_visit_count() << ' '
                 << setprecision(1) << node->get_value_count() << ' '
                 << setprecision(3) << node->get_value() << ' '
                 << bd.to_string(node->get_move(), true) << '\n';
}

void GtpEngine::cmd_name(Response& response)
{
    response.set("Pentobi");
}

void GtpEngine::cmd_save_tree(Arguments args)
{
    auto& search = get_search();
    if (! search.get_last_history().is_valid())
        throw Failure("no search tree");
    ofstream out(args.get<string>());
    libpentobi_mcts::dump_tree(out, search);
}

/** Let the engine play a number of games against itself.
    This is more efficient than using twogtp if selfplay games are needed
    because it has lower memory requirements (only one engine needed), process
    switches between the engines are avoided and parts of the search tree can
    be reused between moves of different players. */
void GtpEngine::cmd_selfplay(Arguments args)
{
    args.check_size(2);
    auto nu_games = args.get<int>(0);
    ofstream out(args.get<string>(1));
    auto variant = get_board().get_variant();
    auto variant_str = to_string(variant);
    Board bd(variant);
    auto& player = get_mcts_player();
    ostringstream s;
    for (int i = 0; i < nu_games; ++i)
    {
        s.str("");
        Writer writer(s);
        writer.set_indent(-1);
        bd.init();
        writer.begin_tree();
        writer.begin_node();
        writer.write_property("GM", variant_str);
        writer.end_node();
        while (! bd.is_game_over())
        {
            auto c = bd.get_effective_to_play();
            auto mv = player.genmove(bd, c);
            bd.play(c, mv);
            writer.begin_node();
            writer.write_property(get_color_id(variant, c),
                                  bd.to_string(mv, false));
            writer.end_node();
        }
        writer.end_tree();
        out << s.str() << '\n';
    }
}

void GtpEngine::cmd_param(Arguments args, Response& response)
{
    auto& p = get_mcts_player();
    auto& s = get_search();
    if (args.get_size() == 0)
        response
            << "avoid_symmetric_draw " << s.get_avoid_symmetric_draw() << '\n'
            << "exploration_constant " << s.get_exploration_constant() << '\n'
            << "fixed_simulations " << p.get_fixed_simulations() << '\n'
            << "rave_child_max " << s.get_rave_child_max() << '\n'
            << "rave_parent_max " << s.get_rave_parent_max() << '\n'
            << "rave_weight " << s.get_rave_weight() << '\n'
            << "reuse_subtree " << s.get_reuse_subtree() << '\n'
            << "use_book " << p.get_use_book() << '\n';
    else
    {
        args.check_size(2);
        auto name = args.get(0);
        if (name == "avoid_symmetric_draw")
            s.set_avoid_symmetric_draw(args.get<bool>(1));
        else if (name == "exploration_constant")
            s.set_exploration_constant(args.get<Float>(1));
        else if (name == "fixed_simulations")
            p.set_fixed_simulations(args.get<Float>(1));
        else if (name == "rave_child_max")
            s.set_rave_child_max(args.get<Float>(1));
        else if (name == "rave_parent_max")
            s.set_rave_parent_max(args.get<Float>(1));
        else if (name == "rave_weight")
            s.set_rave_weight(args.get<Float>(1));
        else if (name == "reuse_subtree")
            s.set_reuse_subtree(args.get<bool>(1));
        else if (name == "use_book")
            p.set_use_book(args.get<bool>(1));
        else
        {
            ostringstream msg;
            msg << "unknown parameter '" << name << "'";
            throw Failure(msg.str());
        }
    }
}

void GtpEngine::cmd_version(Response& response)
{
    string version;
#ifdef VERSION
    version = VERSION;
#else
    version = "unknown";
#endif
#ifdef LIBBOARDGAME_DEBUG
    version.append(" (dbg)");
#endif
    response.set(version);
}

void GtpEngine::create_player(Variant variant, unsigned level,
                           const string& books_dir, unsigned nu_threads)
{
    auto max_level = level;
    m_player = make_unique<Player>(variant, max_level, books_dir, nu_threads);
    get_mcts_player().set_level(level);
    set_player(*m_player);
}

Player& GtpEngine::get_mcts_player()
{
    try
    {
        return dynamic_cast<Player&>(*m_player);
    }
    catch (const bad_cast&)
    {
        throw Failure("current player is not mcts player");
    }
}

Search& GtpEngine::get_search()
{
    return get_mcts_player().get_search();
}

//-----------------------------------------------------------------------------
