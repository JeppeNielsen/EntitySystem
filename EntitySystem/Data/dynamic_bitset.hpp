// -----------------------------------------------------------
//
//   Copyright (c) 2001-2002 Chuck Allison and Jeremy Siek
//        Copyright (c) 2003-2006, 2008 Gennaro Prota
//             Copyright (c) 2014 Ahmed Charles
//
// Copyright (c) 2014 Glen Joseph Fernandes
// glenfe at live dot com
// Copyright (c) 2014 Riccardo Marcangelo
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// -----------------------------------------------------------

#ifndef BOOST_DYNAMIC_BITSET_DYNAMIC_BITSET_HPP
#define BOOST_DYNAMIC_BITSET_DYNAMIC_BITSET_HPP

#include <assert.h>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <climits>      // for CHAR_BIT

#include <istream>
#include <ostream>
#include <memory>
#include "dynamic_bitset_impl.hpp"




namespace boost {
 namespace detail {

  template <typename T>
  int integer_log2_impl(T x, int n) {

      int result = 0;

      while (x != 1) {

          const T t = static_cast<T>(x >> n);
          if (t) {
              result += n;
              x = t;
          }
          n /= 2;

      }

      return result;
  }



  // helper to find the maximum power of two
  // less than p (more involved than necessary,
  // to avoid PTS)
  //
  template <int p, int n>
  struct max_pow2_less {

      enum { c = 2*n < p };

      const static int value =
          c ? (max_pow2_less< c*p, 2*c*n>::value) : n;

  };

  template <>
  struct max_pow2_less<0, 0> {

      const static int value = 0;
  };

  // this template is here just for Borland :(
  // we could simply rely on numeric_limits but sometimes
  // Borland tries to use numeric_limits<const T>, because
  // of its usual const-related problems in argument deduction
  // - gps
  template <typename T>
  struct width {

      static const int value = (std::numeric_limits<T>::digits);
  };

 } // detail


 // ---------
 // integer_log2
 // ---------------
 //
 template <typename T>
 int integer_log2(T x) {

     assert(x > 0);

     const int n = detail::max_pow2_less<
                     detail::width<T> :: value, 4
                   > :: value;

     return detail::integer_log2_impl(x, n);

 }



}


namespace boost {

template <typename Block = unsigned long,
          typename Allocator = std::allocator<Block> >
class dynamic_bitset;

}

namespace boost {

template <typename Block, typename Allocator>
class dynamic_bitset
{
    // Portability note: member function templates are defined inside
    // this class definition to avoid problems with VC++. Similarly,
    // with the member functions of nested classes.
    //
    // [October 2008: the note above is mostly historical; new versions
    // of VC++ are likely able to digest a more drinking form of the
    // code; but changing it now is probably not worth the risks...]

    //static_assert<(bool)detail::dynamic_bitset_impl::allowed_block_type<Block>::value>;
    typedef std::vector<Block, Allocator> buffer_type;

public:
    typedef Block block_type;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;
    typedef typename buffer_type::size_type block_width_type;

    static const block_width_type bits_per_block = (std::numeric_limits<Block>::digits);
    static const size_type npos = static_cast<size_type>(-1);


public:

    // A proxy class to simulate lvalues of bit type.
    //
    class reference
    {
        friend class dynamic_bitset<Block, Allocator>;


        // the one and only non-copy ctor
        reference(block_type & b, block_type pos)
            :m_block(b),
             m_mask( (assert(pos < bits_per_block),
                      block_type(1) << pos )
                   )
        { }

        void operator&(); // left undefined

    public:

        // copy constructor: compiler generated

        operator bool() const { return (m_block & m_mask) != 0; }
        bool operator~() const { return (m_block & m_mask) == 0; }

        reference& flip() { do_flip(); return *this; }

        reference& operator=(bool x)               { do_assign(x);   return *this; } // for b[i] = x
        reference& operator=(const reference& rhs) { do_assign(rhs); return *this; } // for b[i] = b[j]

        reference& operator|=(bool x) { if  (x) do_set();   return *this; }
        reference& operator&=(bool x) { if (!x) do_reset(); return *this; }
        reference& operator^=(bool x) { if  (x) do_flip();  return *this; }
        reference& operator-=(bool x) { if  (x) do_reset(); return *this; }

     private:
        block_type & m_block;
        const block_type m_mask;

        void do_set() { m_block |= m_mask; }
        void do_reset() { m_block &= ~m_mask; }
        void do_flip() { m_block ^= m_mask; }
        void do_assign(bool x) { x? do_set() : do_reset(); }
    };

    typedef bool const_reference;

    // constructors, etc.
    explicit
    dynamic_bitset(const Allocator& alloc = Allocator());

    explicit
    dynamic_bitset(size_type num_bits, unsigned long value = 0,
               const Allocator& alloc = Allocator());


    // WARNING: you should avoid using this constructor.
    //
    //  A conversion from string is, in most cases, formatting,
    //  and should be performed by using operator>>.
    //
    // NOTE:
    //  Leave the parentheses around std::basic_string<CharT, Traits, Alloc>::npos.
    //  g++ 3.2 requires them and probably the standard will - see core issue 325
    // NOTE 2:
    //  split into two constructors because of bugs in MSVC 6.0sp5 with STLport

    template <typename CharT, typename Traits, typename Alloc>
    dynamic_bitset(const std::basic_string<CharT, Traits, Alloc>& s,
        typename std::basic_string<CharT, Traits, Alloc>::size_type pos,
        typename std::basic_string<CharT, Traits, Alloc>::size_type n,
        size_type num_bits = npos,
        const Allocator& alloc = Allocator())

    :m_bits(alloc),
     m_num_bits(0)
    {
      init_from_string(s, pos, n, num_bits);
    }

    template <typename CharT, typename Traits, typename Alloc>
    explicit
    dynamic_bitset(const std::basic_string<CharT, Traits, Alloc>& s,
      typename std::basic_string<CharT, Traits, Alloc>::size_type pos = 0)

    :m_bits(Allocator()),
     m_num_bits(0)
    {
      init_from_string(s, pos, (std::basic_string<CharT, Traits, Alloc>::npos),
                       npos);
    }


    template <typename T>
    void dispatch_init(T first, T last,
                       detail::dynamic_bitset_impl::value_to_type<false>)
    {
        init_from_block_range(first, last);
    }

    template <typename BlockIter>
    void init_from_block_range(BlockIter first, BlockIter last)
    {
        assert(m_bits.size() == 0);
        m_bits.insert(m_bits.end(), first, last);
        m_num_bits = m_bits.size() * bits_per_block;
    }

    // copy constructor
    dynamic_bitset(const dynamic_bitset& b);

    ~dynamic_bitset();

    void swap(dynamic_bitset& b);
    dynamic_bitset& operator=(const dynamic_bitset& b);

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    dynamic_bitset(dynamic_bitset&& src);
    dynamic_bitset& operator=(dynamic_bitset&& src);
#endif // BOOST_NO_CXX11_RVALUE_REFERENCES

    allocator_type get_allocator() const;

    // size changing operations
    void resize(size_type num_bits, bool value = false);
    void clear();
    void push_back(bool bit);
    void pop_back();
    void append(Block block);

    // bitset operations
    dynamic_bitset& operator&=(const dynamic_bitset& b);
    dynamic_bitset& operator|=(const dynamic_bitset& b);
    dynamic_bitset& operator^=(const dynamic_bitset& b);
    dynamic_bitset& operator-=(const dynamic_bitset& b);
    dynamic_bitset& operator<<=(size_type n);
    dynamic_bitset& operator>>=(size_type n);
    dynamic_bitset operator<<(size_type n) const;
    dynamic_bitset operator>>(size_type n) const;

    // basic bit operations
    dynamic_bitset& set(size_type n, bool val = true);
    dynamic_bitset& set();
    dynamic_bitset& reset(size_type n);
    dynamic_bitset& reset();
    dynamic_bitset& flip(size_type n);
    dynamic_bitset& flip();
    bool test(size_type n) const;
    bool test_set(size_type n, bool val = true);
    bool all() const;
    bool any() const;
    bool none() const;
    dynamic_bitset operator~() const;
    size_type count() const noexcept;

    // subscript
    reference operator[](size_type pos) {
        return reference(m_bits[block_index(pos)], bit_index(pos));
    }
    bool operator[](size_type pos) const { return test(pos); }

    unsigned long to_ulong() const;

    size_type size() const noexcept;
    size_type num_blocks() const noexcept;
    size_type max_size() const noexcept;
    bool empty() const noexcept;

    bool is_subset_of(const dynamic_bitset& a) const;
    bool is_proper_subset_of(const dynamic_bitset& a) const;
    bool intersects(const dynamic_bitset & a) const;

    // lookup
    size_type find_first() const;
    size_type find_next(size_type pos) const;


#if !defined BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS
    // lexicographical comparison
    template <typename B, typename A>
    friend bool operator==(const dynamic_bitset<B, A>& a,
                           const dynamic_bitset<B, A>& b);

    template <typename B, typename A>
    friend bool operator<(const dynamic_bitset<B, A>& a,
                          const dynamic_bitset<B, A>& b);


    template <typename B, typename A, typename BlockOutputIterator>
    friend void to_block_range(const dynamic_bitset<B, A>& b,
                               BlockOutputIterator result);

    template <typename BlockIterator, typename B, typename A>
    friend void from_block_range(BlockIterator first, BlockIterator last,
                                 dynamic_bitset<B, A>& result);


    template <typename CharT, typename Traits, typename B, typename A>
    friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is,
                                                         dynamic_bitset<B, A>& b);

    template <typename B, typename A, typename stringT>
    friend void to_string_helper(const dynamic_bitset<B, A> & b, stringT & s, bool dump_all);


#endif


private:
    const static block_width_type ulong_width = std::numeric_limits<unsigned long>::digits;

    void m_zero_unused_bits();
    bool m_check_invariants() const;

    size_type m_do_find_from(size_type first_block) const;

    block_width_type count_extra_bits() const noexcept { return bit_index(size()); }
    static size_type block_index(size_type pos) noexcept { return pos / bits_per_block; }
    static block_width_type bit_index(size_type pos) noexcept { return static_cast<block_width_type>(pos % bits_per_block); }
    static Block bit_mask(size_type pos) noexcept { return Block(1) << bit_index(pos); }

    void init_from_unsigned_long(size_type num_bits,
                                 unsigned long value/*,
                                 const Allocator& alloc*/)
    {

        assert(m_bits.size() == 0);

        m_bits.resize(calc_num_blocks(num_bits));
        m_num_bits = num_bits;

        typedef unsigned long num_type;
        typedef boost::detail::dynamic_bitset_impl
            ::shifter<num_type, bits_per_block, ulong_width> shifter;

        //if (num_bits == 0)
        //    return;

        // zero out all bits at pos >= num_bits, if any;
        // note that: num_bits == 0 implies value == 0
        if (num_bits < static_cast<size_type>(ulong_width)) {
            const num_type mask = (num_type(1) << num_bits) - 1;
            value &= mask;
        }

        typename buffer_type::iterator it = m_bits.begin();
        for( ; value; shifter::left_shift(value), ++it) {
            *it = static_cast<block_type>(value);
        }

    }



private:

    bool m_unchecked_test(size_type pos) const;
    static size_type calc_num_blocks(size_type num_bits);

    Block&        m_highest_block();
    const Block&  m_highest_block() const;

    buffer_type m_bits;
    size_type   m_num_bits;


    class bit_appender;
    friend class bit_appender;
    class bit_appender {
      // helper for stream >>
      // Supplies to the lack of an efficient append at the less
      // significant end: bits are actually appended "at left" but
      // rearranged in the destructor. From the perspective of
      // client code everything works *as if* dynamic_bitset<> had
      // an append_at_right() function (eventually throwing the same
      // exceptions as push_back) except that the function is in fact
      // called bit_appender::do_append().
      //
      dynamic_bitset & bs;
      size_type n;
      Block mask;
      Block * current;

      // not implemented
      bit_appender(const bit_appender &);
      bit_appender & operator=(const bit_appender &);

    public:
        bit_appender(dynamic_bitset & r) : bs(r), n(0), mask(0), current(0) {}
        ~bit_appender() {
            // reverse the order of blocks, shift
            // if needed, and then resize
            //
            std::reverse(bs.m_bits.begin(), bs.m_bits.end());
            const block_width_type offs = bit_index(n);
            if (offs)
                bs >>= (bits_per_block - offs);
            bs.resize(n); // doesn't enlarge, so can't throw
            assert(bs.m_check_invariants());
        }
        inline void do_append(bool value) {

            if (mask == 0) {
                bs.append(Block(0));
                current = &bs.m_highest_block();
                mask = Block(1) << (bits_per_block - 1);
            }

            if(value)
                *current |= mask;

            mask /= 2;
            ++n;
        }
        size_type get_count() const { return n; }
    };

};

#if !defined BOOST_NO_INCLASS_MEMBER_INITIALIZATION

template <typename Block, typename Allocator>
const typename dynamic_bitset<Block, Allocator>::block_width_type
dynamic_bitset<Block, Allocator>::bits_per_block;

template <typename Block, typename Allocator>
const typename dynamic_bitset<Block, Allocator>::size_type
dynamic_bitset<Block, Allocator>::npos;

template <typename Block, typename Allocator>
const typename dynamic_bitset<Block, Allocator>::block_width_type
dynamic_bitset<Block, Allocator>::ulong_width;

#endif

// Global Functions:

// comparison
template <typename Block, typename Allocator>
bool operator!=(const dynamic_bitset<Block, Allocator>& a,
                const dynamic_bitset<Block, Allocator>& b);

template <typename Block, typename Allocator>
bool operator<=(const dynamic_bitset<Block, Allocator>& a,
                const dynamic_bitset<Block, Allocator>& b);

template <typename Block, typename Allocator>
bool operator>(const dynamic_bitset<Block, Allocator>& a,
               const dynamic_bitset<Block, Allocator>& b);

template <typename Block, typename Allocator>
bool operator>=(const dynamic_bitset<Block, Allocator>& a,
                const dynamic_bitset<Block, Allocator>& b);

// stream operators
#ifdef BOOST_OLD_IOSTREAMS
template <typename Block, typename Allocator>
std::ostream& operator<<(std::ostream& os,
                         const dynamic_bitset<Block, Allocator>& b);

template <typename Block, typename Allocator>
std::istream& operator>>(std::istream& is, dynamic_bitset<Block,Allocator>& b);
#else
template <typename CharT, typename Traits, typename Block, typename Allocator>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os,
           const dynamic_bitset<Block, Allocator>& b);

template <typename CharT, typename Traits, typename Block, typename Allocator>
std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits>& is,
           dynamic_bitset<Block, Allocator>& b);
#endif

// bitset operations
template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>
operator&(const dynamic_bitset<Block, Allocator>& b1,
          const dynamic_bitset<Block, Allocator>& b2);

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>
operator|(const dynamic_bitset<Block, Allocator>& b1,
          const dynamic_bitset<Block, Allocator>& b2);

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>
operator^(const dynamic_bitset<Block, Allocator>& b1,
          const dynamic_bitset<Block, Allocator>& b2);

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>
operator-(const dynamic_bitset<Block, Allocator>& b1,
          const dynamic_bitset<Block, Allocator>& b2);

// namespace scope swap
template<typename Block, typename Allocator>
void swap(dynamic_bitset<Block, Allocator>& b1,
          dynamic_bitset<Block, Allocator>& b2);


template <typename Block, typename Allocator, typename stringT>
void
to_string(const dynamic_bitset<Block, Allocator>& b, stringT & s);

template <typename Block, typename Allocator, typename BlockOutputIterator>
void
to_block_range(const dynamic_bitset<Block, Allocator>& b,
               BlockOutputIterator result);


template <typename BlockIterator, typename B, typename A>
inline void
from_block_range(BlockIterator first, BlockIterator last,
                 dynamic_bitset<B, A>& result)
{
    // PRE: distance(first, last) <= numblocks()
    std::copy (first, last, result.m_bits.begin());
}

//=============================================================================
// dynamic_bitset implementation


//-----------------------------------------------------------------------------
// constructors, etc.

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>::dynamic_bitset(const Allocator& alloc)
  : m_bits(alloc), m_num_bits(0)
{

}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>::
dynamic_bitset(size_type num_bits, unsigned long value, const Allocator& alloc)
    : m_bits(alloc),
      m_num_bits(0)
{
    init_from_unsigned_long(num_bits, value);
}

// copy constructor
template <typename Block, typename Allocator>
inline dynamic_bitset<Block, Allocator>::
dynamic_bitset(const dynamic_bitset& b)
  : m_bits(b.m_bits), m_num_bits(b.m_num_bits)
{

}

template <typename Block, typename Allocator>
inline dynamic_bitset<Block, Allocator>::
~dynamic_bitset()
{
    assert(m_check_invariants());
}

template <typename Block, typename Allocator>
inline void dynamic_bitset<Block, Allocator>::
swap(dynamic_bitset<Block, Allocator>& b) // no throw
{
    std::swap(m_bits, b.m_bits);
    std::swap(m_num_bits, b.m_num_bits);
}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>& dynamic_bitset<Block, Allocator>::
operator=(const dynamic_bitset<Block, Allocator>& b)
{
    m_bits = b.m_bits;
    m_num_bits = b.m_num_bits;
    return *this;
}

template <typename Block, typename Allocator>
inline dynamic_bitset<Block, Allocator>::
dynamic_bitset(dynamic_bitset<Block, Allocator>&& b)
  : m_bits(std::move(b.m_bits)), m_num_bits(std::move(b.m_num_bits))
{
    // Required so that assert(m_check_invariants()); works.
    assert((b.m_bits = buffer_type()).empty());
    b.m_num_bits = 0;
}

template <typename Block, typename Allocator>
inline dynamic_bitset<Block, Allocator>& dynamic_bitset<Block, Allocator>::
operator=(dynamic_bitset<Block, Allocator>&& b)
{
    //if (std::addressof(b) == this) { return *this; }

    m_bits = std::move(b.m_bits);
    m_num_bits = std::move(b.m_num_bits);
    // Required so that assert(m_check_invariants()); works.
    assert((b.m_bits = buffer_type()).empty());
    b.m_num_bits = 0;
    return *this;
}

template <typename Block, typename Allocator>
inline typename dynamic_bitset<Block, Allocator>::allocator_type
dynamic_bitset<Block, Allocator>::get_allocator() const
{
    return m_bits.get_allocator();
}

//-----------------------------------------------------------------------------
// size changing operations

template <typename Block, typename Allocator>
void dynamic_bitset<Block, Allocator>::
resize(size_type num_bits, bool value) // strong guarantee
{

  const size_type old_num_blocks = num_blocks();
  const size_type required_blocks = calc_num_blocks(num_bits);

  const block_type v = value? ~Block(0) : Block(0);

  if (required_blocks != old_num_blocks) {
    m_bits.resize(required_blocks, v); // s.g. (copy)
  }


  // At this point:
  //
  //  - if the buffer was shrunk, we have nothing more to do,
  //    except a call to m_zero_unused_bits()
  //
  //  - if it was enlarged, all the (used) bits in the new blocks have
  //    the correct value, but we have not yet touched those bits, if
  //    any, that were 'unused bits' before enlarging: if value == true,
  //    they must be set.

  if (value && (num_bits > m_num_bits)) {

    const block_width_type extra_bits = count_extra_bits();
    if (extra_bits) {
        assert(old_num_blocks >= 1 && old_num_blocks <= m_bits.size());

        // Set them.
        m_bits[old_num_blocks - 1] |= (v << extra_bits);
    }

  }

  m_num_bits = num_bits;
  m_zero_unused_bits();

}

template <typename Block, typename Allocator>
void dynamic_bitset<Block, Allocator>::
clear() // no throw
{
  m_bits.clear();
  m_num_bits = 0;
}


template <typename Block, typename Allocator>
void dynamic_bitset<Block, Allocator>::
push_back(bool bit)
{
  const size_type sz = size();
  resize(sz + 1);
  set(sz, bit);
}

template <typename Block, typename Allocator>
void dynamic_bitset<Block, Allocator>::
pop_back() 
{
  const size_type old_num_blocks = num_blocks();
  const size_type required_blocks = calc_num_blocks(m_num_bits - 1);
  
  if (required_blocks != old_num_blocks) {
    m_bits.pop_back(); 
  }
    
  --m_num_bits;
  m_zero_unused_bits();
}


template <typename Block, typename Allocator>
void dynamic_bitset<Block, Allocator>::
append(Block value) // strong guarantee
{
    const block_width_type r = count_extra_bits();

    if (r == 0) {
        // the buffer is empty, or all blocks are filled
        m_bits.push_back(value);
    }
    else {
        m_bits.push_back(value >> (bits_per_block - r));
        m_bits[m_bits.size() - 2] |= (value << r); // m_bits.size() >= 2
    }

    m_num_bits += bits_per_block;
    assert(m_check_invariants());

}


//-----------------------------------------------------------------------------
// bitset operations
template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>&
dynamic_bitset<Block, Allocator>::operator&=(const dynamic_bitset& rhs)
{
    assert(size() == rhs.size());
    for (size_type i = 0; i < num_blocks(); ++i)
        m_bits[i] &= rhs.m_bits[i];
    return *this;
}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>&
dynamic_bitset<Block, Allocator>::operator|=(const dynamic_bitset& rhs)
{
    assert(size() == rhs.size());
    for (size_type i = 0; i < num_blocks(); ++i)
        m_bits[i] |= rhs.m_bits[i];
    //m_zero_unused_bits();
    return *this;
}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>&
dynamic_bitset<Block, Allocator>::operator^=(const dynamic_bitset& rhs)
{
    assert(size() == rhs.size());
    for (size_type i = 0; i < this->num_blocks(); ++i)
        m_bits[i] ^= rhs.m_bits[i];
    //m_zero_unused_bits();
    return *this;
}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>&
dynamic_bitset<Block, Allocator>::operator-=(const dynamic_bitset& rhs)
{
    assert(size() == rhs.size());
    for (size_type i = 0; i < num_blocks(); ++i)
        m_bits[i] &= ~rhs.m_bits[i];
    //m_zero_unused_bits();
    return *this;
}

//
// NOTE:
//  Note that the 'if (r != 0)' is crucial to avoid undefined
//  behavior when the left hand operand of >> isn't promoted to a
//  wider type (because rs would be too large).
//
template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>&
dynamic_bitset<Block, Allocator>::operator<<=(size_type n)
{
    if (n >= m_num_bits)
        return reset();
    //else
    if (n > 0) {

        size_type    const last = num_blocks() - 1;  // num_blocks() is >= 1
        size_type    const div  = n / bits_per_block; // div is <= last
        block_width_type const r = bit_index(n);
        block_type * const b    = &m_bits[0];

        if (r != 0) {

            block_width_type const rs = bits_per_block - r;

            for (size_type i = last-div; i>0; --i) {
                b[i+div] = (b[i] << r) | (b[i-1] >> rs);
            }
            b[div] = b[0] << r;

        }
        else {
            for (size_type i = last-div; i>0; --i) {
                b[i+div] = b[i];
            }
            b[div] = b[0];
        }

        // zero out div blocks at the less significant end
        std::fill_n(m_bits.begin(), div, static_cast<block_type>(0));

        // zero out any 1 bit that flowed into the unused part
        m_zero_unused_bits(); // thanks to Lester Gong

    }

    return *this;


}


//
// NOTE:
//  see the comments to operator <<=
//
template <typename B, typename A>
dynamic_bitset<B, A> & dynamic_bitset<B, A>::operator>>=(size_type n) {
    if (n >= m_num_bits) {
        return reset();
    }
    //else
    if (n>0) {

        size_type  const last  = num_blocks() - 1; // num_blocks() is >= 1
        size_type  const div   = n / bits_per_block;   // div is <= last
        block_width_type const r     = bit_index(n);
        block_type * const b   = &m_bits[0];


        if (r != 0) {

            block_width_type const ls = bits_per_block - r;

            for (size_type i = div; i < last; ++i) {
                b[i-div] = (b[i] >> r) | (b[i+1]  << ls);
            }
            // r bits go to zero
            b[last-div] = b[last] >> r;
        }

        else {
            for (size_type i = div; i <= last; ++i) {
                b[i-div] = b[i];
            }
            // note the '<=': the last iteration 'absorbs'
            // b[last-div] = b[last] >> 0;
        }



        // div blocks are zero filled at the most significant end
        std::fill_n(m_bits.begin() + (num_blocks()-div), div, static_cast<block_type>(0));
    }

    return *this;
}


template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>
dynamic_bitset<Block, Allocator>::operator<<(size_type n) const
{
    dynamic_bitset r(*this);
    return r <<= n;
}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>
dynamic_bitset<Block, Allocator>::operator>>(size_type n) const
{
    dynamic_bitset r(*this);
    return r >>= n;
}


//-----------------------------------------------------------------------------
// basic bit operations

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>&
dynamic_bitset<Block, Allocator>::set(size_type pos, bool val)
{
    assert(pos < m_num_bits);

    if (val)
        m_bits[block_index(pos)] |= bit_mask(pos);
    else
        reset(pos);

    return *this;
}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>&
dynamic_bitset<Block, Allocator>::set()
{
  std::fill(m_bits.begin(), m_bits.end(), ~Block(0));
  m_zero_unused_bits();
  return *this;
}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>&
dynamic_bitset<Block, Allocator>::reset(size_type pos)
{
    assert(pos < m_num_bits);
    m_bits[block_index(pos)] &= ~bit_mask(pos);
    return *this;
}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>&
dynamic_bitset<Block, Allocator>::reset()
{
  std::fill(m_bits.begin(), m_bits.end(), Block(0));
  return *this;
}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>&
dynamic_bitset<Block, Allocator>::flip(size_type pos)
{
    assert(pos < m_num_bits);
    m_bits[block_index(pos)] ^= bit_mask(pos);
    return *this;
}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>&
dynamic_bitset<Block, Allocator>::flip()
{
    for (size_type i = 0; i < num_blocks(); ++i)
        m_bits[i] = ~m_bits[i];
    m_zero_unused_bits();
    return *this;
}

template <typename Block, typename Allocator>
bool dynamic_bitset<Block, Allocator>::m_unchecked_test(size_type pos) const
{
    return (m_bits[block_index(pos)] & bit_mask(pos)) != 0;
}

template <typename Block, typename Allocator>
bool dynamic_bitset<Block, Allocator>::test(size_type pos) const
{
    assert(pos < m_num_bits);
    return m_unchecked_test(pos);
}

template <typename Block, typename Allocator>
bool dynamic_bitset<Block, Allocator>::test_set(size_type pos, bool val)
{
    bool const b = test(pos);
    if (b != val) {
        set(pos, val);
    }
    return b;
}

template <typename Block, typename Allocator>
bool dynamic_bitset<Block, Allocator>::all() const
{
    if (empty()) {
        return true;
    }

    const block_width_type extra_bits = count_extra_bits();
    block_type const all_ones = ~static_cast<Block>(0);

    if (extra_bits == 0) {
        for (size_type i = 0, e = num_blocks(); i < e; ++i) {
            if (m_bits[i] != all_ones) {
                return false;
            }
        }
    } else {
        for (size_type i = 0, e = num_blocks() - 1; i < e; ++i) {
            if (m_bits[i] != all_ones) {
                return false;
            }
        }
        block_type const mask = ~(~static_cast<Block>(0) << extra_bits);
        if (m_highest_block() != mask) {
            return false;
        }
    }
    return true;
}

template <typename Block, typename Allocator>
bool dynamic_bitset<Block, Allocator>::any() const
{
    for (size_type i = 0; i < num_blocks(); ++i)
        if (m_bits[i])
            return true;
    return false;
}

template <typename Block, typename Allocator>
inline bool dynamic_bitset<Block, Allocator>::none() const
{
    return !any();
}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>
dynamic_bitset<Block, Allocator>::operator~() const
{
    dynamic_bitset b(*this);
    b.flip();
    return b;
}

template <typename Block, typename Allocator>
typename dynamic_bitset<Block, Allocator>::size_type
dynamic_bitset<Block, Allocator>::count() const noexcept
{
    using detail::dynamic_bitset_impl::table_width;
    using detail::dynamic_bitset_impl::access_by_bytes;
    using detail::dynamic_bitset_impl::access_by_blocks;
    using detail::dynamic_bitset_impl::value_to_type;

    // NOTE: Explicitly qualifying "bits_per_block" to workaround
    //       regressions of gcc 3.4.x
    enum { no_padding =
        dynamic_bitset<Block, Allocator>::bits_per_block
        == CHAR_BIT * sizeof(Block) };

    enum { enough_table_width = table_width >= CHAR_BIT };

    enum { mode = (no_padding && enough_table_width)
                          ? access_by_bytes
                          : access_by_blocks };

    return do_count(m_bits.begin(), num_blocks(), Block(0),
                    static_cast<value_to_type<(bool)mode> *>(0));
}


//-----------------------------------------------------------------------------
// conversions


// A comment similar to the one about the constructor from
// basic_string can be done here. Thanks to James Kanze for
// making me (Gennaro) realize this important separation of
// concerns issue, as well as many things about i18n.
//
template <typename Block, typename Allocator, typename stringT>
inline void
to_string(const dynamic_bitset<Block, Allocator>& b, stringT& s)
{
    to_string_helper(b, s, false);
}


// Differently from to_string this function dumps out
// every bit of the internal representation (may be
// useful for debugging purposes)
//
template <typename B, typename A, typename stringT>
inline void
dump_to_string(const dynamic_bitset<B, A>& b, stringT& s)
{
    to_string_helper(b, s, true /* =dump_all*/);
}

template <typename Block, typename Allocator, typename BlockOutputIterator>
inline void
to_block_range(const dynamic_bitset<Block, Allocator>& b,
               BlockOutputIterator result)
{
    // note how this copies *all* bits, including the
    // unused ones in the last block (which are zero)
    std::copy(b.m_bits.begin(), b.m_bits.end(), result);
}

template <typename Block, typename Allocator>
unsigned long dynamic_bitset<Block, Allocator>::
to_ulong() const
{

  if (m_num_bits == 0)
      return 0; // convention

  // Check for overflows. This may be a performance burden on very
  // large bitsets but is required by the specification, sorry
  if (find_next(ulong_width - 1) != npos)
    throw (std::overflow_error("boost::dynamic_bitset::to_ulong overflow"));


  // Ok, from now on we can be sure there's no "on" bit
  // beyond the "allowed" positions
  typedef unsigned long result_type;

  const size_type maximum_size =
            (std::min)(m_num_bits, static_cast<size_type>(ulong_width));

  const size_type last_block = block_index( maximum_size - 1 );

  assert((last_block * bits_per_block) < static_cast<size_type>(ulong_width));

  result_type result = 0;
  for (size_type i = 0; i <= last_block; ++i) {
    const size_type offset = i * bits_per_block;
    result |= (static_cast<result_type>(m_bits[i]) << offset);
  }

  return result;
}

template <typename Block, typename Allocator>
inline typename dynamic_bitset<Block, Allocator>::size_type
dynamic_bitset<Block, Allocator>::size() const noexcept
{
    return m_num_bits;
}

template <typename Block, typename Allocator>
inline typename dynamic_bitset<Block, Allocator>::size_type
dynamic_bitset<Block, Allocator>::num_blocks() const noexcept
{
    return m_bits.size();
}

template <typename Block, typename Allocator>
inline typename dynamic_bitset<Block, Allocator>::size_type
dynamic_bitset<Block, Allocator>::max_size() const noexcept
{
    // Semantics of vector<>::max_size() aren't very clear
    // (see lib issue 197) and many library implementations
    // simply return dummy values, _unrelated_ to the underlying
    // allocator.
    //
    // Given these problems, I was tempted to not provide this
    // function at all but the user could need it if he provides
    // his own allocator.
    //

    const size_type m = detail::dynamic_bitset_impl::
                        vector_max_size_workaround(m_bits);

    return m <= (size_type(-1)/bits_per_block) ?
        m * bits_per_block :
        size_type(-1);
}

template <typename Block, typename Allocator>
inline bool dynamic_bitset<Block, Allocator>::empty() const noexcept
{
  return size() == 0;
}

template <typename Block, typename Allocator>
bool dynamic_bitset<Block, Allocator>::
is_subset_of(const dynamic_bitset<Block, Allocator>& a) const
{
    assert(size() == a.size());
    for (size_type i = 0; i < num_blocks(); ++i)
        if (m_bits[i] & ~a.m_bits[i])
            return false;
    return true;
}

template <typename Block, typename Allocator>
bool dynamic_bitset<Block, Allocator>::
is_proper_subset_of(const dynamic_bitset<Block, Allocator>& a) const
{
    assert(size() == a.size());
    assert(num_blocks() == a.num_blocks());

    bool proper = false;
    for (size_type i = 0; i < num_blocks(); ++i) {
        const Block & bt =   m_bits[i];
        const Block & ba = a.m_bits[i];

        if (bt & ~ba)
            return false; // not a subset at all
        if (ba & ~bt)
            proper = true;
    }
    return proper;
}

template <typename Block, typename Allocator>
bool dynamic_bitset<Block, Allocator>::intersects(const dynamic_bitset & b) const
{
    size_type common_blocks = num_blocks() < b.num_blocks()
                              ? num_blocks() : b.num_blocks();

    for(size_type i = 0; i < common_blocks; ++i) {
        if(m_bits[i] & b.m_bits[i])
            return true;
    }
    return false;
}

// --------------------------------
// lookup


// look for the first bit "on", starting
// from the block with index first_block
//
template <typename Block, typename Allocator>
typename dynamic_bitset<Block, Allocator>::size_type
dynamic_bitset<Block, Allocator>::m_do_find_from(size_type first_block) const
{
    size_type i = first_block;

    // skip null blocks
    while (i < num_blocks() && m_bits[i] == 0)
        ++i;

    if (i >= num_blocks())
        return npos; // not found

    return i * bits_per_block + static_cast<size_type>(lowest_bit(m_bits[i]));

}


template <typename Block, typename Allocator>
typename dynamic_bitset<Block, Allocator>::size_type
dynamic_bitset<Block, Allocator>::find_first() const
{
    return m_do_find_from(0);
}


template <typename Block, typename Allocator>
typename dynamic_bitset<Block, Allocator>::size_type
dynamic_bitset<Block, Allocator>::find_next(size_type pos) const
{

    const size_type sz = size();
    if (pos >= (sz-1) || sz == 0)
        return npos;

    ++pos;

    const size_type blk = block_index(pos);
    const block_width_type ind = bit_index(pos);

    // shift bits upto one immediately after current
    const Block fore = m_bits[blk] >> ind;

    return fore?
        pos + static_cast<size_type>(lowest_bit(fore))
        :
        m_do_find_from(blk + 1);

}



//-----------------------------------------------------------------------------
// comparison

template <typename Block, typename Allocator>
bool operator==(const dynamic_bitset<Block, Allocator>& a,
                const dynamic_bitset<Block, Allocator>& b)
{
    return (a.m_num_bits == b.m_num_bits)
           && (a.m_bits == b.m_bits);
}

template <typename Block, typename Allocator>
inline bool operator!=(const dynamic_bitset<Block, Allocator>& a,
                       const dynamic_bitset<Block, Allocator>& b)
{
    return !(a == b);
}

template <typename Block, typename Allocator>
bool operator<(const dynamic_bitset<Block, Allocator>& a,
               const dynamic_bitset<Block, Allocator>& b)
{
    assert(a.size() == b.size());
    typedef typename dynamic_bitset<Block, Allocator>::size_type size_type;

    //if (a.size() == 0)
    //  return false;

    // Since we are storing the most significant bit
    // at pos == size() - 1, we need to do the comparisons in reverse.
    //
    for (size_type ii = a.num_blocks(); ii > 0; --ii) {
      size_type i = ii-1;
      if (a.m_bits[i] < b.m_bits[i])
        return true;
      else if (a.m_bits[i] > b.m_bits[i])
        return false;
    }
    return false;
}

template <typename Block, typename Allocator>
inline bool operator<=(const dynamic_bitset<Block, Allocator>& a,
                       const dynamic_bitset<Block, Allocator>& b)
{
    return !(a > b);
}

template <typename Block, typename Allocator>
inline bool operator>(const dynamic_bitset<Block, Allocator>& a,
                      const dynamic_bitset<Block, Allocator>& b)
{
    return b < a;
}

template <typename Block, typename Allocator>
inline bool operator>=(const dynamic_bitset<Block, Allocator>& a,
                       const dynamic_bitset<Block, Allocator>& b)
{
    return !(a < b);
}

//-----------------------------------------------------------------------------
// bitset operations

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>
operator&(const dynamic_bitset<Block, Allocator>& x,
          const dynamic_bitset<Block, Allocator>& y)
{
    dynamic_bitset<Block, Allocator> b(x);
    return b &= y;
}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>
operator|(const dynamic_bitset<Block, Allocator>& x,
          const dynamic_bitset<Block, Allocator>& y)
{
    dynamic_bitset<Block, Allocator> b(x);
    return b |= y;
}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>
operator^(const dynamic_bitset<Block, Allocator>& x,
          const dynamic_bitset<Block, Allocator>& y)
{
    dynamic_bitset<Block, Allocator> b(x);
    return b ^= y;
}

template <typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>
operator-(const dynamic_bitset<Block, Allocator>& x,
          const dynamic_bitset<Block, Allocator>& y)
{
    dynamic_bitset<Block, Allocator> b(x);
    return b -= y;
}

//-----------------------------------------------------------------------------
// namespace scope swap

template<typename Block, typename Allocator>
inline void
swap(dynamic_bitset<Block, Allocator>& left,
     dynamic_bitset<Block, Allocator>& right) // no throw
{
    left.swap(right);
}


//-----------------------------------------------------------------------------
// private (on conforming compilers) member functions


template <typename Block, typename Allocator>
inline typename dynamic_bitset<Block, Allocator>::size_type
dynamic_bitset<Block, Allocator>::calc_num_blocks(size_type num_bits)
{
    return num_bits / bits_per_block
           + static_cast<size_type>( num_bits % bits_per_block != 0 );
}

// gives a reference to the highest block
//
template <typename Block, typename Allocator>
inline Block& dynamic_bitset<Block, Allocator>::m_highest_block()
{
    return const_cast<Block &>
           (static_cast<const dynamic_bitset *>(this)->m_highest_block());
}

// gives a const-reference to the highest block
//
template <typename Block, typename Allocator>
inline const Block& dynamic_bitset<Block, Allocator>::m_highest_block() const
{
    assert(size() > 0 && num_blocks() > 0);
    return m_bits.back();
}


// If size() is not a multiple of bits_per_block
// then not all the bits in the last block are used.
// This function resets the unused bits (convenient
// for the implementation of many member functions)
//
template <typename Block, typename Allocator>
inline void dynamic_bitset<Block, Allocator>::m_zero_unused_bits()
{
    assert (num_blocks() == calc_num_blocks(m_num_bits));

    // if != 0 this is the number of bits used in the last block
    const block_width_type extra_bits = count_extra_bits();

    if (extra_bits != 0)
        m_highest_block() &= ~(~static_cast<Block>(0) << extra_bits);

}

// check class invariants
template <typename Block, typename Allocator>
bool dynamic_bitset<Block, Allocator>::m_check_invariants() const
{
    const block_width_type extra_bits = count_extra_bits();
    if (extra_bits > 0) {
        block_type const mask = (~static_cast<Block>(0) << extra_bits);
        if ((m_highest_block() & mask) != 0)
            return false;
    }
    if (m_bits.size() > m_bits.capacity() || num_blocks() != calc_num_blocks(size()))
        return false;

    return true;

}


} // namespace boost


#undef BOOST_BITSET_CHAR

#endif // include guard

