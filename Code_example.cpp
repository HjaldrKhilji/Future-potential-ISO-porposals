#include <iostream>
#include <type_traits>
#include <variant>
#include <functional>
#include <array>


template<template<std::size_t N, typename...> class F, std::size_t N, typename ... Args>
using Delay = F<N, Args...>;//credits to bjarne stroustrup's c++ book from 2013 for this one
template< std::size_t N, typename Head, typename ...Types>
struct get_variant_of_unique_ts :
    std::conditional_t<
    (std::is_same_v<Head, Types> || ...),
    Delay<get_variant_of_unique_ts, N - 1, Types... >,
    Delay<get_variant_of_unique_ts, N - 1, Head, Types... >
    >


{
    using intermediate =
        std::conditional_t< (std::is_same_v<Head, Types> || ...),
        Delay<get_variant_of_unique_ts, N - 1, Types... >,
        Delay<get_variant_of_unique_ts, N - 1, Head, Types... >
        >;
    using Variant_t = typename  intermediate::Variant_t;
};

template<typename Head, typename... Types>
struct get_variant_of_unique_ts<0, Head, Types...>
{
    using Variant_t = std::variant<Head*, Types*...>;
};
template<typename complex_elem, typename ... Types>
struct reference_holding_arrays {
    std::array<complex_elem, (sizeof...(Types))> book_keeper;
    template<typename ... Types_>
    constexpr reference_holding_arrays(Types_&... b) : book_keeper{ complex_elem { &b }... } {}
    constexpr complex_elem operator[](size_t index) {
        return book_keeper[index];
    }
};
template<typename ... Types>
struct tuple {

    std::tuple<Types...> data;
    using complex_elem = typename  get_variant_of_unique_ts<sizeof...(Types) - 1, Types...>::Variant_t;
    using Book_keeping_t = reference_holding_arrays<complex_elem, Types...>;
    Book_keeping_t book_keeper;
    template<typename ... Types_>
    constexpr tuple(Types_&&... b) :data{ std::forward<Types_>(b)... },
        book_keeper{
        std::apply([](auto&... args) {
        return Book_keeping_t{args...};
        }, data)

    }
    {
    }


    constexpr complex_elem operator[](size_t index) {
        return book_keeper[index];
    }
    ~tuple() = default;
};
template<typename Head>
struct tuple<Head> {
    Head head;
    constexpr tuple(Head&& a) : head{ a } {}
    constexpr tuple(Head& a) : head{ a } {}
    constexpr std::variant<Head*> operator[](size_t index) {
        return std::variant<Head*>{ &head };
    }
    ~tuple() = default;

};



int main() {
    tuple<int, double, float, int> list{ 1, 2.0, 3.0f,3 };
    std::cout << *std::get<2>(list[0]) << std::endl;
    std::cout << *std::get<0>(list[1]) << std::endl;
    std::cout << *std::get<1>(list[2]) << std::endl;
  //The reason the get<N> is shuffled in my usage is because while trying to remove duplicates, the exact order is lost. This is not an issue,since, the
  //usage of runtime tuples should be done using std::visit
    return 0;
}
