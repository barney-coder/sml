//
// Copyright (c) 2012-2015 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

//[msm
//<-
#include <cassert>
#include <iostream>
#include <boost/units/detail/utility.hpp>
#include <typeinfo>
#include "msm.hpp"

//->
using e1 = msm::event<int, __LINE__>;
using e2 = msm::event<int, __LINE__>;
using e3 = msm::event<int, __LINE__>;

auto guard1 = [](auto, int i) {
  // assert(42 == i);
  std::cout << "guard1: " << i << std::endl;
  return true;
};

auto guard2 = [](auto) { return true; };
auto action1 = [] { std::cout << "action1" << std::endl; };
auto action2 = [](double, float &) { std::cout << "action2" << std::endl; };

auto controller() noexcept {
  using namespace msm;
  auto idle = init_state<__LINE__>{}; // todo constexpr counter
  auto idle2 = init_state<__LINE__>{};
  auto s1 = state<__LINE__>{};
  auto s2 = state<__LINE__>{};

  // clang-format off
  return make_transition_table(
   // +-----------------------------------------------------------------+
        idle == s1 + e1() [guard1 && guard2] / []{std::cout << "hej" << std::endl;}
	  , s1    == s1 + e1() [guard1] / (action1, action2)
   ////+-----------------------------------------------------------------+
	, idle2   == s2 + e2() [!guard1 && guard2] / (action1, []() {std::cout << "action2" << std::endl; })
   //+-----------------------------------------------------------------+
  );
  // clang-format on
}

int main() {
  auto injector = di::make_injector(di::bind<int>().to(42));
  using q = decltype(controller());
  auto sm = injector.create<decltype(controller())>();

  // std::cout <<
  // boost::units::detail::demangle(typeid(decltype(controller())).name()) <<
  // std::endl;
  sm.process_event(e1());
  sm.process_event(e1());

  // sm.visit_current_states(
  //[](auto s) { std::cout << "\t" << typeid(s).name() << std::endl; });
  // sm.process_event(e1());
  // sm.visit_current_states(
  //[](auto s) { std::cout << "\t" << typeid(s).name() << std::endl; });
  // sm.process_event(e2());
  // sm.visit_current_states(
  //[](auto s) { std::cout << "\t" << typeid(s).name() << std::endl; });
  // sm.process_event(e3());
  // sm.visit_current_states(
  //[](auto s) { std::cout << "\t" << typeid(s).name() << std::endl; });
}

//]