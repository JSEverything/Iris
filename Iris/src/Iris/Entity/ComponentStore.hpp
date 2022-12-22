#pragma once

template <class... Ts> requires (sizeof...(Ts) > 0)
class ComponentStore final {
public:
    template <class T>
    std::vector<T>& GetComponents() {
        return std::get<std::vector<T>>(m_Tuple);
    }
private:
    std::tuple<std::vector<Ts>...> m_Tuple{};
};