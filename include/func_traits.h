#pragma once
#include <tuple>
#include <any>
#include <vector>
#include <utility>

template <typename T>
struct func_traits : func_traits<decltype(&T::operator())> {};

template <typename C, typename R, typename... Args>
struct func_traits<R(C::*)(Args...)> : func_traits<R(*)(Args...)> {};

template <typename C, typename R, typename... Args>
struct func_traits<R(C::*)(Args...) const> : func_traits<R(*)(Args...)> {};

template <typename R, typename... Args> struct func_traits<R(*)(Args...)> {
    using result_type = R;
    static constexpr std::size_t input_size = sizeof...(Args);

    template <std::size_t ... I>
    static std::tuple<Args...> getArgsFromVector(std::vector<std::any> input, std::index_sequence<I...>)
    {
        return std::make_tuple(std::any_cast<Args>(input[I])...);
    }

    static std::tuple<Args...> getArgsFromVector(std::vector<std::any> input)
    {
        return getArgsFromVector(input, std::make_index_sequence<sizeof...(Args)>{});
    }
};