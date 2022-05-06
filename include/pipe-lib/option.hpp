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
class b {};
class c {};
class d {};
class E {};
class f {};
class i {};
class l {};
class m {};
class n {};
class r {};
class s {};
class u {};
class w {};

// A helper that takes a list of options types and validates them.
template <typename ...OptionsList>
struct list {
   template <typename Option>
   static constexpr bool contains() {
      return (std::is_same_v<Option, OptionsList> or ...);
   }
   
   template <typename ...AllowedOptions>
   static constexpr bool allows() {
      using AllowedOptionList = opt::list<AllowedOptions...>;
      return (AllowedOptionList::template contains<OptionsList>() and ...);
   }
   
   template <typename ...RequiredOptions>
   static constexpr bool requires(size_t min = sizeof...(RequiredOptions)) {
      return (contains<RequiredOptions>() + ...) >= min;
   }
   
   static constexpr bool empty() {
      return sizeof...(OptionsList) == 0;
   }
};

} // namespace opt
