/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _BITSET_H_
#define _BITSET_H_

#include <atomic>
#include <vector>
#include <sstream>  // for to_string
#include <cassert>

namespace tasks {
namespace tools {

/// A thread safe lock free bitset.
class bitset {
  public:
    using data_type = std::atomic<bool>;
    using int_type = std::vector<data_type>::size_type;

    /// Constructor
    ///
    /// \param bits The number if bits the bitset should keep.
    bitset(int_type bits = 8)
        : m_bitset(std::vector<data_type>(bits)),
          m_bits(bits) {}

    /// \return The number of bits the bitset keeps.
    inline int_type bits() const { return m_bits; }

    /// \copydoc bits()
    inline size_t buckets() const { return m_bitset.size(); }

    /// Toggle a bit.
    inline void toggle(int_type p) {
        assert(p < m_bits);
        m_bitset[p] = !m_bitset[p];
    }

    /// Set a bit.
    inline void set(int_type p) {
        assert(p < m_bits);
        m_bitset[p] = true;
    }

    /// Unset a bit.
    inline void unset(int_type p) {
        assert(p < m_bits);
        m_bitset[p] = false;
    }

    /// Test if a bit is set.
    inline bool test(int_type p) const {
        assert(p < m_bits);
        return m_bitset[p];
    }

    /// \return True if at least one bit is set. The bit offset is set to the offset parameter.
    inline bool any(int_type& offset) const {
        offset = 0;
        for (auto& bit : m_bitset) {
            if (bit == true) {
                return true;
            }
            offset++;
        }
        return false;
    }

    // \copydoc any(int_type& offset)
    inline bool first(int_type& idx) const {
        return any(idx);
    }

    /// Find a bit that is set. The full bitset will be searched as worst case.
    ///
    /// \param idx The index of the first bit is stored to idx.
    /// \param start Uses start as starting point to search.
    /// \return True if a bit was found.
    inline bool next(int_type& idx, int_type start = 0) const {
        assert(start < m_bits);
        int_type chk_idx = start;
        int_type chk_cnt = 0;
        do {
            if (test(chk_idx)) {
                idx = chk_idx;
                return true;
            }
            chk_idx++;
            // overflow check
            if (chk_idx == m_bits) {
                chk_idx = 0;
            }
            chk_cnt++;
        } while (chk_cnt < m_bits);
        return false;
    }

    /// Return a string representation.
    std::string to_string() {
        std::stringstream out;
        for (int_type i = 0; i < m_bits; i++) {
            out << (test(i) ? "1" : "0");
            if (((i + 1) % 8) == 0) {
                out << " ";
            }
        }
        return out.str();
    }

  private:
    std::vector<data_type> m_bitset;
    int_type m_bits;
};

}  // tools
}  // tasks

#endif  // _BITSET_H_
