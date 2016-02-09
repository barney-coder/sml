All code examples include `boost/msm-lite.hpp` as well as declare a convienent msm namespace alias.

```cpp
#include <boost/msm-lite.hpp>
namespace msm = boost::msm::lite;
```

###0. Read Boost.MSM - eUML documentation
* [Boost.MSM - UML Short Guide](http://www.boost.org/doc/libs/1_60_0/libs/msm/doc/HTML/ch02.html)
* [Boost.MSM - eUML Documentation](http://www.boost.org/doc/libs/1_60_0/libs/msm/doc/HTML/ch03s04.html)

###1. Create events and states

State machine is composed of finite number of states and transitions which are triggered via events.

An Event is just a unique type, which will be processed by the state machine.

```cpp
struct my_event { ... };
```

You can also create event instance in order to simplify transition table notation.

```cpp
auto event = msm::event<my_event>;
```

If you happen to have a Clang/GCC compiler, you can create an Event on the fly.

```cpp
using namespace msm;
auto event  = "event"_t;
```

However, such event will not store any data.

A State can have entry/exit behaviour executed whenever machine enters/leaves State and
represents current location of the state machine flow.

To create a state below snippet might be used.

```cpp
msm::state<class idle> idle;
// or
auto idle = msm::state<class idle>{};
```

If you happen to have a Clang/GCC compiler, you can create a State on the fly.

```cpp
using namespace msm;
auto state  = "idle"_s;
```

However, please notice that above solution is a non-standard extension for Clang/GCC.

`msm-lite` states cannot have data as data is injected directly into guards/actions instead.

A state machine might be a State itself.

```cpp
msm::state<msm::sm<state_machine>> composite;
```

`msm-lite` supports `terminate` state, which stops events to be processed. It defined by `X`.

```cpp
  "state"_s = X;
```

States are printable too.

```cpp
assert(string("idle") == "idle"_s.c_str());
```

![CPP(BTN)](Run_Events_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/events.cpp)
![CPP(BTN)](Run_States_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/states.cpp)
![CPP(BTN)](Run_Composite_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/composite.cpp)

&nbsp;

---

###2. Create guards and actions

Guards and actions are callable objects which will be executed by the state machine in order to verify whether a transition, followed by an action should take place.

Guard MUST return boolean value.
```cpp
auto guard1 = [] {
  return true;
};

auto guard2 = [](int, double) { // guard with dependencies
  return true;
};

auto guard3 = [](int, auto event, double) { // guard with an event and dependencies
  return true;
};

struct guard4 {
    bool operator()() const noexcept {
        return true;
    }
};
```

Action MUST not return.
```cpp
auto action1 = [] { };
auto action2 = [](int, double) { }; // action with dependencies
auto action3 = [](int, auto event, double) { }; // action with an event and dependencies
struct action4 {
    void operator()() noexcept { }
};
```

![CPP(BTN)](Run_Actions_Guards_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/actions_guards.cpp)

&nbsp;

---

###3. Create a transition table

When we have states and events handy we can finally create a transition table which represents
our transitions.

`msm-lite` is using eUML-like DSL in order to be as close as possible to UML design.

* Transition Table DSL

    * Postfix Notation

    | Expression | Description |
    |------------|-------------|
    | state + event<e> [ guard ] | internal transition on event e when guard |
    | src\_state / [] {} = dst\_state | anonymous transition with action |
    | src\_state + event<e> = dst\_state | transition on event e without guard or action |
    | src\_state + event<e> [ guard ] / action = dst\_state | transition from src\_state to dst\_state on event e with guard and action |
    | src\_state + event<e> [ guard && (![]{return true;} && guard2) ] / (action, action2, []{}) = dst\_state | transition from src\_state to dst\_state on event e with guard and action |

    * Prefix Notation

    | Expression | Description |
    |------------|-------------|
    | state + event<e> [ guard ] | internal transition on event e when guard |
    | dst\_state <= src\_state / [] {} | anonymous transition with action |
    | dst\_state <= src\_state + event<e> | transition on event e without guard or action |
    | dst\_state <= src\_state + event<e> [ guard ] / action | transition from src\_state to dst\_state on event e with guard and action |
    | dst\_state <= src\_state + event<e> [ guard && (![]{return true;} && guard2) ] / (action, action2, []{}) | transition from src\_state to dst\_state on event e with guard and action |

To create a transition table [`make_transition_table`](user_guide.md#make_transition_table-state-machine) is provided.

```cpp
using namespace msm; // Postfix Notation

make_transition_table(
 *"src_state"_s + event<my_event> [ guard ] / action = "dst_state"_s
, "dst_state"_s + "other_event"_t = X
);
```

or

```cpp
using namespace msm; // Prefix Notation

make_transition_table(
  "dst_state"_s <= *"src_state"_s + event<my_event> [ guard ] / action
, X             <= "dst_state"_s  + "other_event"_t
);
```

![CPP(BTN)](Run_Transition_Table_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/transitions.cpp)
![CPP(BTN)](Run_eUML_Emulation_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/euml_emulation.cpp)

&nbsp;

---

###4. Set initial states

Initial state tells the state machine where to start. It can be set by prefixing a State with `*`.

```cpp
using namespace msm;
make_transition_table(
 *"src_state"_s + event<my_event> [ guard ] / action = "dst_state"_s,
  "dst_state"_s + event<game_over> = X
);
```

Initial/Current state might be remembered by the State Machine so that whenever it will reentered
the last active state will reactivated. In order to enable history you just have
to replace `*` with postfixed `(H)` when declaring the initial state.

```cpp
using namespace msm;
make_transition_table(
  "src_state"_s(H) + event<my_event> [ guard ] / action = "dst_state"_s,
  "dst_state"_s    + event<game_over>                   = X
);
```

You can have more than one initial state. All initial states will be executed in pseudo-parallel way
and are called orthogonal regions.

```cpp
using namespace msm;
make_transition_table(
 *"region_1"_s   + event<my_event1> [ guard ] / action = "dst_state1"_s,
  "dst_state1"_s + event<game_over> = X,

 *"region_2"_s   + event<my_event2> [ guard ] / action = "dst_state2"_s,
  "dst_state2"_s + event<game_over> = X
);
```

![CPP(BTN)](Run_Orthogonal_Regions_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/orthogonal_regions.cpp)
![CPP(BTN)](Run_History_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/history.cpp)

&nbsp;

---

###5. Create a state machine

State machine is an abstraction for transition table holding current states and processing events.
To create a state machine, we have to configure our transition table.

```cpp
class example {
public:
  auto configure() noexcept {
    using namespace msm;
    return make_transition_table(
     *"src_state"_s + event<my_event> [ guard ] / action = "dst_state"_s,
      "dst_state"_s + event<game_over> = X
    );
  }
};
```

Having transition table configured we can create a state machine.

```cpp
msm::sm<example> sm;
```

State machine constructor provides required dependencies for actions and guards.

```cpp
                            /---- event (injected from process_event)
                            |
auto guard = [](double d, auto event) { return true; }
                   |
                   \-------\
                           |
auto action = [](int i){}  |
                 |         |
                 |         |
                 \-\   /---/
                   |   |
msm::sm<exmple> s{42, 87.0};

msm::sm<exmple> s{87.0, 42}; // order in which parameters have to passed is not specificied
```

Passing and maintaining a lot of dependencies might be tedious and requires huge amount of boilerplate code.
In order to avoid it, Dependency Injection Library might be used to automate this process.
For example, we can use [Experimental Boost.DI](https://github.com/boost-experimental/di).

```cpp
auto injector = di::make_injector(
    di::bind<>.to(42)
  , di::bind<interface>.to<implementation>()
);

auto sm = injector.create<sm<example>>();
assert(sm.process_event(e1{}));
```

![CPP(BTN)](Run_Hello_World_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/hello_world.cpp)
![CPP(BTN)](Run_Dependency_Injection_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/dependency_injection.cpp)

&nbsp;

---

###6. Process events

State machine is a simple creature. Its main purpose is to process events.
In order to do it, `process_event` method might be used.

```cpp
msm::sm<example> sm;

assert(sm.process_event(my_event{})); // returns true when handled
assert(!sm.process_event(int{})); // not handled by the state machine
```

Process event might be also triggered on transition table.

```
using namespace msm;
return make_transition_table(
 *"s1"_s + event<my_event> / process_event(other_event{}) = "s2"_s,
  "s2"_s + event<other_event> = X
);
```

`msm-lite` also provides a way to dispatch dynamically created events into the state machine.

```cpp
struct game_over {
  static constexpr auto id = SDL_QUIT;
  // explicit game_over(const SDL_Event&) noexcept; // optional, when defined runtime event will be passed
};

auto dispatch_event = msm::make_dispatch_table<SDL_Event, SDL_FIRSTEVENT, SDL_LASTEVENT>(sm);
SDL_Event event{SDL_QUIT};
assert(dispatch_event(event, event.type)); // will call sm.process(game_over{});
```

![CPP(BTN)](Run_Hello_World_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/hello_world.cpp)
![CPP(BTN)](Run_Dispatch_Table_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/dispatch_table.cpp)
![CPP(BTN)](Run_SDL2_Integration_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/sdl2.cpp)

&nbsp;

---

###8. Error handling

Different errors may occur when processing events. Firstly, event might not be handled (transition has not happened).
In such scenario `process_event` returns false and `unexpected_event` is fired.

```cpp
make_transition_table(
 *"src_state"_s + event<my_event> [ guard ] / action = "dst_state"_s
, "src_state"_s + unexpected_event<some_event> = X
);
```

Usually, it is handy to create additional orthogonal region in order to handle unexpected scenarios.
This way State in which unexpected event occurred does not matter.

```cpp
make_transition_table(
 *"idle"_s + event<my_event> [ guard ] / action = "s1"_s
, "s1"_s + event<other_event> [ guard ] / action = "s2"_s
, "s2"_s + event<yet_another_event> [ guard ] / action = X

,*"error_handler"_s + unexpected_event<some_event> = X
);
```

We can always check whether a State Machine is in terminate state by.
```cpp
assert(sm.is(msm::X)); // doesn't matter how many regions there are
```

However, unexpected events do not cover all problems we can get into. Sometimes, exception might be thrown (when they are enabled).
Boost.MSM-lite handles those the same way as unexpected events via `exception` event.
Please notice that for performance reasons when exceptions are enabled (__cpp_exceptions == true) `noexcept` should be added
onto `configure` in order to disable handling exceptions when they can not be thrown.

```cpp
class example {
public:
  auto configure() noexcept; // no exceptions handling, terminate will be called when throw will happen
}
```

```cpp
class example {
public:
  auto configure(); // okay, guards/actions may throw now
}
```

```cpp
make_transition_table(
 *"idle"_s + event<event> / [] { throw std::runtime_error{"error"}; }

,*"error_handler"_s + exception<std::runtime_error> = X
, "error_handler"_s + exception<std::logic_error> = X
, "error_handler"_s + exception<> / [] { cleanup...; } = X // any exception
);
```

![CPP(BTN)](Run_Error_Handling_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/error_handling.cpp)

&nbsp;

---

###9. Testing

Sometimes it's useful to verify whether a state machine is in a specific states, for example, if
we are in a terminate state or not. We can do it with `msm-lite` using `is` or `visit_current_states`
functionality.

```cpp
msm::sm<example> sm;
assert(sm.process_event(my_event{}));
assert(sm.is(X)); // is(X, s1, ...) when you have orthogonal regions

//or

sm.visit_current_states([](auto state) { std::cout << state.c_str() << std::endl; });
```

On top of that, `msm-lite` provides testing facilities to check state machine as a whole.
`set_current_states` method is available from `testing::sm` in order to set state machine
in a requested state.

```cpp
testing::sm<example> sm{fake_data...};
sm.set_current_states("s3"_s); // set_current_states("s3"_s, "s1"_s, ...) for orthogonal regions
assert(sm.process_event(event{}));
assert(sm.is(X));
```

![CPP(BTN)](Run_Testing_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/testing.cpp)

&nbsp;

---

###10. Debugging

`msm-lite` provides logging capabilities in order to inspect state machine flow.
To enable logging you have to define `BOOST_MSM_LITE_LOG`.

```cpp
template <class SM, class TEvent>
void log_process_event(const TEvent&) {
  printf("[%s][process_event] %s\n", typeid(SM).name(), typeid(TEvent).name());
}

template <class SM, class TAction, class TEvent>
void log_guard(const TAction&, const TEvent&, bool result) {
  printf("[%s][guard] %s %s %s\n", typeid(SM).name(), typeid(TAction).name(), typeid(TEvent).name(),
         (result ? "[OK]" : "[Reject]"));
}

template <class SM, class TAction, class TEvent>
void log_action(const TAction&, const TEvent&) {
  printf("[%s][action] %s %s\n", typeid(SM).name(), typeid(TAction).name(), typeid(TEvent).name());
}

template <class SM, class TSrcState, class TDstState>
void log_state_change(const TSrcState& src, const TDstState& dst) {
  printf("[%s][transition] %s -> %s\n", typeid(SM).name(), src.c_str(), dst.c_str());
}

#define BOOST_MSM_LITE_LOG(T, SM, ...) log_##T<SM>(__VA_ARGS__)
#include <boost/msm-lite.hpp>
```

![CPP(BTN)](Run_Logging_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/logging.cpp)
![CPP(BTN)](Run_Plant_UML_Example|https://raw.githubusercontent.com/boost-experimental/msm-lite/master/example/plant_uml.cpp)

&nbsp;

---