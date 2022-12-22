#pragma once

namespace Iris {
    template <class... Args>
    struct EventHandler {
        std::function<void(Args...)> fn;

        void operator()(Args... args) {
            fn(args...);
        }
    };

    template <class... Ts>
    class EventEmitter {
    public:
        EventEmitter() = default;

        template <class Event, class... Args>
        void add_listener(std::function<void(Args...)> fn) {
            std::get<std::vector<Event>>(tuple).push_back({std::move(fn)});
        }

        template<class Event, typename LambdaType>
        void on(LambdaType lambda) {
            add_listener<Event>(make_function(lambda));
        }

        template <class Event, class... Args>
        void emit(Args... args) {
            for (Event& e : std::get<std::vector<Event>>(tuple)) {
                e(args...);
            }
        }
    private:
        template <typename T>
        struct function_traits : public function_traits<decltype(&T::operator())> {};

        template <typename ClassType, typename... Args>
        struct function_traits<void(ClassType::*)(Args...) const> {
            [[maybe_unused]] typedef std::function<void(Args...)> f_type;
        };

        template <typename L>
        typename function_traits<L>::f_type make_function(L l){
            return (typename function_traits<L>::f_type)(l);
        }
    private:
        std::tuple<std::vector<Ts>...> tuple{};
    };
}