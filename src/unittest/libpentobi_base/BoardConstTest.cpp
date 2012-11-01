//-----------------------------------------------------------------------------
/** @file unittest/libpentobi_base/BoardConstTest.cpp */
//-----------------------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "libpentobi_base/BoardConst.h"

#include "libboardgame_test/Test.h"

using namespace std;
using namespace libpentobi_base;

//-----------------------------------------------------------------------------

/** Check symmetry information in MoveInfoExt for some moves. */
LIBBOARDGAME_TEST_CASE(pentobi_base_board_const_symmetry_info)
{
    const BoardConst& bc = BoardConst::get(variant_trigon_2);
    const MoveInfoExt& info_ext =
        bc.get_move_info_ext(bc.from_string("q9,q10,r10,q11,r11,s11"));
    LIBBOARDGAME_CHECK(! info_ext.breaks_symmetry);
    LIBBOARDGAME_CHECK_EQUAL(info_ext.symmetric_move.to_int(),
                             bc.from_string("q8,r8,s8,r9,s9,s10").to_int());
}

//-----------------------------------------------------------------------------
