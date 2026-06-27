#ifndef __DOUBLE_LINKEDLIST_H__
#define __DOUBLE_LINKEDLIST_H__

#include "linkedlist.h"
#include <utility>
#include <stdexcept>

using namespace std;

template <typename T>
class DLLNode : public LLNode<T> {
public:
    using value_type = T;
    using Node       = DLLNode<T>;

private:
    Node *m_pPrev;

public:
    DLLNode(T data, Ref ref, Node *pNext = nullptr, Node *pPrev = nullptr)
        : LLNode<T>(data, ref, pNext), m_pPrev(pPrev) {}

    virtual Node* getNext() const override {
        return static_cast<Node*>(LLNode<T>::getNext());
    }

    Node* getPrev() const { return m_pPrev; }
    Node*& getPrevRef()   { return m_pPrev; }
    void setPrev(Node* pPrev) { m_pPrev = pPrev; }
};

// Traits para lista doble
template <typename T>
struct BaseDoubleLinkedListTrait : public BaseContainerTrait<T, DLLNode<T>> {};

template <typename T>
struct AscendingDoubleLinkedListTrait : public BaseDoubleLinkedListTrait<T> {
    using Comp = less<T>;
};

template <typename T>
struct DescendingDoubleLinkedListTrait : public BaseDoubleLinkedListTrait<T> {
    using Comp = greater<T>;
};

// Iterador hacia atrás
template <typename Container>
class DoubleLinkedListBackwardIterator
    : public general_iterator<Container, DoubleLinkedListBackwardIterator<Container>> {
    using MySelf = DoubleLinkedListBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
public:
    MySelf& operator++() {
        if (this->m_pNode) this->m_pNode = this->m_pNode->getPrev();
        return *this;
    }
};

// DoubleLinkedList hereda de LinkedList
template <typename Traits>
class DoubleLinkedList : public LinkedList<Traits> {
public:
    using value_type = typename Traits::value_type;
    using Node       = typename Traits::Node;
    using Comp       = typename Traits::Comp;
    using MySelf     = DoubleLinkedList<Traits>;

    using forward_iterator  = LinkedListForwardIterator<MySelf>;
    using backward_iterator = DoubleLinkedListBackwardIterator<MySelf>;

    DoubleLinkedList() : LinkedList<Traits>() {}

    DoubleLinkedList(const DoubleLinkedList &other) : LinkedList<Traits>() {
        for (Node *p = other.m_pRoot; p; p = p->getNext())
            push_back(p->getData(), p->getRef());
    }

    DoubleLinkedList(DoubleLinkedList &&other) noexcept
        : LinkedList<Traits>(std::move(other)) {}

    // Los operadores de asignación se pueden implementar si se desean
    DoubleLinkedList& operator=(const DoubleLinkedList &other) = delete;
    DoubleLinkedList& operator=(DoubleLinkedList &&other) = delete;

    // Sobrescritura de métodos para mantener enlaces prev
    void push_front(value_type value, Ref ref) override {
        scoped_lock lock(this->m_mtx);
        Node *pNew = new Node(value, ref, this->m_pRoot, nullptr);
        if (this->m_pRoot) this->m_pRoot->setPrev(pNew);
        else this->m_pTail = pNew;
        this->m_pRoot = pNew;
        ++this->m_size;
    }

    pair<value_type, Ref> pop_front() override {
        scoped_lock lock(this->m_mtx);
        if (!this->m_pRoot) throw out_of_range("pop_front(): empty list");
        Node *pDel = this->m_pRoot;
        this->m_pRoot = this->m_pRoot->getNext();
        if (this->m_pRoot) this->m_pRoot->setPrev(nullptr);
        else this->m_pTail = nullptr;
        auto result = make_pair(pDel->getData(), pDel->getRef());
        delete pDel;
        --this->m_size;
        return result;
    }

    void push_back(value_type value, Ref ref) override {
        scoped_lock lock(this->m_mtx);
        Node *pNew = new Node(value, ref, nullptr, this->m_pTail);
        if (this->m_pTail) this->m_pTail->setNext(pNew);
        else this->m_pRoot = pNew;
        this->m_pTail = pNew;
        ++this->m_size;
    }

    pair<value_type, Ref> pop_back() override {
        scoped_lock lock(this->m_mtx);
        if (!this->m_pTail) throw out_of_range("pop_back(): empty list");
        Node *pDel = this->m_pTail;
        this->m_pTail = this->m_pTail->getPrev();
        if (this->m_pTail) this->m_pTail->setNext(nullptr);
        else this->m_pRoot = nullptr;
        auto result = make_pair(pDel->getData(), pDel->getRef());
        delete pDel;
        --this->m_size;
        return result;
    }

    // Inserción ordenada con doble enlace
    void insert(const value_type &value, Ref ref) override {
        scoped_lock lock(this->m_mtx);
        Node *pCurrent = this->m_pRoot;
        Node *pPrev = nullptr;
        while (pCurrent && this->m_comp(pCurrent->getData(), value)) {
            pPrev = pCurrent;
            pCurrent = pCurrent->getNext();
        }
        Node *pNew = new Node(value, ref, pCurrent, pPrev);
        if (pPrev) pPrev->setNext(pNew);
        else this->m_pRoot = pNew;
        if (pCurrent) pCurrent->setPrev(pNew);
        else this->m_pTail = pNew;
        ++this->m_size;
    }

    // Métodos adicionales de lista doble
    backward_iterator rbegin() { return backward_iterator(this, this->m_pTail); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args&&... args) {
        scoped_lock lock(this->m_mtx);
        ::ForEach(rbegin(), rend(), func, forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    backward_iterator ReverseFirstThat(Func func, Args&&... args) {
        scoped_lock lock(this->m_mtx);
        return ::FirstThat(rbegin(), rend(), func, forward<Args>(args)...);
    }

};

#endif // __DOUBLE_LINKEDLIST_H__