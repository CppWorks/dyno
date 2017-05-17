// Copyright Louis Dionne 2017
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#ifndef DYNO_CONCEPT_HPP
#define DYNO_CONCEPT_HPP

#include <dyno/detail/dsl.hpp>

#include <boost/hana/at_key.hpp>
#include <boost/hana/bool.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/flatten.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>

#include <type_traits>


namespace dyno {

template <typename ...Clauses>
struct concept;

namespace detail {
  template <typename Str, typename Fun>
  constexpr auto expand_clauses(boost::hana::pair<Str, Fun> const& p) {
    return boost::hana::make_tuple(p);
  }

  template <typename ...Clauses>
  constexpr auto expand_clauses(dyno::concept<Clauses...> const&) {
    return boost::hana::flatten(
      boost::hana::make_tuple(detail::expand_clauses(Clauses{})...)
    );
  }

  struct concept_base { };
  struct is_concept {
    template <typename T, bool IsBase = std::is_base_of<detail::concept_base, T>::value>
    constexpr boost::hana::bool_<IsBase>
    operator()(boost::hana::basic_type<T>) const {
      return {};
    }
  };
} // end namespace detail

// A `concept` is a collection of clauses and refined concepts representing
// requirements for a type to model the concept.
//
// A concept is created by using `dyno::requires`.
//
// From a `concept`, one can generate a virtual function table by looking at
// the signatures of the functions defined in the concept. In the future, it
// would also be possible to do much more, like getting a predicate that checks
// whether a type satisfies the concept.
template <typename ...Clauses>
struct concept : detail::concept_base {
  boost::hana::tuple<boost::hana::basic_type<Clauses>...> clauses;

  template <typename Name>
  constexpr auto get_signature(Name name) const {
    auto clauses = all_clauses();
    return clauses[name];
  }

  static constexpr auto all_clauses() {
    auto all = boost::hana::make_tuple(detail::expand_clauses(Clauses{})...);
    auto flat = boost::hana::flatten(all);
    return boost::hana::to_map(flat);
  }
};

// Returns a sequence of the concepts refined by the given concept.
//
// Only the concepts that are refined directly by `c` are returned, i.e. we
// do not get the refined concepts of the refined concepts recursively. Also,
// the concepts are returned wrapped in `hana::basic_type`s, not straight
// concepts.
template <typename Concept>
constexpr auto refined_concepts(Concept c) {
  return boost::hana::filter(c.clauses, detail::is_concept{});
}

// Creates a `concept` with the given clauses. Note that a clause may be a
// concept itself, in which case the clauses of that concept are used, and
// that, recursively. For example:
//
// ```
// template <typename Reference>
// struct Iterator : decltype(dyno::requires(
//   Incrementable{},
//   "dereference"_s = dyno::function<Reference (dyno::T&)>
//   ...
// )) { };
// ```
//
// It is recommended to make every concept its own structure (and not just an
// alias), as above, because that ensures the uniqueness of concepts that have
// the same clauses.
template <typename ...Clauses>
constexpr dyno::concept<Clauses...> requires(Clauses ...) {
  return {};
}

} // end namespace dyno

#endif // DYNO_CONCEPT_HPP
