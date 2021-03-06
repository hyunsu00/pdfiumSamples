// span.h
#pragma once

namespace experimental {
    template<typename T>
    struct span
    {
    public:
        static_assert(std::is_object<T>::value, "span<non-object-type> is illegal");

        using value_type = std::decay_t<T>;
        using element_type = T;
        using pointer = std::add_pointer_t<T>;
        using reference = std::add_lvalue_reference_t<T>;
        using iterator = pointer;

        constexpr span() noexcept : m_ptr(nullptr), m_count(0) { }
        constexpr span(std::nullptr_t) noexcept : m_ptr(nullptr), m_count(0) { }
        constexpr span(pointer ptr, size_t count) noexcept : m_ptr(ptr), m_count(count) { }
        constexpr span(pointer ptr_begin, pointer ptr_end) noexcept : m_ptr(ptr_begin), m_count(ptr_end - ptr_begin) { }

        template<size_t N>
        constexpr span(T(&arr)[N]) noexcept : m_ptr(arr), m_count(N)
        {
        }

        template<class Range,
            class = decltype(std::declval<Range>().data()),
            class = std::enable_if_t<!std::is_same<std::decay_t<Range>, span>::value>>
            constexpr span(Range&& v) noexcept : span(v.data(), v.size())
        {
            static_assert(std::is_same<typename std::decay_t<Range>::value_type, value_type>::value,
                "Cannot convert incompatible ranges");
        }

        constexpr iterator begin() const { return m_ptr; }
        constexpr iterator end() const { return m_ptr + m_count; }

        constexpr reference operator[](size_t i) const { return m_ptr[i]; }
        constexpr pointer data() const { return m_ptr; }
        constexpr size_t size() const { return m_count; }

    private:
        pointer m_ptr;
        size_t m_count;
    }; // struct span
} // experimental

namespace std {
    template<typename T>
    using span = typename experimental::span<T>;
}
