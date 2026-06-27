#ifndef __CLINKEDLIST_H__
#define __CLINKEDLIST_H__

#include "linkedlist.h"
#include <mutex>
#include <utility>
#include <stdexcept>

using namespace std;

template <typename Traits>
class CLinkedList : public LinkedList<Traits> {
public:
    using value_type = typename Traits::value_type;
    using Node       = typename Traits::Node;
    using Comp       = typename Traits::Comp;
    using MySelf     = CLinkedList<Traits>;
    using Base       = LinkedList<Traits>;

    using forward_iterator = LinkedListForwardIterator<MySelf>;

private:
    void breakCircle() {
        if (this->m_pTail) this->m_pTail->setNext(nullptr);
    }

    void restoreCircle() {
        if (this->m_pTail && this->m_pRoot) this->m_pTail->setNext(this->m_pRoot);
    }

public:
    CLinkedList() : Base() {}

    CLinkedList(const CLinkedList &other) : Base() {
        if (other.size() == 0) return;
        Node *p = other.m_pRoot;
        for (size_t i = 0; i < other.m_size; ++i) {
            push_back(p->getData(), p->getRef());
            p = p->getNext();
        }
    }

    CLinkedList(CLinkedList &&other) noexcept : Base(std::move(other)) {}

    ~CLinkedList() {
        if (this->size() != 0)
            this->m_pTail->setNext(nullptr);
    }

    CLinkedList& operator=(const CLinkedList &other) = delete;
    CLinkedList& operator=(CLinkedList &&other) = delete;

    void push_front(value_type value, Ref ref) override {
        if (this->size() == 0) {
            Base::push_front(value, ref);
            this->m_pTail->setNext(this->m_pRoot);
        } else {
            breakCircle();
            Base::push_front(value, ref);
            restoreCircle();
        }
    }

    pair<value_type, Ref> pop_front() override {
        if (this->size() == 0) throw out_of_range("pop_front(): empty list");
        if (this->size() == 1) return Base::pop_front();
        breakCircle();
        auto result = Base::pop_front();
        restoreCircle();
        return result;
    }

    void push_back(value_type value, Ref ref) override {
        if (this->size() == 0) {
            Base::push_back(value, ref);
            this->m_pTail->setNext(this->m_pRoot);
        } else {
            breakCircle();
            Base::push_back(value, ref);
            restoreCircle();
        }
    }

    pair<value_type, Ref> pop_back() override {
        if (this->size() == 0) throw out_of_range("pop_back(): empty list");
        if (this->size() == 1) return Base::pop_back();
        breakCircle();
        auto result = Base::pop_back();
        restoreCircle();
        return result;
    }

    void insert(const value_type &value, Ref ref) override {
        if (this->size() == 0) {
            Base::insert(value, ref);
            this->m_pTail->setNext(this->m_pRoot);
        } else {
            breakCircle();
            Base::insert(value, ref);
            restoreCircle();
        }
    }

    string toString() const override {
        scoped_lock lock(this->m_mtx);
        if (this->size() == 0) return "[]";
        Node *oldNext = this->m_pTail->getNext();
        const_cast<Node*>(this->m_pTail)->setNext(nullptr);
        string result = Base::toString();
        const_cast<Node*>(this->m_pTail)->setNext(oldNext);
        return result;
    }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        scoped_lock lock(this->m_mtx);
        for (size_t i = 0; i < this->m_size; ++i)
            func((*this)[i], std::forward<Args>(args)...);
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
};

#endif // __CLINKEDLIST_H__