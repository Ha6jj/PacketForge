#pragma once

#include <PacketForge/impl/detail/vector_view/VectorView.hpp>
#include <array>

namespace packet_forge {

template <typename T, size_t N>
VectorView(std::array<T, N>&) -> VectorView<T>;

template <typename T, size_t N>
VectorView(const std::array<T, N>&) -> VectorView<const T>;

} // namespace packet_forge