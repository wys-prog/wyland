#pragma once

namespace op {
  template <typename TyA, typename TyB>
  void mov(TyA &a, TyB &b) { a = b; };

  template <typename TyA, typename TyB>
  void add(TyA &a, TyB &b) { a += b; };

  template <typename TyA, typename TyB>
  void sub(TyA &a, TyB &b) { a -= b; };

  template <typename TyA, typename TyB>
  void mul(TyA &a, TyB &b) { a *= b; };
  
  template <typename TyA, typename TyB>
  void div(TyA &a, TyB &b) { a /= b; };

  template <typename TyA, typename TyB>
  void mod(TyA &a, TyB &b) { a = a % b; };
} // namespace op
