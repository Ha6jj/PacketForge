#include <gtest/gtest.h>
#include <vector>
#include <array>
#include <string>

#include <PacketForge/impl/detail/vector_view/VectorView.hpp>

namespace packet_forge {

TEST(VectorViewTest, DefaultConstructor) {
    VectorView<int> vv;
    EXPECT_EQ(vv.size(), 0u);
    EXPECT_TRUE(vv.empty());
    EXPECT_EQ(vv.data(), nullptr);
}

TEST(VectorViewTest, PointerAndSizeConstructor) {
    int arr[] = {10, 20, 30, 40};
    VectorView<int> vv(arr, 4);
    EXPECT_EQ(vv.size(), 4u);
    EXPECT_FALSE(vv.empty());
    EXPECT_EQ(vv.data(), arr);
}

TEST(VectorViewTest, CArrayConstructorWithDeduction) {
    double arr[] = {1.1, 2.2, 3.3};
    VectorView vv(arr);
    EXPECT_EQ(vv.size(), 3u);
    EXPECT_DOUBLE_EQ(vv[0], 1.1);
    EXPECT_DOUBLE_EQ(vv[2], 3.3);
}

TEST(VectorViewTest, ContainerConstructor) {
    std::vector<int> vec = {5, 6, 7, 8, 9};
    VectorView<int> vv(vec);
    EXPECT_EQ(vv.size(), vec.size());
    EXPECT_EQ(vv.data(), vec.data());
}

TEST(VectorViewTest, SizeAndEmptyMethods) {
    int data[] = {1, 2};
    VectorView<int> vv(data, 2);
    EXPECT_EQ(vv.size(), 2u);
    EXPECT_FALSE(vv.empty());

    VectorView<int> empty_vv;
    EXPECT_EQ(empty_vv.size(), 0u);
    EXPECT_TRUE(empty_vv.empty());
}

TEST(VectorViewTest, OperatorIndex) {
    int arr[] = {100, 200, 300};
    VectorView<int> vv(arr, 3);
    
    EXPECT_EQ(vv[0], 100);
    EXPECT_EQ(vv[2], 300);

    vv[1] = 999;
    EXPECT_EQ(arr[1], 999);
}

TEST(VectorViewTest, AtMethodBoundsChecking) {
    int arr[] = {1, 2, 3};
    VectorView<int> vv(arr, 3);
    
    EXPECT_EQ(vv.at(0), 1);
    EXPECT_THROW(vv.at(3), std::out_of_range);
    EXPECT_THROW(vv.at(10), std::out_of_range);
}

TEST(VectorViewTest, ConstAccessors) {
    std::vector<int> vec = {10, 20, 30};
    const VectorView<int> cvv(vec);
    
    EXPECT_EQ(cvv.size(), 3u);
    EXPECT_EQ(cvv[1], 20);
    EXPECT_EQ(cvv.at(2), 30);
    EXPECT_EQ(cvv.data(), vec.data());
}

TEST(VectorViewTest, IteratorsAndRangeBasedFor) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    VectorView<int> vv(vec);

    int sum = 0;
    for (const auto& val : vv) { sum += val; }
    EXPECT_EQ(sum, 15);

    EXPECT_EQ(*vv.begin(), 1);
    EXPECT_EQ(*(vv.end() - 1), 5);

    EXPECT_EQ(*vv.rbegin(), 5);
    EXPECT_EQ(*(vv.rend() - 1), 1);
}

TEST(VectorViewTest, FirstAndLast) {
    int arr[] = {10, 20, 30, 40, 50};
    VectorView<int> vv(arr, 5);

    auto first2 = vv.first(2);
    EXPECT_EQ(first2.size(), 2u);
    EXPECT_EQ(first2[0], 10);
    EXPECT_EQ(first2[1], 20);

    auto last2 = vv.last(2);
    EXPECT_EQ(last2.size(), 2u);
    EXPECT_EQ(last2[0], 40);
    EXPECT_EQ(last2[1], 50);

    EXPECT_EQ(vv.first(0).size(), 0u);
    EXPECT_EQ(vv.last(0).size(), 0u);
}

TEST(VectorViewTest, Subspan) {
    int arr[] = {1, 2, 3, 4, 5};
    VectorView<int> vv(arr, 5);

    auto span = vv.subspan(1, 3);
    EXPECT_EQ(span.size(), 3u);
    EXPECT_EQ(span[0], 2);
    EXPECT_EQ(span[1], 3);
    EXPECT_EQ(span[2], 4);

    auto spanToEnd = vv.subspan(2);
    EXPECT_EQ(spanToEnd.size(), 3u);
    EXPECT_EQ(spanToEnd[0], 3);
    EXPECT_EQ(spanToEnd[2], 5);

    auto spanClipped = vv.subspan(3, 10);
    EXPECT_EQ(spanClipped.size(), 2u);
    EXPECT_EQ(spanClipped[0], 4);
    EXPECT_EQ(spanClipped[1], 5);
}

TEST(VectorViewTest, Swap) {
    int arr1[] = {1, 2};
    int arr2[] = {3, 4, 5};
    VectorView<int> v1(arr1, 2);
    VectorView<int> v2(arr2, 3);

    v1.swap(v2);

    EXPECT_EQ(v1.size(), 3u);
    EXPECT_EQ(v1.data(), arr2);
    EXPECT_EQ(v2.size(), 2u);
    EXPECT_EQ(v2.data(), arr1);
}

TEST(VectorViewTest, TypeAliases) {
    using VV = VectorView<int>;
    static_assert(std::is_same_v<VV::value_type, int>);
    static_assert(std::is_same_v<VV::size_type, std::size_t>);
    static_assert(std::is_same_v<VV::reference, int&>);
    static_assert(std::is_same_v<VV::const_reference, const int&>);
    static_assert(std::is_same_v<VV::iterator, int*>);
    SUCCEED();
}

TEST(VectorViewTest, ConstArrayDeduction) {
    const double arr[] = {1.1, 2.2, 3.3};
    VectorView vv(arr);
    static_assert(std::is_same_v<decltype(vv), VectorView<const double>>);
    EXPECT_EQ(vv.size(), 3u);
    EXPECT_DOUBLE_EQ(vv[0], 1.1);
    static_assert(std::is_same_v<decltype(vv[0]), const double&>);
}

TEST(VectorViewTest, StdArrayAndStdVectorConstructors) {
    std::array<int, 4> arr = {10, 20, 30, 40};
    VectorView<int> vv_arr(arr);
    EXPECT_EQ(vv_arr.size(), 4u);
    EXPECT_EQ(vv_arr.data(), arr.data());
    vv_arr[0] = 99;
    EXPECT_EQ(arr[0], 99);

    std::vector<int> vec = {1, 2, 3};
    VectorView<int> vv_vec(vec);
    EXPECT_EQ(vv_vec.size(), vec.size());
    EXPECT_EQ(vv_vec.data(), vec.data());
}

TEST(VectorViewTest, FullViewAndEmptySubspans) {
    int arr[] = {1, 2, 3};
    VectorView<int> vv(arr, 3);

    EXPECT_EQ(vv.first(3).size(), 3u);
    EXPECT_EQ(vv.first(3).data(), arr);
    
    EXPECT_EQ(vv.last(3).size(), 3u);
    EXPECT_EQ(vv.last(3).data(), arr);

    auto full_span = vv.subspan(0);
    EXPECT_EQ(full_span.size(), 3u);

    auto empty_span = vv.subspan(3);
    EXPECT_TRUE(empty_span.empty());
    EXPECT_EQ(empty_span.size(), 0u);
    EXPECT_EQ(empty_span.data(), arr + 3);
}

TEST(VectorViewTest, SubspanWithNumericLimitsMax) {
    int arr[] = {5, 6, 7, 8};
    VectorView<int> vv(arr, 4);
    auto span = vv.subspan(1, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(span.size(), 3u);
    EXPECT_EQ(span[0], 6);
    EXPECT_EQ(span[2], 8);
}

TEST(VectorViewTest, IteratorArithmeticAndProperties) {
    std::vector<int> vec = {10, 20, 30, 40, 50};
    VectorView<int> vv(vec);
    
    EXPECT_EQ(std::distance(vv.begin(), vv.end()), 5);
    EXPECT_EQ(vv.end() - vv.begin(), 5);
    EXPECT_TRUE(vv.begin() < vv.end());
}

TEST(VectorViewTest, StlAlgorithmsCompatibility) {
    int arr[] = {5, 1, 4, 2, 3};
    VectorView<int> vv(arr, 5);

    auto it = std::find(vv.begin(), vv.end(), 4);
    EXPECT_NE(it, vv.end());
    EXPECT_EQ(*it, 4);

    std::sort(vv.begin(), vv.end());
    EXPECT_EQ(vv[0], 1);
    EXPECT_EQ(vv[4], 5);
}

TEST(VectorViewTest, SwapSelfAndWithEmpty) {
    int arr1[] = {1, 2, 3};
    VectorView<int> v1(arr1, 3);
    
    v1.swap(v1);
    EXPECT_EQ(v1.size(), 3u);
    EXPECT_EQ(v1.data(), arr1);

    VectorView<int> v2;
    v1.swap(v2);
    EXPECT_TRUE(v1.empty());
    EXPECT_EQ(v1.data(), nullptr);
    EXPECT_EQ(v2.size(), 3u);
    EXPECT_EQ(v2.data(), arr1);
}

TEST(VectorViewTest, ConstViewFromMutableArray) {
    int arr[] = {100, 200};
    VectorView<const int> cvv(arr, 2); 
    EXPECT_EQ(cvv[0], 100);
    static_assert(std::is_same_v<decltype(cvv[0]), const int&>);
}

TEST(VectorViewTest, PointerArithmeticConsistency) {
    std::array<int, 3> arr = {1, 2, 3};
    VectorView<int> vv(arr);
    for (std::size_t i = 0; i < vv.size(); ++i) {
        EXPECT_EQ(&vv[i], vv.data() + i);
        EXPECT_EQ(&vv[i], &vv.at(i));
    }
}

TEST(VectorViewTest, NoexceptSpecifiers) {
    VectorView<int> vv;
    static_assert(noexcept(vv.size()));
    static_assert(noexcept(vv.empty()));
    static_assert(noexcept(vv.begin()));
    static_assert(noexcept(vv.swap(vv)));
    static_assert(noexcept(VectorView<int>(nullptr, 0)));
}

TEST(VectorViewTest, SwappableConcept) {
    static_assert(std::is_swappable_v<VectorView<int>>);
    static_assert(std::is_swappable_v<VectorView<float>>);
    static_assert(std::is_trivially_copyable_v<VectorView<double>>);
}

struct Point { int x, y; };
TEST(VectorViewTest, CustomStructView) {
    Point pts[] = {{1, 2}, {3, 4}, {5, 6}};
    VectorView<Point> vv(pts, 3);
    EXPECT_EQ(vv[1].x, 3);
    vv[0].y = 10;
    EXPECT_EQ(pts[0].y, 10);
}

TEST(VectorViewTest, BoolTypeView) {
    bool arr[] = {true, false, true};
    VectorView<bool> vv(arr, 3);
    EXPECT_TRUE(vv[0]);
    EXPECT_FALSE(vv[1]);
    vv[2] = false;
    EXPECT_FALSE(vv[2]);
}

} // namespace packet_forge
