#ifndef __CDOUBLE_LINKEDLIST_H__
#define __CDOUBLE_LINKEDLIST_H__

#include "doublelinkedlist.h"
#include <mutex>
#include <utility>
#include <stdexcept>

using namespace std;

// Lista doblemente enlazada circular que hereda de DoubleLinkedList (lineal)
template <typename Traits>
class CDoubleLinkedList : public DoubleLinkedList<Traits> {
public:
    using value_type = typename Traits::value_type;
    using Node       = typename Traits::Node;   // DLLNode<T>
    using Comp       = typename Traits::Comp;
    using MySelf     = CDoubleLinkedList<Traits>;
    using Base       = DoubleLinkedList<Traits>;

    // Iteradores: reutilizamos los de las clases base pero con el tipo correcto
    using forward_iterator  = LinkedListForwardIterator<MySelf>;
    using backward_iterator = DoubleLinkedListBackwardIterator<MySelf>;

private:
    // Rompe la circularidad: convierte la lista en lineal (tail->next = nullptr, head->prev = nullptr)
    void breakCircle() {
        if (this->m_pTail)
            this->m_pTail->setNext(nullptr);
        if (this->m_pRoot)
            this->m_pRoot->setPrev(nullptr);
    }

    // Restaura la circularidad: tail->next = head, head->prev = tail
    void restoreCircle() {
        if (this->m_pTail && this->m_pRoot) {
            this->m_pTail->setNext(this->m_pRoot);
            this->m_pRoot->setPrev(this->m_pTail);
        }
    }

public:
    CDoubleLinkedList() : Base() {}

    CDoubleLinkedList(const CDoubleLinkedList &other) : Base() {
        if (other.size() == 0) return;
        Node *p = other.m_pRoot;
        for (size_t i = 0; i < other.m_size; ++i) {
            push_back(p->getData(), p->getRef());
            p = p->getNext();
        }
    }

    CDoubleLinkedList(CDoubleLinkedList &&other) noexcept : Base(std::move(other)) {}

    ~CDoubleLinkedList() {
        if (this->size() != 0) breakCircle();  
        }

    CDoubleLinkedList& operator=(const CDoubleLinkedList &other) = delete;
    CDoubleLinkedList& operator=(CDoubleLinkedList &&other) = delete;

    void push_front(value_type value, Ref ref) override {
        if (this->size() == 0) {
            Base::push_front(value, ref);
            this->m_pTail->setNext(this->m_pRoot);
            this->m_pRoot->setPrev(this->m_pTail);
        } else {
            breakCircle();
            Base::push_front(value, ref);
            restoreCircle();
        }
    }

    pair<value_type, Ref> pop_front() override {
        if (this->size() == 0) throw out_of_range("pop_front(): empty list");
        if (this->size() == 1) {
            // Al eliminar el único nodo, la lista queda vacía
            auto result = Base::pop_front();
            return result;
        }
        breakCircle();
        auto result = Base::pop_front();
        restoreCircle();
        return result;
    }

    void push_back(value_type value, Ref ref) override {
        if (this->size() == 0) {
            Base::push_back(value, ref);
            this->m_pTail->setNext(this->m_pRoot);
            this->m_pRoot->setPrev(this->m_pTail);
        } else {
            breakCircle();
            Base::push_back(value, ref);
            restoreCircle();
        }
    }

    pair<value_type, Ref> pop_back() override {
        if (this->size() == 0)
            throw out_of_range("pop_back(): empty list");
        if (this->size() == 1)
            return Base::pop_back();
        breakCircle();
        auto result = Base::pop_back();
        restoreCircle();
        return result;
    }

    void insert(const value_type &value, Ref ref) override {
        if (this->size() == 0) {
            Base::insert(value, ref);
            this->m_pTail->setNext(this->m_pRoot);
            this->m_pRoot->setPrev(this->m_pTail);
        } else {
            breakCircle();
            Base::insert(value, ref);
            restoreCircle();
        }
    }

    string toString() const override {
        scoped_lock lock(this->m_mtx);
        if (this->size() == 0) return "[]";
  
        Node *oldTailNext = this->m_pTail->getNext();
        Node *oldHeadPrev = this->m_pRoot->getPrev();
    
        const_cast<Node*>(this->m_pTail)->setNext(nullptr);
        const_cast<Node*>(this->m_pRoot)->setPrev(nullptr);
        string result = Base::toString();
    
        const_cast<Node*>(this->m_pTail)->setNext(oldTailNext);
        const_cast<Node*>(this->m_pRoot)->setPrev(oldHeadPrev);
        return result;
    }

    forward_iterator begin() { return forward_iterator(this, this->m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }
    backward_iterator rbegin() { return backward_iterator(this, this->m_pTail); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    forward_iterator begin() const { return forward_iterator(const_cast<MySelf*>(this), this->m_pRoot); }
    forward_iterator end()   const { return forward_iterator(const_cast<MySelf*>(this), nullptr); }
    backward_iterator rbegin() const { return backward_iterator(const_cast<MySelf*>(this), this->m_pTail); }
    backward_iterator rend()   const { return backward_iterator(const_cast<MySelf*>(this), nullptr); }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        scoped_lock lock(this->m_mtx);
        for (size_t i = 0; i < this->m_size; ++i)
            func((*this)[i], std::forward<Args>(args)...);
        cout << endl;
    }

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args&&... args) {
        scoped_lock lock(this->m_mtx);
        for (size_t i = this->m_size; i > 0; --i)
            func((*this)[i-1], std::forward<Args>(args)...);
        cout << endl;
    }

    template <typename Func, typename... Args>
    forward_iterator FirstThat(Func func, Args&&... args) {
        scoped_lock lock(this->m_mtx);
        for (size_t i = 0; i < this->m_size; ++i) {
            if (func((*this)[i], std::forward<Args>(args)...))
                return forward_iterator(this, &(*this)[i]);
        }
        return this->end();
    }

    template <typename Func, typename... Args>
    backward_iterator ReverseFirstThat(Func func, Args&&... args) {
        scoped_lock lock(this->m_mtx);
        for (size_t i = this->m_size; i > 0; --i) {
            if (func((*this)[i-1], std::forward<Args>(args)...))
                return backward_iterator(this, &(*this)[i-1]);
        }
        return backward_iterator(this, nullptr);
    }
};

#endif // __CDOUBLE_LINKEDLIST_H__