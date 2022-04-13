#pragma once

#include <type_traits>

//
// Metaprogramming Helpers used to mimic command line options.
//
namespace opt {
// Command line options are passed as undefined class types
// that get evaluated at compile time.
class none {};
class a {};
class c {};
class d {};
class l {};
class m {};
class n {};
class u {};
class w {};

// A helper that takes a list of options types and validates them.
template <typename ...Options>
struct list {
   template <typename option>
   static constexpr bool contains() {
      return (std::is_same_v<option, Options> || ...);
   }
   
   template <typename ...OptionsToTest>
   static constexpr bool contains_all() {
      return (contains<OptionsToTest>() && ...);
   }
   
   template <typename ...OptionsToTest>
   static constexpr bool contains_any() {
      return (contains<OptionsToTest>() || ...);
   }
   
   static constexpr bool empty() {
      return sizeof...(Options) == 0;
   }
};

} // namespace opt
