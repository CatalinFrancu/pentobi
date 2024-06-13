//-----------------------------------------------------------------------------
/** @file twogtp/Main.cpp
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

#include <atomic>
#include <thread>
#include "Analyze.h"
#include "TwoGtp.h"
#include "libboardgame_base/Log.h"
#include "libboardgame_base/Options.h"
#include "libpentobi_base/Variant.h"

using namespace std;
using libboardgame_base::Options;
using libpentobi_base::Variant;

//-----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    libboardgame_base::LogInitializer log_initializer;
    atomic<int> result(0);
    try
    {
        vector<string> specs = {
            "analyze:",
            "black|b:",
            "blue:",
            "fastopen",
            "file|f:",
            "game|g:",
            "green:",
            "nugames|n:",
            "quiet",
            "red:",
            "saveinterval:",
            "threads:",
            "tree",
            "white|w:",
            "yellow:",
        };
        Options opt(argc, argv, specs);
        if (opt.contains("analyze"))
        {
            analyze(opt.get("analyze"));
            return 0;
        }
        auto black = opt.get("black", "");
        auto blue = opt.get("blue", "");
        auto white = opt.get("white", "");
        auto yellow = opt.get("yellow", "");
        auto red = opt.get("red", "");
        auto green = opt.get("green", "");
        auto prefix = opt.get("file", "output");
        auto nu_games = opt.get<unsigned>("nugames", 1);
        auto nu_threads = opt.get<unsigned>("threads", 1);
        auto variant_string = opt.get("game", "classic");
        auto save_interval = opt.get<double>("saveinterval", 60);
        bool quiet = opt.contains("quiet");
        if (quiet)
            libboardgame_base::disable_logging();
        bool fast_open = opt.contains("fastopen");
        bool create_tree = opt.contains("tree") || fast_open;
        Variant variant;
        if (! parse_variant_id(variant_string, variant))
            throw runtime_error("invalid game variant " + variant_string);
        Output output(variant, prefix, create_tree);
        vector<shared_ptr<TwoGtp>> twogtps;
        twogtps.reserve(nu_threads);
        vector<string> binaries;
        if (black.length() && white.length()) {
          binaries = { black, white };
        } else if (blue.length() && yellow.length() && red.length() && green.length()) {
          binaries = { blue, yellow, red, green };
        } else {
          throw runtime_error("must supply black and white or blue, yellow, red and green.");
        }
        for (unsigned i = 0; i < nu_threads; ++i)
        {
            string log_prefix;
            if (nu_threads > 1)
                log_prefix = to_string(i + 1);
            auto twogtp = make_shared<TwoGtp>(binaries, variant,
                                              nu_games, output, quiet,
                                              log_prefix, fast_open);
            twogtp->set_save_interval(save_interval);
            twogtps.push_back(twogtp);
        }
        vector<thread> threads;
        threads.reserve(nu_threads);
        for (auto& i : twogtps)
            threads.emplace_back([&i, &result]()
            {
                try
                {
                    i->run();
                }
                catch (const exception& e)
                {
                    LIBBOARDGAME_LOG("Error: ", e.what());
                    result = 1;
                }
            });
        for (auto& t : threads)
            t.join();
    }
    catch (const exception& e)
    {
        LIBBOARDGAME_LOG("Error: ", e.what());
        result = 1;
    }
    return result;
}

//-----------------------------------------------------------------------------
