#pragma once

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <iterator>
#include <limits>

namespace packet_forge {

template <typename T>
class VectorView
{
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static_assert(!std::is_reference_v<T>, "VectorView<T>: T cannot be a reference");
    static_assert(!std::is_void_v<T>, "VectorView<T>: T cannot be void");
    static_assert(std::is_object_v<T>, "VectorView<T>: T must be an object type");

    constexpr VectorView() noexcept = default;

    constexpr VectorView(pointer ptr, size_type size) noexcept
        : ptr_(ptr), size_(size) 
    {
        assert(!(ptr == nullptr && size > 0));
    }

    template <size_type N>
    constexpr VectorView(element_type (&arr)[N]) noexcept
        : VectorView(arr, N) {}

    template <typename Container,
              typename = decltype(std::declval<Container>().data()),
              typename = decltype(std::declval<Container>().size())>
    constexpr VectorView(Container& cont) noexcept
        : VectorView(cont.data(), cont.size()) {}

    template <typename Container,
              typename = decltype(std::declval<Container>().data()),
              typename = decltype(std::declval<Container>().size())>
    constexpr VectorView(const Container& cont) noexcept
        : VectorView(cont.data(), cont.size()) {}


    constexpr size_type size() const noexcept { return size_; }
    constexpr bool empty() const noexcept { return size_ == 0; }

    // Non-Const methods
    constexpr reference operator[](size_type idx) noexcept
    {
        assert(idx < size_ && "Index out of bounds");
        return *(ptr_ + idx);
    }

    constexpr reference at(size_type idx)
    {
        if (idx >= size_) throw std::out_of_range("VectorView index out of range");
        return *(ptr_ + idx);
    }

    constexpr pointer data() noexcept { return ptr_; }
    constexpr iterator begin() noexcept { return ptr_; }
    constexpr iterator end() noexcept { return ptr_ + size_; }
    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

    // Const methods
    constexpr const_reference operator[](size_type idx) const noexcept
    {
        assert(idx < size_ && "Index out of bounds");
        return *(ptr_ + idx);
    }

    constexpr const_reference at(size_type idx) const
    {
        if (idx >= size_) throw std::out_of_range("VectorView index out of range");
        return *(ptr_ + idx);
    }

    constexpr const_pointer data() const noexcept { return ptr_; }
    constexpr const_iterator begin() const noexcept { return ptr_; }
    constexpr const_iterator end() const noexcept { return ptr_ + size_; }
    constexpr const_reverse_iterator rbegin() const noexcept
    { 
        return const_reverse_iterator(end()); 
    }
    constexpr const_reverse_iterator rend() const noexcept
    { 
        return const_reverse_iterator(begin()); 
    }


    constexpr VectorView first(size_type count) const noexcept
    {
        assert(count <= size_ && "Count exceeds size");
        return {ptr_, count};
    }

    constexpr VectorView last(size_type count) const noexcept
    {
        assert(count <= size_ && "Count exceeds size");
        return {ptr_ + size_ - count, count};
    }

    constexpr VectorView subspan(size_type offset, 
                                 size_type count = std::numeric_limits<size_type>::max()) const noexcept
    {
        assert(offset <= size_ && "Offset exceeds size");
        count = std::min(count, size_ - offset);
        return {ptr_ + offset, count};
    }

    constexpr void swap(VectorView& other) noexcept
    {
        std::swap(ptr_, other.ptr_);
        std::swap(size_, other.size_);
    }

private:
    pointer ptr_ = nullptr;
    size_type size_ = 0;
};

template <typename T, size_t N>
VectorView(T (&)[N]) -> VectorView<T>;

template <typename T, size_t N>
VectorView(const T (&)[N]) -> VectorView<const T>;

} // namespace packet_forge
