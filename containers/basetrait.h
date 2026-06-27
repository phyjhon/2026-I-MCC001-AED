#ifndef __BASE_TRAIT_H__
#define __BASE_TRAIT_H__

template <typename _T, typename _Node>
struct BaseContainerTrait{
    using value_type = _T;
    using Node       = _Node;
};
template <typename T>
struct AscendingTrait {
    using Comp = less<T>;
};

template <typename T>
struct DescendingTrait {
    using Comp = greater<T>;
};

#endif // __BASE_TRAIT_H__