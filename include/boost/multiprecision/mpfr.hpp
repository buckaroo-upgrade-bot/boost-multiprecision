///////////////////////////////////////////////////////////////////////////////
//  Copyright 2011 John Maddock. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MATH_BN_MPFR_HPP
#define BOOST_MATH_BN_MPFR_HPP

#include <boost/multiprecision/number.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/multiprecision/detail/big_lanczos.hpp>
#include <boost/multiprecision/detail/digits.hpp>
#include <mpfr.h>
#include <cmath>
#include <algorithm>

namespace boost{ 
namespace multiprecision{
namespace backends{

template <unsigned digits10>
struct mpfr_float_backend;

} // namespace backends

template <unsigned digits10>
struct number_category<backends::mpfr_float_backend<digits10> > : public mpl::int_<number_kind_floating_point>{};

namespace backends{

namespace detail{

inline long get_default_precision() { return 50; }

template <unsigned digits10>
struct mpfr_float_imp
{
   typedef mpl::list<long, long long>                     signed_types;
   typedef mpl::list<unsigned long, unsigned long long>   unsigned_types;
   typedef mpl::list<double, long double>                 float_types;
   typedef long                                           exponent_type;

   mpfr_float_imp() BOOST_NOEXCEPT {}

   mpfr_float_imp(const mpfr_float_imp& o)
   {
      mpfr_init2(m_data, multiprecision::detail::digits10_2_2(digits10 ? digits10 : get_default_precision()));
      if(o.m_data[0]._mpfr_d)
         mpfr_set(m_data, o.m_data, GMP_RNDN);
   }
#ifndef BOOST_NO_RVALUE_REFERENCES
   mpfr_float_imp(mpfr_float_imp&& o) BOOST_NOEXCEPT
   {
      m_data[0] = o.m_data[0];
      o.m_data[0]._mpfr_d = 0;
   }
#endif
   mpfr_float_imp& operator = (const mpfr_float_imp& o) BOOST_NOEXCEPT
   {
      if(m_data[0]._mpfr_d == 0)
         mpfr_init2(m_data, multiprecision::detail::digits10_2_2(digits10 ? digits10 : get_default_precision()));
      if(o.m_data[0]._mpfr_d)
         mpfr_set(m_data, o.m_data, GMP_RNDN);
      return *this;
   }
#ifndef BOOST_NO_RVALUE_REFERENCES
   mpfr_float_imp& operator = (mpfr_float_imp&& o) BOOST_NOEXCEPT
   {
      mpfr_swap(m_data, o.m_data);
      return *this;
   }
#endif
#ifdef _MPFR_H_HAVE_INTMAX_T
   mpfr_float_imp& operator = (unsigned long long i) BOOST_NOEXCEPT
   {
      if(m_data[0]._mpfr_d == 0)
         mpfr_init2(m_data, multiprecision::detail::digits10_2_2(digits10 ? digits10 : get_default_precision()));
      mpfr_set_uj(m_data, i, GMP_RNDN);
      return *this;
   }
   mpfr_float_imp& operator = (long long i) BOOST_NOEXCEPT
   {
      if(m_data[0]._mpfr_d == 0)
         mpfr_init2(m_data, multiprecision::detail::digits10_2_2(digits10 ? digits10 : get_default_precision()));
      mpfr_set_sj(m_data, i, GMP_RNDN);
      return *this;
   }
#else
   mpfr_float_imp& operator = (unsigned long long i)
   {
      if(m_data[0]._mpfr_d == 0)
         mpfr_init2(m_data, multiprecision::detail::digits10_2_2(digits10 ? digits10 : get_default_precision()));
      unsigned long long mask = ((1uLL << std::numeric_limits<unsigned>::digits) - 1);
      unsigned shift = 0;
      mpfr_t t;
      mpfr_init2(t, (std::max)(static_cast<unsigned>(std::numeric_limits<unsigned long long>::digits), static_cast<unsigned>(multiprecision::detail::digits10_2_2(digits10))));
      mpfr_set_ui(m_data, 0, GMP_RNDN);
      while(i)
      {
         mpfr_set_ui(t, static_cast<unsigned>(i & mask), GMP_RNDN);
         if(shift)
            mpfr_mul_2exp(t, t, shift, GMP_RNDN);
         mpfr_add(m_data, m_data, t, GMP_RNDN);
         shift += std::numeric_limits<unsigned>::digits;
         i >>= std::numeric_limits<unsigned>::digits;
      }
      mpfr_clear(t);
      return *this;
   }
   mpfr_float_imp& operator = (long long i)
   {
      BOOST_MP_USING_ABS
      if(m_data[0]._mpfr_d == 0)
         mpfr_init2(m_data, multiprecision::detail::digits10_2_2(digits10 ? digits10 : get_default_precision()));
      bool neg = i < 0;
      *this = static_cast<unsigned long long>(abs(i));
      if(neg)
         mpfr_neg(m_data, m_data, GMP_RNDN);
      return *this;
   }
#endif
   mpfr_float_imp& operator = (unsigned long i) BOOST_NOEXCEPT
   {
      if(m_data[0]._mpfr_d == 0)
         mpfr_init2(m_data, multiprecision::detail::digits10_2_2(digits10 ? digits10 : get_default_precision()));
      mpfr_set_ui(m_data, i, GMP_RNDN);
      return *this;
   }
   mpfr_float_imp& operator = (long i) BOOST_NOEXCEPT
   {
      if(m_data[0]._mpfr_d == 0)
         mpfr_init2(m_data, multiprecision::detail::digits10_2_2(digits10 ? digits10 : get_default_precision()));
      mpfr_set_si(m_data, i, GMP_RNDN);
      return *this;
   }
   mpfr_float_imp& operator = (double d) BOOST_NOEXCEPT
   {
      if(m_data[0]._mpfr_d == 0)
         mpfr_init2(m_data, multiprecision::detail::digits10_2_2(digits10 ? digits10 : get_default_precision()));
      mpfr_set_d(m_data, d, GMP_RNDN);
      return *this;
   }
   mpfr_float_imp& operator = (long double a) BOOST_NOEXCEPT
   {
      if(m_data[0]._mpfr_d == 0)
         mpfr_init2(m_data, multiprecision::detail::digits10_2_2(digits10 ? digits10 : get_default_precision()));
      mpfr_set_ld(m_data, a, GMP_RNDN);
      return *this;
   }
   mpfr_float_imp& operator = (const char* s)
   {
      if(m_data[0]._mpfr_d == 0)
         mpfr_init2(m_data, multiprecision::detail::digits10_2_2(digits10 ? digits10 : get_default_precision()));
      if(mpfr_set_str(m_data, s, 10, GMP_RNDN) != 0)
      {
         BOOST_THROW_EXCEPTION(std::runtime_error(std::string("Unable to parse string \"") + s + std::string("\"as a valid floating point number.")));
      }
      return *this;
   }
   void swap(mpfr_float_imp& o) BOOST_NOEXCEPT
   {
      mpfr_swap(m_data, o.m_data);
   }
   std::string str(std::streamsize digits, std::ios_base::fmtflags f)const
   {
      BOOST_ASSERT(m_data[0]._mpfr_d);

      bool scientific = (f & std::ios_base::scientific) == std::ios_base::scientific;
      bool fixed      = (f & std::ios_base::fixed) == std::ios_base::fixed;

      std::streamsize org_digits(digits);

      if(scientific && digits)
         ++digits;

      std::string result;
      mp_exp_t e;
      if(mpfr_inf_p(m_data))
      {
         if(mpfr_sgn(m_data) < 0)
            result = "-inf";
         else if(f & std::ios_base::showpos)
            result = "+inf";
         else
            result = "inf";
         return result;
      }
      if(mpfr_nan_p(m_data))
      {
         result = "nan";
         return result;
      }
      if(mpfr_zero_p(m_data))
      {
         e = 0;
         result = "0";
      }
      else
      {
         char* ps = mpfr_get_str (0, &e, 10, static_cast<std::size_t>(digits), m_data, GMP_RNDN);
        --e;  // To match with what our formatter expects.
         if(fixed && e != -1)
         {
            // Oops we actually need a different number of digits to what we asked for:
            mpfr_free_str(ps);
            digits += e + 1;
            if(digits == 0)
            {
               // We need to get *all* the digits and then possibly round up,
               // we end up with either "0" or "1" as the result.
               ps = mpfr_get_str (0, &e, 10, 0, m_data, GMP_RNDN);
               --e;
               unsigned offset = *ps == '-' ? 1 : 0;
               if(ps[offset] > '5')
               {
                  ++e;
                  ps[offset] = '1';
                  ps[offset + 1] = 0;
               }
               else if(ps[offset] == '5')
               {
                  unsigned i = offset + 1;
                  bool round_up = false;
                  while(ps[i] != 0)
                  {
                     if(ps[i] != '0')
                     {
                        round_up = true;
                        break;
                     }
                  }
                  if(round_up)
                  {
                     ++e;
                     ps[offset] = '1';
                     ps[offset + 1] = 0;
                  }
                  else
                  {
                     ps[offset] = '0';
                     ps[offset + 1] = 0;
                  }
               }
               else
               {
                  ps[offset] = '0';
                  ps[offset + 1] = 0;
               }
            }
            else if(digits > 0)
            {
               ps = mpfr_get_str (0, &e, 10, static_cast<std::size_t>(digits), m_data, GMP_RNDN);
               --e;  // To match with what our formatter expects.
            }
            else
            {
               ps = mpfr_get_str (0, &e, 10, 1, m_data, GMP_RNDN);
               --e;
               unsigned offset = *ps == '-' ? 1 : 0;
               ps[offset] = '0';
               ps[offset + 1] = 0;
            }
         }
         result = ps ? ps : "0";
         if(ps)
            mpfr_free_str(ps);
      }
      boost::multiprecision::detail::format_float_string(result, e, org_digits, f, 0 != mpfr_zero_p(m_data));
      return result;
   }
   ~mpfr_float_imp() BOOST_NOEXCEPT
   {
      if(m_data[0]._mpfr_d)
         mpfr_clear(m_data);
   }
   void negate() BOOST_NOEXCEPT
   {
      BOOST_ASSERT(m_data[0]._mpfr_d);
      mpfr_neg(m_data, m_data, GMP_RNDN);
   }
   int compare(const mpfr_float_backend<digits10>& o)const BOOST_NOEXCEPT
   {
      BOOST_ASSERT(m_data[0]._mpfr_d && o.m_data[0]._mpfr_d);
      return mpfr_cmp(m_data, o.m_data);
   }
   int compare(long i)const BOOST_NOEXCEPT
   {
      BOOST_ASSERT(m_data[0]._mpfr_d);
      return mpfr_cmp_si(m_data, i);
   }
   int compare(unsigned long i)const BOOST_NOEXCEPT
   {
      BOOST_ASSERT(m_data[0]._mpfr_d);
      return mpfr_cmp_ui(m_data, i);
   }
   template <class V>
   int compare(V v)const BOOST_NOEXCEPT
   {
      mpfr_float_backend<digits10> d;
      d = v;
      return compare(d);
   }
   mpfr_t& data() BOOST_NOEXCEPT 
   { 
      BOOST_ASSERT(m_data[0]._mpfr_d);
      return m_data; 
   }
   const mpfr_t& data()const BOOST_NOEXCEPT 
   { 
      BOOST_ASSERT(m_data[0]._mpfr_d);
      return m_data; 
   }
protected:
   mpfr_t m_data;
   static unsigned& get_default_precision() BOOST_NOEXCEPT
   {
      static unsigned val = 50;
      return val;
   }
};

} // namespace detail

template <unsigned digits10>
struct mpfr_float_backend : public detail::mpfr_float_imp<digits10>
{
   mpfr_float_backend()
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(digits10));
   }
   mpfr_float_backend(const mpfr_float_backend& o) : detail::mpfr_float_imp<digits10>(o) {}
#ifndef BOOST_NO_RVALUE_REFERENCES
   mpfr_float_backend(mpfr_float_backend&& o) : detail::mpfr_float_imp<digits10>(static_cast<detail::mpfr_float_imp<digits10>&&>(o)) {}
#endif
   template <unsigned D>
   mpfr_float_backend(const mpfr_float_backend<D>& val, typename enable_if_c<D <= digits10>::type* = 0)
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(digits10));
      mpfr_set(this->m_data, val.data(), GMP_RNDN);
   }
   template <unsigned D>
   explicit mpfr_float_backend(const mpfr_float_backend<D>& val, typename disable_if_c<D <= digits10>::type* = 0)
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(digits10));
      mpfr_set(this->m_data, val.data(), GMP_RNDN);
   }
   template <unsigned D>
   mpfr_float_backend(const gmp_float<D>& val, typename enable_if_c<D <= digits10>::type* = 0)
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(digits10));
      mpfr_set_f(this->m_data, val.data(), GMP_RNDN);
   }
   template <unsigned D>
   mpfr_float_backend(const gmp_float<D>& val, typename disable_if_c<D <= digits10>::type* = 0)
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(digits10));
      mpfr_set_f(this->m_data, val.data(), GMP_RNDN);
   }
   mpfr_float_backend(const gmp_int& val)
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(digits10));
      mpfr_set_z(this->m_data, val.data(), GMP_RNDN);
   }
   mpfr_float_backend(const gmp_rational& val)
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(digits10));
      mpfr_set_q(this->m_data, val.data(), GMP_RNDN);
   }
   mpfr_float_backend(const mpfr_t val)
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(digits10));
      mpfr_set(this->m_data, val, GMP_RNDN);
   }
   mpfr_float_backend(const mpf_t val)
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(digits10));
      mpfr_set_f(this->m_data, val, GMP_RNDN);
   }
   mpfr_float_backend(const mpz_t val)
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(digits10));
      mpfr_set_z(this->m_data, val, GMP_RNDN);
   }
   mpfr_float_backend(const mpq_t val)
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(digits10));
      mpfr_set_q(this->m_data, val, GMP_RNDN);
   }
   mpfr_float_backend& operator=(const mpfr_float_backend& o)
   {
      *static_cast<detail::mpfr_float_imp<digits10>*>(this) = static_cast<detail::mpfr_float_imp<digits10> const&>(o);
      return *this;
   }
#ifndef BOOST_NO_RVALUE_REFERENCES
   mpfr_float_backend& operator=(mpfr_float_backend&& o) BOOST_NOEXCEPT
   {
      *static_cast<detail::mpfr_float_imp<digits10>*>(this) = static_cast<detail::mpfr_float_imp<digits10>&&>(o);
      return *this;
   }
#endif
   template <class V>
   mpfr_float_backend& operator=(const V& v)
   {
      *static_cast<detail::mpfr_float_imp<digits10>*>(this) = v;
      return *this;
   }
   mpfr_float_backend& operator=(const mpfr_t val)BOOST_NOEXCEPT
   {
      mpfr_set(this->m_data, val, GMP_RNDN);
      return *this;
   }
   mpfr_float_backend& operator=(const mpf_t val)BOOST_NOEXCEPT
   {
      mpfr_set_f(this->m_data, val, GMP_RNDN);
      return *this;
   }
   mpfr_float_backend& operator=(const mpz_t val)BOOST_NOEXCEPT
   {
      mpfr_set_z(this->m_data, val, GMP_RNDN);
      return *this;
   }
   mpfr_float_backend& operator=(const mpq_t val)BOOST_NOEXCEPT
   {
      mpfr_set_q(this->m_data, val, GMP_RNDN);
      return *this;
   }
   // We don't change our precision here, this is a fixed precision type:
   template <unsigned D>
   mpfr_float_backend& operator=(const mpfr_float_backend<D>& val) BOOST_NOEXCEPT
   {
      mpfr_set(this->m_data, val.data(), GMP_RNDN);
      return *this;
   }
   template <unsigned D>
   mpfr_float_backend& operator=(const gmp_float<D>& val) BOOST_NOEXCEPT
   {
      mpfr_set_f(this->m_data, val.data(), GMP_RNDN);
      return *this;
   }
   mpfr_float_backend& operator=(const gmp_int& val) BOOST_NOEXCEPT
   {
      mpfr_set_z(this->m_data, val.data(), GMP_RNDN);
      return *this;
   }
   mpfr_float_backend& operator=(const gmp_rational& val) BOOST_NOEXCEPT
   {
      mpfr_set_q(this->m_data, val.data(), GMP_RNDN);
      return *this;
   }
};

template <>
struct mpfr_float_backend<0> : public detail::mpfr_float_imp<0>
{
   mpfr_float_backend()
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(get_default_precision()));
   }
   mpfr_float_backend(const mpfr_t val)
   {
      mpfr_init2(this->m_data, mpfr_get_prec(val));
      mpfr_set(this->m_data, val, GMP_RNDN);
   }
   mpfr_float_backend(const mpf_t val)
   {
      mpfr_init2(this->m_data, mpf_get_prec(val));
      mpfr_set_f(this->m_data, val, GMP_RNDN);
   }
   mpfr_float_backend(const mpz_t val)
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(get_default_precision()));
      mpfr_set_z(this->m_data, val, GMP_RNDN);
   }
   mpfr_float_backend(const mpq_t val)
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(get_default_precision()));
      mpfr_set_q(this->m_data, val, GMP_RNDN);
   }
   mpfr_float_backend(const mpfr_float_backend& o) : detail::mpfr_float_imp<0>(o) {}
#ifndef BOOST_NO_RVALUE_REFERENCES
   mpfr_float_backend(mpfr_float_backend&& o) BOOST_NOEXCEPT : detail::mpfr_float_imp<0>(static_cast<detail::mpfr_float_imp<0>&&>(o)) {}
#endif
   mpfr_float_backend(const mpfr_float_backend& o, unsigned digits10) 
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(digits10));
      *this = o;
   }
   template <unsigned D>
   mpfr_float_backend(const mpfr_float_backend<D>& val)
   {
      mpfr_init2(this->m_data, mpfr_get_prec(val.data()));
      mpfr_set(this->m_data, val.data(), GMP_RNDN);
   }
   template <unsigned D>
   mpfr_float_backend(const gmp_float<D>& val)
   {
      mpfr_init2(this->m_data, mpf_get_prec(val.data()));
      mpfr_set_f(this->m_data, val.data(), GMP_RNDN);
   }
   mpfr_float_backend(const gmp_int& val)
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(get_default_precision()));
      mpfr_set_z(this->m_data, val.data(), GMP_RNDN);
   }
   mpfr_float_backend(const gmp_rational& val)
   {
      mpfr_init2(this->m_data, multiprecision::detail::digits10_2_2(get_default_precision()));
      mpfr_set_q(this->m_data, val.data(), GMP_RNDN);
   }

   mpfr_float_backend& operator=(const mpfr_float_backend& o)
   {
      *static_cast<detail::mpfr_float_imp<0>*>(this) = static_cast<detail::mpfr_float_imp<0> const&>(o);
      return *this;
   }
#ifndef BOOST_NO_RVALUE_REFERENCES
   mpfr_float_backend& operator=(mpfr_float_backend&& o) BOOST_NOEXCEPT
   {
      *static_cast<detail::mpfr_float_imp<0>*>(this) = static_cast<detail::mpfr_float_imp<0> &&>(o);
      return *this;
   }
#endif
   template <class V>
   mpfr_float_backend& operator=(const V& v)
   {
      *static_cast<detail::mpfr_float_imp<0>*>(this) = v;
      return *this;
   }
   mpfr_float_backend& operator=(const mpfr_t val) BOOST_NOEXCEPT
   {
      mpfr_set_prec(this->m_data, mpfr_get_prec(val));
      mpfr_set(this->m_data, val, GMP_RNDN);
      return *this;
   }
   mpfr_float_backend& operator=(const mpf_t val) BOOST_NOEXCEPT
   {
      mpfr_set_prec(this->m_data, mpf_get_prec(val));
      mpfr_set_f(this->m_data, val, GMP_RNDN);
      return *this;
   }
   mpfr_float_backend& operator=(const mpz_t val) BOOST_NOEXCEPT
   {
      mpfr_set_z(this->m_data, val, GMP_RNDN);
      return *this;
   }
   mpfr_float_backend& operator=(const mpq_t val) BOOST_NOEXCEPT
   {
      mpfr_set_q(this->m_data, val, GMP_RNDN);
      return *this;
   }
   template <unsigned D>
   mpfr_float_backend& operator=(const mpfr_float_backend<D>& val) BOOST_NOEXCEPT
   {
      mpfr_set_prec(this->m_data, mpfr_get_prec(val.data()));
      mpfr_set(this->m_data, val.data(), GMP_RNDN);
      return *this;
   }
   template <unsigned D>
   mpfr_float_backend& operator=(const gmp_float<D>& val) BOOST_NOEXCEPT
   {
      mpfr_set_prec(this->m_data, mpf_get_prec(val.data()));
      mpfr_set_f(this->m_data, val.data(), GMP_RNDN);
      return *this;
   }
   mpfr_float_backend& operator=(const gmp_int& val) BOOST_NOEXCEPT
   {
      mpfr_set_z(this->m_data, val.data(), GMP_RNDN);
      return *this;
   }
   mpfr_float_backend& operator=(const gmp_rational& val) BOOST_NOEXCEPT
   {
      mpfr_set_q(this->m_data, val.data(), GMP_RNDN);
      return *this;
   }
   static unsigned default_precision() BOOST_NOEXCEPT
   {
      return get_default_precision();
   }
   static void default_precision(unsigned v) BOOST_NOEXCEPT
   {
      get_default_precision() = v;
   }
   unsigned precision()const BOOST_NOEXCEPT
   {
      return multiprecision::detail::digits2_2_10(mpfr_get_prec(this->m_data));
   }
   void precision(unsigned digits10) BOOST_NOEXCEPT
   {
      mpfr_set_prec(this->m_data, multiprecision::detail::digits2_2_10((digits10)));
   }
};

template <unsigned digits10, class T>
inline typename enable_if<is_arithmetic<T>, bool>::type eval_eq(const mpfr_float_backend<digits10>& a, const T& b) BOOST_NOEXCEPT
{
   return a.compare(b) == 0;
}
template <unsigned digits10, class T>
inline typename enable_if<is_arithmetic<T>, bool>::type eval_lt(const mpfr_float_backend<digits10>& a, const T& b) BOOST_NOEXCEPT
{
   return a.compare(b) < 0;
}
template <unsigned digits10, class T>
inline typename enable_if<is_arithmetic<T>, bool>::type eval_gt(const mpfr_float_backend<digits10>& a, const T& b) BOOST_NOEXCEPT
{
   return a.compare(b) > 0;
}

template <unsigned D1, unsigned D2>
inline void eval_add(mpfr_float_backend<D1>& result, const mpfr_float_backend<D2>& o) BOOST_NOEXCEPT
{
   mpfr_add(result.data(), result.data(), o.data(), GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_subtract(mpfr_float_backend<D1>& result, const mpfr_float_backend<D2>& o) BOOST_NOEXCEPT
{
   mpfr_sub(result.data(), result.data(), o.data(), GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_multiply(mpfr_float_backend<D1>& result, const mpfr_float_backend<D2>& o) BOOST_NOEXCEPT
{
   mpfr_mul(result.data(), result.data(), o.data(), GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_divide(mpfr_float_backend<D1>& result, const mpfr_float_backend<D2>& o)
{
   mpfr_div(result.data(), result.data(), o.data(), GMP_RNDN);
}
template <unsigned digits10>
inline void eval_add(mpfr_float_backend<digits10>& result, unsigned long i) BOOST_NOEXCEPT
{
   mpfr_add_ui(result.data(), result.data(), i, GMP_RNDN);
}
template <unsigned digits10>
inline void eval_subtract(mpfr_float_backend<digits10>& result, unsigned long i) BOOST_NOEXCEPT
{
   mpfr_sub_ui(result.data(), result.data(), i, GMP_RNDN);
}
template <unsigned digits10>
inline void eval_multiply(mpfr_float_backend<digits10>& result, unsigned long i) BOOST_NOEXCEPT
{
   mpfr_mul_ui(result.data(), result.data(), i, GMP_RNDN);
}
template <unsigned digits10>
inline void eval_divide(mpfr_float_backend<digits10>& result, unsigned long i)
{
   mpfr_div_ui(result.data(), result.data(), i, GMP_RNDN);
}
template <unsigned digits10>
inline void eval_add(mpfr_float_backend<digits10>& result, long i) BOOST_NOEXCEPT
{
   if(i > 0)
      mpfr_add_ui(result.data(), result.data(), i, GMP_RNDN);
   else
      mpfr_sub_ui(result.data(), result.data(), std::abs(i), GMP_RNDN);
}
template <unsigned digits10>
inline void eval_subtract(mpfr_float_backend<digits10>& result, long i) BOOST_NOEXCEPT
{
   if(i > 0)
      mpfr_sub_ui(result.data(), result.data(), i, GMP_RNDN);
   else
      mpfr_add_ui(result.data(), result.data(), std::abs(i), GMP_RNDN);
}
template <unsigned digits10>
inline void eval_multiply(mpfr_float_backend<digits10>& result, long i) BOOST_NOEXCEPT
{
   mpfr_mul_ui(result.data(), result.data(), std::abs(i), GMP_RNDN);
   if(i < 0)
      mpfr_neg(result.data(), result.data(), GMP_RNDN);
}
template <unsigned digits10>
inline void eval_divide(mpfr_float_backend<digits10>& result, long i)
{
   mpfr_div_ui(result.data(), result.data(), std::abs(i), GMP_RNDN);
   if(i < 0)
      mpfr_neg(result.data(), result.data(), GMP_RNDN);
}
//
// Specialised 3 arg versions of the basic operators:
//
template <unsigned D1, unsigned D2, unsigned D3>
inline void eval_add(mpfr_float_backend<D1>& a, const mpfr_float_backend<D2>& x, const mpfr_float_backend<D3>& y) BOOST_NOEXCEPT
{
   mpfr_add(a.data(), x.data(), y.data(), GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_add(mpfr_float_backend<D1>& a, const mpfr_float_backend<D2>& x, unsigned long y) BOOST_NOEXCEPT
{
   mpfr_add_ui(a.data(), x.data(), y, GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_add(mpfr_float_backend<D1>& a, const mpfr_float_backend<D2>& x, long y) BOOST_NOEXCEPT
{
   if(y < 0)
      mpfr_sub_ui(a.data(), x.data(), -y, GMP_RNDN);
   else
      mpfr_add_ui(a.data(), x.data(), y, GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_add(mpfr_float_backend<D1>& a, unsigned long x, const mpfr_float_backend<D2>& y) BOOST_NOEXCEPT
{
   mpfr_add_ui(a.data(), y.data(), x, GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_add(mpfr_float_backend<D1>& a, long x, const mpfr_float_backend<D2>& y) BOOST_NOEXCEPT
{
   if(x < 0)
   {
      mpfr_ui_sub(a.data(), -x, y.data(), GMP_RNDN);
      mpfr_neg(a.data(), a.data(), GMP_RNDN);
   }
   else
      mpfr_add_ui(a.data(), y.data(), x, GMP_RNDN);
}
template <unsigned D1, unsigned D2, unsigned D3>
inline void eval_subtract(mpfr_float_backend<D1>& a, const mpfr_float_backend<D2>& x, const mpfr_float_backend<D3>& y) BOOST_NOEXCEPT
{
   mpfr_sub(a.data(), x.data(), y.data(), GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_subtract(mpfr_float_backend<D1>& a, const mpfr_float_backend<D2>& x, unsigned long y) BOOST_NOEXCEPT
{
   mpfr_sub_ui(a.data(), x.data(), y, GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_subtract(mpfr_float_backend<D1>& a, const mpfr_float_backend<D2>& x, long y) BOOST_NOEXCEPT
{
   if(y < 0)
      mpfr_add_ui(a.data(), x.data(), -y, GMP_RNDN);
   else
      mpfr_sub_ui(a.data(), x.data(), y, GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_subtract(mpfr_float_backend<D1>& a, unsigned long x, const mpfr_float_backend<D2>& y) BOOST_NOEXCEPT
{
   mpfr_ui_sub(a.data(), x, y.data(), GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_subtract(mpfr_float_backend<D1>& a, long x, const mpfr_float_backend<D2>& y) BOOST_NOEXCEPT
{
   if(x < 0)
   {
      mpfr_add_ui(a.data(), y.data(), -x, GMP_RNDN);
      mpfr_neg(a.data(), a.data(), GMP_RNDN);
   }
   else
      mpfr_ui_sub(a.data(), x, y.data(), GMP_RNDN);
}

template <unsigned D1, unsigned D2, unsigned D3>
inline void eval_multiply(mpfr_float_backend<D1>& a, const mpfr_float_backend<D2>& x, const mpfr_float_backend<D3>& y) BOOST_NOEXCEPT
{
   mpfr_mul(a.data(), x.data(), y.data(), GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_multiply(mpfr_float_backend<D1>& a, const mpfr_float_backend<D2>& x, unsigned long y) BOOST_NOEXCEPT
{
   mpfr_mul_ui(a.data(), x.data(), y, GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_multiply(mpfr_float_backend<D1>& a, const mpfr_float_backend<D2>& x, long y) BOOST_NOEXCEPT
{
   if(y < 0)
   {
      mpfr_mul_ui(a.data(), x.data(), -y, GMP_RNDN);
      a.negate();
   }
   else
      mpfr_mul_ui(a.data(), x.data(), y, GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_multiply(mpfr_float_backend<D1>& a, unsigned long x, const mpfr_float_backend<D2>& y) BOOST_NOEXCEPT
{
   mpfr_mul_ui(a.data(), y.data(), x, GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_multiply(mpfr_float_backend<D1>& a, long x, const mpfr_float_backend<D2>& y) BOOST_NOEXCEPT
{
   if(x < 0)
   {
      mpfr_mul_ui(a.data(), y.data(), -x, GMP_RNDN);
      mpfr_neg(a.data(), a.data(), GMP_RNDN);
   }
   else
      mpfr_mul_ui(a.data(), y.data(), x, GMP_RNDN);
}

template <unsigned D1, unsigned D2, unsigned D3>
inline void eval_divide(mpfr_float_backend<D1>& a, const mpfr_float_backend<D2>& x, const mpfr_float_backend<D3>& y)
{
   mpfr_div(a.data(), x.data(), y.data(), GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_divide(mpfr_float_backend<D1>& a, const mpfr_float_backend<D2>& x, unsigned long y)
{
   mpfr_div_ui(a.data(), x.data(), y, GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_divide(mpfr_float_backend<D1>& a, const mpfr_float_backend<D2>& x, long y)
{
   if(y < 0)
   {
      mpfr_div_ui(a.data(), x.data(), -y, GMP_RNDN);
      a.negate();
   }
   else
      mpfr_div_ui(a.data(), x.data(), y, GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_divide(mpfr_float_backend<D1>& a, unsigned long x, const mpfr_float_backend<D2>& y)
{
   mpfr_ui_div(a.data(), x, y.data(), GMP_RNDN);
}
template <unsigned D1, unsigned D2>
inline void eval_divide(mpfr_float_backend<D1>& a, long x, const mpfr_float_backend<D2>& y)
{
   if(x < 0)
   {
      mpfr_ui_div(a.data(), -x, y.data(), GMP_RNDN);
      mpfr_neg(a.data(), a.data(), GMP_RNDN);
   }
   else
      mpfr_ui_div(a.data(), x, y.data(), GMP_RNDN);
}

template <unsigned digits10>
inline bool eval_is_zero(const mpfr_float_backend<digits10>& val) BOOST_NOEXCEPT
{
   return 0 != mpfr_zero_p(val.data());
}
template <unsigned digits10>
inline int eval_get_sign(const mpfr_float_backend<digits10>& val) BOOST_NOEXCEPT
{
   return mpfr_sgn(val.data());
}

template <unsigned digits10>
inline void eval_convert_to(unsigned long* result, const mpfr_float_backend<digits10>& val)
{
   if(mpfr_nan_p(val.data()))
   {
      BOOST_THROW_EXCEPTION(std::runtime_error("Could not convert NaN to integer."));
   }
   *result = mpfr_get_ui(val.data(), GMP_RNDN);
}
template <unsigned digits10>
inline void eval_convert_to(long* result, const mpfr_float_backend<digits10>& val)
{
   if(mpfr_nan_p(val.data()))
   {
      BOOST_THROW_EXCEPTION(std::runtime_error("Could not convert NaN to integer."));
   }
   *result = mpfr_get_si(val.data(), GMP_RNDN);
}
#ifdef _MPFR_H_HAVE_INTMAX_T
template <unsigned digits10>
inline void eval_convert_to(unsigned long long* result, const mpfr_float_backend<digits10>& val)
{
   if(mpfr_nan_p(val.data()))
   {
      BOOST_THROW_EXCEPTION(std::runtime_error("Could not convert NaN to integer."));
   }
   *result = mpfr_get_uj(val.data(), GMP_RNDN);
}
template <unsigned digits10>
inline void eval_convert_to(long long* result, const mpfr_float_backend<digits10>& val)
{
   if(mpfr_nan_p(val.data()))
   {
      BOOST_THROW_EXCEPTION(std::runtime_error("Could not convert NaN to integer."));
   }
   *result = mpfr_get_sj(val.data(), GMP_RNDN);
}
#endif
template <unsigned digits10>
inline void eval_convert_to(double* result, const mpfr_float_backend<digits10>& val) BOOST_NOEXCEPT
{
   *result = mpfr_get_d(val.data(), GMP_RNDN);
}
template <unsigned digits10>
inline void eval_convert_to(long double* result, const mpfr_float_backend<digits10>& val) BOOST_NOEXCEPT
{
   *result = mpfr_get_ld(val.data(), GMP_RNDN);
}

//
// Native non-member operations:
//
template <unsigned Digits10>
inline void eval_sqrt(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& val) BOOST_NOEXCEPT
{
   mpfr_sqrt(result.data(), val.data(), GMP_RNDN);
}

template <unsigned Digits10>
inline void eval_abs(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& val) BOOST_NOEXCEPT
{
   mpfr_abs(result.data(), val.data(), GMP_RNDN);
}

template <unsigned Digits10>
inline void eval_fabs(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& val) BOOST_NOEXCEPT
{
   mpfr_abs(result.data(), val.data(), GMP_RNDN);
}
template <unsigned Digits10>
inline void eval_ceil(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& val) BOOST_NOEXCEPT
{
   mpfr_ceil(result.data(), val.data());
}
template <unsigned Digits10>
inline void eval_floor(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& val) BOOST_NOEXCEPT
{
   mpfr_floor(result.data(), val.data());
}
template <unsigned Digits10>
inline void eval_trunc(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& val)
{
   if(0 == mpfr_number_p(val.data()))
   {
      result = boost::math::policies::raise_rounding_error("boost::multiprecision::trunc<%1%>(%1%)", 0, number<mpfr_float_backend<Digits10> >(val), 0, boost::math::policies::policy<>()).backend();
      return;
   }
   mpfr_trunc(result.data(), val.data());
}
template <unsigned Digits10>
inline void eval_ldexp(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& val, long e) BOOST_NOEXCEPT
{
   if(e > 0)
      mpfr_mul_2exp(result.data(), val.data(), e, GMP_RNDN);
   else if(e < 0)
      mpfr_div_2exp(result.data(), val.data(), -e, GMP_RNDN);
   else
      result = val;
}
template <unsigned Digits10>
inline void eval_frexp(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& val, int* e) BOOST_NOEXCEPT
{
   long v;
   mpfr_get_d_2exp(&v, val.data(), GMP_RNDN);
   *e = v;
   eval_ldexp(result, val, -v);
}
template <unsigned Digits10>
inline void eval_frexp(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& val, long* e) BOOST_NOEXCEPT
{
   mpfr_get_d_2exp(e, val.data(), GMP_RNDN);
   return eval_ldexp(result, val, -*e);
}

template <unsigned Digits10>
inline int eval_fpclassify(const mpfr_float_backend<Digits10>& val) BOOST_NOEXCEPT
{
   return mpfr_inf_p(val.data()) ? FP_INFINITE : mpfr_nan_p(val.data()) ? FP_NAN : mpfr_zero_p(val.data()) ? FP_ZERO : FP_NORMAL;
}

template <unsigned Digits10>
inline void eval_pow(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& b, const mpfr_float_backend<Digits10>& e) BOOST_NOEXCEPT
{
   mpfr_pow(result.data(), b.data(), e.data(), GMP_RNDN);
}

#ifdef BOOST_MSVC
//
// The enable_if usage below doesn't work with msvc - but only when
// certain other enable_if usages are defined first.  It's a capricious
// and rather annoying compiler bug in other words....
//
# define BOOST_MP_ENABLE_IF_WORKAROUND (Digits10 || !Digits10) && 
#else
#define BOOST_MP_ENABLE_IF_WORKAROUND
#endif

template <unsigned Digits10, class Integer>
inline typename enable_if<mpl::and_<is_signed<Integer>, mpl::bool_<BOOST_MP_ENABLE_IF_WORKAROUND (sizeof(Integer) <= sizeof(long))> > >::type eval_pow(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& b, const Integer& e) BOOST_NOEXCEPT
{
   mpfr_pow_si(result.data(), b.data(), e, GMP_RNDN);
}

template <unsigned Digits10, class Integer>
inline typename enable_if<mpl::and_<is_unsigned<Integer>, mpl::bool_<BOOST_MP_ENABLE_IF_WORKAROUND (sizeof(Integer) <= sizeof(long))> > >::type eval_pow(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& b, const Integer& e) BOOST_NOEXCEPT
{
   mpfr_pow_ui(result.data(), b.data(), e, GMP_RNDN);
}

#undef BOOST_MP_ENABLE_IF_WORKAROUND

template <unsigned Digits10>
inline void eval_exp(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& arg) BOOST_NOEXCEPT
{
   mpfr_exp(result.data(), arg.data(), GMP_RNDN);
}

template <unsigned Digits10>
inline void eval_log(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& arg) BOOST_NOEXCEPT
{
   mpfr_log(result.data(), arg.data(), GMP_RNDN);
}

template <unsigned Digits10>
inline void eval_log10(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& arg) BOOST_NOEXCEPT
{
   mpfr_log10(result.data(), arg.data(), GMP_RNDN);
}

template <unsigned Digits10>
inline void eval_sin(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& arg) BOOST_NOEXCEPT
{
   mpfr_sin(result.data(), arg.data(), GMP_RNDN);
}

template <unsigned Digits10>
inline void eval_cos(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& arg) BOOST_NOEXCEPT
{
   mpfr_cos(result.data(), arg.data(), GMP_RNDN);
}

template <unsigned Digits10>
inline void eval_tan(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& arg) BOOST_NOEXCEPT
{
   mpfr_tan(result.data(), arg.data(), GMP_RNDN);
}

template <unsigned Digits10>
inline void eval_asin(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& arg) BOOST_NOEXCEPT
{
   mpfr_asin(result.data(), arg.data(), GMP_RNDN);
}

template <unsigned Digits10>
inline void eval_acos(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& arg) BOOST_NOEXCEPT
{
   mpfr_acos(result.data(), arg.data(), GMP_RNDN);
}

template <unsigned Digits10>
inline void eval_atan(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& arg) BOOST_NOEXCEPT
{
   mpfr_atan(result.data(), arg.data(), GMP_RNDN);
}

template <unsigned Digits10>
inline void eval_atan2(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& arg1, const mpfr_float_backend<Digits10>& arg2) BOOST_NOEXCEPT
{
   mpfr_atan2(result.data(), arg1.data(), arg2.data(), GMP_RNDN);
}

template <unsigned Digits10>
inline void eval_sinh(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& arg) BOOST_NOEXCEPT
{
   mpfr_sinh(result.data(), arg.data(), GMP_RNDN);
}

template <unsigned Digits10>
inline void eval_cosh(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& arg) BOOST_NOEXCEPT
{
   mpfr_cosh(result.data(), arg.data(), GMP_RNDN);
}

template <unsigned Digits10>
inline void eval_tanh(mpfr_float_backend<Digits10>& result, const mpfr_float_backend<Digits10>& arg) BOOST_NOEXCEPT
{
   mpfr_tanh(result.data(), arg.data(), GMP_RNDN);
}

} // namespace backends

#ifdef BOOST_NO_SFINAE_EXPR

namespace detail{

template<unsigned D1, unsigned D2>
struct is_explicitly_convertible<backends::mpfr_float_backend<D1>, backends::mpfr_float_backend<D2> > : public mpl::true_ {};

}

#endif

template<>
struct number_category<typename detail::canonical<mpfr_t, backends::mpfr_float_backend<0> >::type> : public mpl::int_<number_kind_floating_point>{};

using boost::multiprecision::backends::mpfr_float_backend;

typedef number<mpfr_float_backend<50> >    mpfr_float_50;
typedef number<mpfr_float_backend<100> >   mpfr_float_100;
typedef number<mpfr_float_backend<500> >   mpfr_float_500;
typedef number<mpfr_float_backend<1000> >  mpfr_float_1000;
typedef number<mpfr_float_backend<0> >     mpfr_float;

} // namespace multiprecision

namespace math{

namespace tools{

template <>
inline int digits<boost::multiprecision::mpfr_float>() BOOST_NOEXCEPT
{
   return boost::multiprecision::backends::detail::get_default_precision();
}
template <>
inline int digits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, boost::multiprecision::et_off> >() BOOST_NOEXCEPT
{
   return boost::multiprecision::backends::detail::get_default_precision();
}

}

}}  // namespaces

namespace std{

//
// numeric_limits [partial] specializations for the types declared in this header:
//
template<unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates> 
class numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >
{
   typedef boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> number_type;
public:
   BOOST_STATIC_CONSTEXPR bool is_specialized = true;
   BOOST_STATIC_CONSTEXPR number_type (min)() BOOST_NOEXCEPT
   { 
      initializer.do_nothing();
      static std::pair<bool, number_type> value;
      if(!value.first)
      {
         value.first = true;
         value.second = 0.5;
         mpfr_div_2exp(value.second.backend().data(), value.second.backend().data(), -mpfr_get_emin(), GMP_RNDN);
      }
      return value.second;
   }
   BOOST_STATIC_CONSTEXPR number_type (max)() BOOST_NOEXCEPT
   { 
      initializer.do_nothing();
      static std::pair<bool, number_type> value;
      if(!value.first)
      {
         value.first = true;
         value.second = 0.5;
         mpfr_mul_2exp(value.second.backend().data(), value.second.backend().data(), mpfr_get_emax(), GMP_RNDN);
      }
      return value.second;
   }
   BOOST_STATIC_CONSTEXPR number_type lowest() BOOST_NOEXCEPT
   {
      return -(max)();
   }
   BOOST_STATIC_CONSTEXPR int digits = static_cast<int>((Digits10 * 1000L) / 301L + ((Digits10 * 1000L) % 301 ? 2 : 1));
   BOOST_STATIC_CONSTEXPR int digits10 = Digits10;
   // Is this really correct???
   BOOST_STATIC_CONSTEXPR int max_digits10 = Digits10 + 2;
   BOOST_STATIC_CONSTEXPR bool is_signed = true;
   BOOST_STATIC_CONSTEXPR bool is_integer = false;
   BOOST_STATIC_CONSTEXPR bool is_exact = false;
   BOOST_STATIC_CONSTEXPR int radix = 2;
   BOOST_STATIC_CONSTEXPR number_type epsilon() BOOST_NOEXCEPT 
   { 
      initializer.do_nothing();
      static std::pair<bool, number_type> value;
      if(!value.first)
      {
         value.first = true;
         value.second = 1;
         mpfr_div_2exp(value.second.backend().data(), value.second.backend().data(), std::numeric_limits<number_type>::digits - 1, GMP_RNDN);
      }
      return value.second;
   }
   // What value should this be????
   BOOST_STATIC_CONSTEXPR number_type round_error() BOOST_NOEXCEPT 
   { 
      // returns epsilon/2
      initializer.do_nothing();
      static std::pair<bool, number_type> value;
      if(!value.first)
      {
         value.first = true;
         value.second = 1;
         mpfr_div_2exp(value.second.backend().data(), value.second.backend().data(), digits, GMP_RNDN);
      }
      return value.second;
   }
   BOOST_STATIC_CONSTEXPR long min_exponent = MPFR_EMIN_DEFAULT;
   BOOST_STATIC_CONSTEXPR long min_exponent10 = (MPFR_EMIN_DEFAULT / 1000) * 301L;
   BOOST_STATIC_CONSTEXPR long max_exponent = MPFR_EMAX_DEFAULT;
   BOOST_STATIC_CONSTEXPR long max_exponent10 = (MPFR_EMAX_DEFAULT / 1000) * 301L;
   BOOST_STATIC_CONSTEXPR bool has_infinity = true;
   BOOST_STATIC_CONSTEXPR bool has_quiet_NaN = true;
   BOOST_STATIC_CONSTEXPR bool has_signaling_NaN = false;
   BOOST_STATIC_CONSTEXPR float_denorm_style has_denorm = denorm_absent;
   BOOST_STATIC_CONSTEXPR bool has_denorm_loss = false;
   BOOST_STATIC_CONSTEXPR number_type infinity() BOOST_NOEXCEPT 
   { 
      // returns epsilon/2
      initializer.do_nothing();
      static std::pair<bool, number_type> value;
      if(!value.first)
      {
         value.first = true;
         value.second = 1;
         mpfr_set_inf(value.second.backend().data(), 1);
      }
      return value.second;
   }
   BOOST_STATIC_CONSTEXPR number_type quiet_NaN() BOOST_NOEXCEPT 
   { 
      // returns epsilon/2
      initializer.do_nothing();
      static std::pair<bool, number_type> value;
      if(!value.first)
      {
         value.first = true;
         value.second = 1;
         mpfr_set_nan(value.second.backend().data());
      }
      return value.second;
   }
   BOOST_STATIC_CONSTEXPR number_type signaling_NaN() BOOST_NOEXCEPT 
   { 
      return number_type(0); 
   }
   BOOST_STATIC_CONSTEXPR number_type denorm_min() BOOST_NOEXCEPT { return number_type(0); }
   BOOST_STATIC_CONSTEXPR bool is_iec559 = false;
   BOOST_STATIC_CONSTEXPR bool is_bounded = true;
   BOOST_STATIC_CONSTEXPR bool is_modulo = false;
   BOOST_STATIC_CONSTEXPR bool traps = true;
   BOOST_STATIC_CONSTEXPR bool tinyness_before = false;
   BOOST_STATIC_CONSTEXPR float_round_style round_style = round_to_nearest;

private:
   struct data_initializer
   {
      data_initializer()
      {
         std::numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<digits10> > >::epsilon();
         std::numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<digits10> > >::round_error();
         (std::numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<digits10> > >::min)();
         (std::numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<digits10> > >::max)();
         std::numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<digits10> > >::infinity();
         std::numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<digits10> > >::quiet_NaN();
      }
      void do_nothing()const{}
   };
   static const data_initializer initializer;
};

template<unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates> 
const typename numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::data_initializer numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::initializer;

#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION

template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST int numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::digits;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST int numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::digits10;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST int numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::max_digits10;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::is_signed;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::is_integer;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::is_exact;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST int numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::radix;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST long numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::min_exponent;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST long numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::min_exponent10;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST long numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::max_exponent;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST long numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::max_exponent10;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::has_infinity;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::has_quiet_NaN;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::has_signaling_NaN;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST float_denorm_style numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::has_denorm;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::has_denorm_loss;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::is_iec559;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::is_bounded;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::is_modulo;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::traps;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::tinyness_before;
template <unsigned Digits10, boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST float_round_style numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<Digits10>, ExpressionTemplates> >::round_style;

#endif


template<boost::multiprecision::expression_template_option ExpressionTemplates> 
class numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >
{
   typedef boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> number_type;
public:
   BOOST_STATIC_CONSTEXPR bool is_specialized = false;
   static number_type (min)() BOOST_NOEXCEPT { return number_type(0); }
   static number_type (max)() BOOST_NOEXCEPT { return number_type(0); }
   static number_type lowest() BOOST_NOEXCEPT { return number_type(0); }
   BOOST_STATIC_CONSTEXPR int digits = 0;
   BOOST_STATIC_CONSTEXPR int digits10 = 0;
   BOOST_STATIC_CONSTEXPR int max_digits10 = 0;
   BOOST_STATIC_CONSTEXPR bool is_signed = false;
   BOOST_STATIC_CONSTEXPR bool is_integer = false;
   BOOST_STATIC_CONSTEXPR bool is_exact = false;
   BOOST_STATIC_CONSTEXPR int radix = 0;
   static number_type epsilon() BOOST_NOEXCEPT { return number_type(0); }
   static number_type round_error() BOOST_NOEXCEPT { return number_type(0); }
   BOOST_STATIC_CONSTEXPR int min_exponent = 0;
   BOOST_STATIC_CONSTEXPR int min_exponent10 = 0;
   BOOST_STATIC_CONSTEXPR int max_exponent = 0;
   BOOST_STATIC_CONSTEXPR int max_exponent10 = 0;
   BOOST_STATIC_CONSTEXPR bool has_infinity = false;
   BOOST_STATIC_CONSTEXPR bool has_quiet_NaN = false;
   BOOST_STATIC_CONSTEXPR bool has_signaling_NaN = false;
   BOOST_STATIC_CONSTEXPR float_denorm_style has_denorm = denorm_absent;
   BOOST_STATIC_CONSTEXPR bool has_denorm_loss = false;
   static number_type infinity() BOOST_NOEXCEPT { return number_type(0); }
   static number_type quiet_NaN() BOOST_NOEXCEPT { return number_type(0); }
   static number_type signaling_NaN() BOOST_NOEXCEPT { return number_type(0); }
   static number_type denorm_min() BOOST_NOEXCEPT { return number_type(0); }
   BOOST_STATIC_CONSTEXPR bool is_iec559 = false;
   BOOST_STATIC_CONSTEXPR bool is_bounded = false;
   BOOST_STATIC_CONSTEXPR bool is_modulo = false;
   BOOST_STATIC_CONSTEXPR bool traps = false;
   BOOST_STATIC_CONSTEXPR bool tinyness_before = false;
   BOOST_STATIC_CONSTEXPR float_round_style round_style = round_toward_zero;
};

#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION

template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST int numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::digits;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST int numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::digits10;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST int numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::max_digits10;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::is_signed;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::is_integer;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::is_exact;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST int numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::radix;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST int numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::min_exponent;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST int numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::min_exponent10;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST int numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::max_exponent;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST int numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::max_exponent10;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::has_infinity;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::has_quiet_NaN;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::has_signaling_NaN;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST float_denorm_style numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::has_denorm;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::has_denorm_loss;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::is_iec559;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::is_bounded;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::is_modulo;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::traps;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST bool numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::tinyness_before;
template <boost::multiprecision::expression_template_option ExpressionTemplates>
BOOST_CONSTEXPR_OR_CONST float_round_style numeric_limits<boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<0>, ExpressionTemplates> >::round_style;

#endif
} // namespace std
#endif
