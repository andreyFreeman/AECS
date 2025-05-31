//
//  Templates.hpp
//  3DEngine
//
//  Created by ANDREY KLADOV on 30/05/2024.
//

#pragma once

#include <tuple>
#include <type_traits>

template <typename T, typename U>
concept Derived = std::is_base_of_v<U, T>;

template <typename T> struct TypeID {
    static const std::size_t value;
};

template <typename T> const std::size_t TypeID<T>::value = reinterpret_cast<std::size_t>(&TypeID<T>::value);

template <typename T, typename Tuple> struct Index;

template <typename T, typename... Types> struct Index<T, std::tuple<T, Types...>> {
    static constexpr std::size_t value = 0;
};

template <typename T, typename U, typename... Types> struct Index<T, std::tuple<U, Types...>> {
    static constexpr std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
};

template <typename Tuple, typename T> auto tuple_append(Tuple &&t, T &&value) {
    return std::tuple_cat(std::forward<Tuple>(t), std::make_tuple(std::forward<T>(value)));
}

template <typename T> using decay = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T, typename... Ts> struct index_of;

template <typename T, typename... Ts> struct index_of<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

template <typename T, typename U, typename... Ts>
struct index_of<T, U, Ts...> : std::integral_constant<std::size_t, 1 + index_of<T, Ts...>::value> {};
