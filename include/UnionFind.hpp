#ifndef CAV_INCLUDE_UTILS_UNIONFIND_HPP
#define CAV_INCLUDE_UTILS_UNIONFIND_HPP

#include <cassert>
#include <cstddef>
#include <vector>

namespace cav {

template <typename Int = size_t>
class UnionFind {
public:
    struct Node {
        Int size, parent;
    };

    explicit UnionFind(Int size)
        : components_num(size) {
        nodes.resize(size);
        for (Int i = 0; i < size; ++i)
            nodes[i] = {1, i};
    }

    inline Int make_set() {
        Int old_size = nodes.size();
        nodes.emplace_back(1, old_size);
        ++components_num;
        return old_size;
    }

    [[nodiscard]] Int find(Int n) {
        assert(static_cast<size_t>(n) < nodes.size());
        Int& p = nodes[n].parent;
        if (n != p)
            p = find(p);
        return p;
    }

    inline bool link_nodes(Int r1, Int r2) {
        assert(static_cast<size_t>(r1) < nodes.size());
        assert(static_cast<size_t>(r2) < nodes.size());
        if (r1 != r2) {
            if (nodes[r1].size >= nodes[r2].size) {
                nodes[r2].parent = r1;
                nodes[r1].size += nodes[r2].size;
            } else {
                nodes[r1].parent = r2;
                nodes[r2].size += nodes[r1].size;
            }
            assert(components_num > 1);
            --components_num;
            return false;
        }
        return true;
    }

    inline bool union_nodes(Int n1, Int n2) {
        assert(static_cast<size_t>(n1) < nodes.size());
        assert(static_cast<size_t>(n2) < nodes.size());
        return link_nodes(find(n1), find(n2));
    }

    [[nodiscard]] Int get_comp_size(Int n) {
        assert(static_cast<size_t>(n) < nodes.size());
        return nodes[find(n)].size;
    }

    [[nodiscard]] size_t get_components_num() const {
        return components_num;
    }

    [[nodiscard]] size_t size() const {
        return nodes.size();
    }

private:
    std::vector<Node> nodes;
    size_t            components_num = 0;
};
}  // namespace cav

#endif /* CAV_INCLUDE_UTILS_UNIONFIND_HPP */
