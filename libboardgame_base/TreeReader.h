//-----------------------------------------------------------------------------
/** @file libboardgame_base/TreeReader.h
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

#ifndef LIBBOARDGAME_BASE_TREE_READER_H
#define LIBBOARDGAME_BASE_TREE_READER_H

#include <memory>
#include <stack>
#include "Reader.h"
#include "SgfNode.h"

namespace libboardgame_base {

//-----------------------------------------------------------------------------

class TreeReader
    : public Reader
{
public:
    TreeReader();

    ~TreeReader() override;

    void on_begin_tree(bool is_root) override;

    void on_end_tree(bool is_root) override;

    void on_begin_node(bool is_root) override;

    void on_end_node() override;

    void on_property(const string& identifier,
                     const vector<string>& values) override;

    const SgfNode& get_tree() const { return *m_root; }

    /** Get the tree and transfer the ownership to the caller. */
    unique_ptr<SgfNode> move_tree();

private:
    SgfNode* m_current = nullptr;

    unique_ptr<SgfNode> m_root;

    stack<SgfNode*> m_stack;
};

//-----------------------------------------------------------------------------

} // namespace libboardgame_base

#endif // LIBBOARDGAME_BASE_TREE_READER_H
