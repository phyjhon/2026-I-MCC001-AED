#ifndef __HEAP_H__
#define __HEAP_H__

#include <vector>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <iostream>
#include <sstream>
#include "basetrait.h"
#include "../types.h"

using namespace std;

template <typename T>
class HeapNode {
private:
    T    m_data;
    Ref  m_ref;
public:
    HeapNode() : m_data(T()), m_ref(Ref()) {}
    HeapNode(const T& data, const Ref& ref) : m_data(data), m_ref(ref) {}
    HeapNode(T&& data, Ref&& ref) : m_data(move(data)), m_ref(move(ref)) {}

    const T& GetData() const { return m_data; }
    // el return
    T& GetDataRef() { return m_data; } // puntero 
    const Ref& GetRef() const { return m_ref; }
    Ref& GetRefRef() { return m_ref; }

    string ToString() const {
        ostringstream oss;
        oss << "(" << m_data << "," << m_ref << ")";
        return oss.str();
    }
};

template <typename T>
struct BaseHeapTrait : public BaseContainerTrait<T, HeapNode<T>> {};

template <typename T>
struct AscendingHeapTrait : public BaseHeapTrait<T>, public AscendingTrait<T> {};

template <typename T>
struct DescendingHeapTrait : public BaseHeapTrait<T>, public DescendingTrait<T> {};

template <typename Traits>
class Heap {
public:
    using value_type = typename Traits::value_type;
    using Node       = typename Traits::Node;
    using Comp       = typename Traits::Comp;

private:
    vector<Node>       m_heap;
    Comp               m_comp;
    mutable mutex      m_mtx;

    static size_t parent(size_t i) noexcept { return (i - 1) / 2; }
    static size_t left(size_t i)   noexcept { return 2 * i + 1; }
    static size_t right(size_t i)  noexcept { return 2 * i + 2; }

    void heapify_up(size_t idx) {
        while (idx > 0) {
            size_t p = parent(idx);
            if (m_comp(m_heap[idx].GetData(), m_heap[p].GetData())) {
                swap(m_heap[idx], m_heap[p]);
                idx = p;
            } else break;
        }
    }

    void heapify_down(size_t idx) {
        size_t n = m_heap.size();
        while (left(idx) < n) {
            size_t child = left(idx);
            size_t r = right(idx);
            if (r < n && m_comp(m_heap[r].GetData(), m_heap[child].GetData()))
                child = r;
            if (m_comp(m_heap[idx].GetData(), m_heap[child].GetData()))
                break;
            swap(m_heap[idx], m_heap[child]);
            idx = child;
        }
    }

    void build_heap() {
        if (m_heap.size() < 2) return;
        for (size_t i = parent(m_heap.size() - 1); ; --i) {
            heapify_down(i);
            if (i == 0) break;
        }
    }

public:
    Heap() : m_comp(Comp()) {}

    explicit Heap(const vector<Node>& vec) : m_heap(vec), m_comp(Comp()) { build_heap(); }
    explicit Heap(vector<Node>&& vec) : m_heap(std::move(vec)), m_comp(Comp()) { build_heap(); }

    Heap(const Heap& other) {
    lock_guard<std::mutex> lock(other.m_mtx);
    m_heap = other.m_heap;
    m_comp = other.m_comp;
}


    Heap(Heap&& other) noexcept : m_heap(std::move(other.m_heap)), m_comp(std::move(other.m_comp)) {}

    Heap& operator=(Heap other) noexcept {
        swap(*this, other);
        return *this;
    }

    friend void swap(Heap& lhs, Heap& rhs) noexcept {
        using std::swap;
        swap(lhs.m_heap, rhs.m_heap);
        swap(lhs.m_comp, rhs.m_comp);
    }

    ~Heap() = default;

    void insert(const value_type& value, const Ref& ref) {
        lock_guard<mutex> lock(m_mtx);
        m_heap.emplace_back(value, ref);
        heapify_up(m_heap.size() - 1);
    }

    Node extract() {
        lock_guard<mutex> lock(m_mtx);
        if (m_heap.empty()) throw out_of_range("Heap::extract: empty");
        Node root = std::move(m_heap[0]);
        if (m_heap.size() == 1) {
            m_heap.pop_back();
        } else {
            m_heap[0] = std::move(m_heap.back());
            m_heap.pop_back();
            heapify_down(0);
        }
        return root;
    }

    const Node& peek() const {
        lock_guard<mutex> lock(m_mtx);
        if (m_heap.empty()) throw out_of_range("Heap::peek: empty");
        return m_heap[0];
    }

    const value_type peek_min() const { return peek().GetData(); }

    bool empty() const {
        lock_guard<mutex> lock(m_mtx);
        return m_heap.empty();
    }

    size_t size() const {
        lock_guard<mutex> lock(m_mtx);
        return m_heap.size();
    }

    friend ostream& operator<<(ostream& os, const Heap& heap) {
        lock_guard<mutex> lock(heap.m_mtx);
        os << heap.m_heap.size() << " ";
        for (const auto& node : heap.m_heap)
            os << node.GetData() << " " << node.GetRef() << " ";
        return os;
    }

    friend istream& operator>>(istream& is, Heap& heap) {
        lock_guard<mutex> lock(heap.m_mtx);
        size_t n;
        //is >> n;
        if (!(is >> n)) return is;
        heap.m_heap.clear();
        heap.m_heap.reserve(n);
        value_type data;
        Ref ref;
        for (size_t i = 0; i < n; ++i) {
            is >> data >> ref;
            heap.m_heap.emplace_back(data, ref);
        }
        heap.build_heap();
        return is;
    }
};

#endif // __HEAP_H__