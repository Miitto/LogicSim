#pragma once

#include <type_traits>

template <typename Enum>
  requires std::is_enum_v<Enum>
class Bitflag {
public:
  using Underlying = std::underlying_type_t<Enum>;

private:
  Underlying m_flags = 0;

public:
  constexpr Bitflag() = default;
  constexpr Bitflag(Enum flag) : m_flags(static_cast<Underlying>(flag)) {}

  constexpr Bitflag& operator|=(Enum rhs) {
    m_flags |= static_cast<Underlying>(rhs);
    return *this;
  }
  constexpr friend Bitflag operator|(Bitflag lhs, Enum rhs) {
    lhs.m_flags |= static_cast<Underlying>(rhs);
    return lhs;
  }
  constexpr friend Bitflag operator&(Bitflag lhs, Enum rhs) {
    lhs.m_flags &= static_cast<Underlying>(rhs);
    return lhs;
  }

  constexpr friend Bitflag operator|(Bitflag lhs, Bitflag rhs) {
    lhs.m_flags |= rhs.m_flags;
    return lhs;
  }

  constexpr friend Bitflag operator&(Bitflag lhs, Bitflag rhs) {
    lhs.m_flags &= rhs.m_flags;
    return lhs;
  }

  constexpr operator Underlying() const { return m_flags; }
};

template <typename Enum>
  requires std::constructible_from<Bitflag<Enum>, Enum> && std::is_enum_v<Enum>
constexpr inline Bitflag<Enum> operator|(Enum lhs, Enum rhs) {
  Bitflag<Enum> result(lhs);
  result |= rhs;
  return result;
}
