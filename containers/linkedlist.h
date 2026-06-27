#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <iostream>
#include <cstddef>
#include <string>
#include <sstream>
#include <mutex>
#include "general_iterator.h"
#include "../util.h"
#include "../types.h"
#include "../foreach.h"
#include "basetrait.h"

using namespace std;

template <typename Container>
class LinkedListForwardIterator : public general_iterator<Container, 
                                            LinkedListForwardIterator<Container>>{
    using MySelf = LinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
public:
    MySelf& operator++(){
        this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
};

template <typename T>
class LLNode{
public:
    using value_type = T;
    using Node       = LLNode<T>;
protected:
    value_type m_data;
    Ref        m_ref;
    Node      *m_pNext;
public:
    LLNode(){}
    LLNode(value_type data, Ref ref, Node *pNext = nullptr) 
           : m_data(data), m_ref(ref), m_pNext(pNext) {}
    virtual ~LLNode() {}

    value_type      getData() const { return m_data; }
    value_type&     getDataRef()    { return m_data; }
    void            setData(value_type data) { m_data = data; }
    Ref             getRef() const  { return m_ref; }
    Ref&            getRefRef()     { return m_ref; }
    void            setRef(Ref ref) { m_ref = ref; }
    virtual Node*   getNext() const { return m_pNext; }
    void            setNext(Node *pNext) { m_pNext = pNext; }
};

template <typename T>
ostream &operator<<(ostream &os, const LLNode<T> &node){
    return os << "(" << node.getData() << ", " << node.getRef() << ")";
}

template <typename T>
struct BaseLinkedListTrait : public BaseContainerTrait<T, LLNode<T>>{};

template <typename T>
struct AscendingLinkedListTrait : public BaseLinkedListTrait<T>{
    using Comp = less<T>;
};

template <typename T>
struct DescendingLinkedListTrait : public BaseLinkedListTrait<T>{
    using Comp = greater<T>;
};

template <typename Traits>
class LinkedList{
public:
    using value_type = typename Traits::value_type;
    using Node       = typename Traits::Node;
    using Comp       = typename Traits::Comp;
    using MySelf     = LinkedList<Traits>;
    using forward_iterator = LinkedListForwardIterator<MySelf>;

protected:
    Node *m_pRoot = nullptr;
    Node *m_pTail = nullptr;
    size_t m_size = 0;
    Comp   m_comp;
    mutable mutex m_mtx;

public:
    LinkedList() = default;
    LinkedList(const LinkedList &other) {
        for (Node *p = other.m_pRoot; p; p = p->getNext())
            push_back(p->getData(), p->getRef());
    }

    LinkedList(LinkedList &&other) noexcept {
        scoped_lock<mutex> lock(m_mtx);
        m_pRoot = exchange(other.m_pRoot, nullptr);
        m_pTail = exchange(other.m_pTail, nullptr);
        m_size  = exchange(other.m_size, 0);
    }

    LinkedList& operator=(const LinkedList &other) = delete;
    LinkedList& operator=(LinkedList &&other) = delete;

    virtual ~LinkedList() {
        scoped_lock<mutex> lock(m_mtx);
        Node* pTemp = m_pRoot;
        while (pTemp){
            Node* pNext = pTemp->getNext();
            delete pTemp;
            pTemp = pNext;
        }
    }

    virtual void push_front(value_type value, Ref ref) {
        Node *pNew = new Node(value, ref, m_pRoot);
        scoped_lock<mutex> lock(m_mtx);
        m_pRoot = pNew;
        if (m_size == 0) m_pTail = pNew;
        ++m_size;
    }

    virtual pair<value_type, Ref> pop_front() {
        scoped_lock<mutex> lock(m_mtx);
        if (!m_pRoot) throw out_of_range("pop_front(): empty list");
        Node *pDel = m_pRoot;
        m_pRoot = m_pRoot->getNext();
        --m_size;
        auto result = make_pair(pDel->getData(), pDel->getRef());
        delete pDel;
        if (m_size == 0) m_pTail = nullptr;
        return result;
    }

    virtual void push_back(value_type value, Ref ref) {
        Node *pNew = new Node(value, ref, nullptr);
        scoped_lock<mutex> lock(m_mtx);
        if (m_size == 0){
            m_pRoot = pNew;
            m_pTail = pNew;
        } else {
            m_pTail->setNext(pNew);
            m_pTail = pNew;
        }
        ++m_size;
    }

    virtual pair<value_type, Ref> pop_back() {
        scoped_lock<mutex> lock(m_mtx);
        if (!m_pRoot) throw out_of_range("pop_back(): empty list");
        if (m_pRoot == m_pTail) {
            auto result = make_pair(m_pTail->getData(), m_pTail->getRef());
            delete m_pTail;
            m_pRoot = m_pTail = nullptr;
            --m_size;
            return result;
        }
        Node *pTemp = m_pRoot;
        while (pTemp->getNext() != m_pTail)
            pTemp = pTemp->getNext();
        auto result = make_pair(m_pTail->getData(), m_pTail->getRef());
        delete m_pTail;
        pTemp->setNext(nullptr);
        m_pTail = pTemp;
        --m_size;
        return result;
    }

    virtual void insert(const value_type &value, Ref ref) {
        scoped_lock<mutex> lock(m_mtx);
        Node *pCurrent = m_pRoot;
        Node *pPrev = nullptr;
        while (pCurrent && m_comp(pCurrent->getData(), value)) {
            pPrev = pCurrent;
            pCurrent = pCurrent->getNext();
        }
        Node *pNew = new Node(value, ref, pCurrent);
        if (pPrev) pPrev->setNext(pNew);
        else m_pRoot = pNew;
        if (!pCurrent) m_pTail = pNew;
        ++m_size;
    }

    virtual Node& operator[](size_t index) const {
        if (index >= m_size) throw out_of_range("Index out of range");
        Node *p = m_pRoot;
        for (size_t i = 0; i < index; ++i) p = p->getNext();
        return *p;
    }

    virtual size_t size() const { return m_size; }
    virtual string toString() const {
        stringstream ss;
        ss << "[";
        Node *pNode = m_pRoot;
        if (m_size > 0){
            for (size_t i = 0; i < m_size-1; ++i){
                ss << *pNode << ",";
                pNode = pNode->getNext();
            }
            ss << *pNode;
        }
        ss << "]";
        return ss.str();
    }

    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        unique_lock<mutex> lock(m_mtx);
        ::ForEach(begin(), end(), func, std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    forward_iterator FirstThat(Func func, Args&&... args) {
        return ::FirstThat(begin(), end(), func, std::forward<Args>(args)...);
    }
};

template <typename Traits>
ostream& operator<<(ostream& os, const LinkedList<Traits>& list) {
    return os << list.toString();
}

template <typename Traits>
istream& operator>>(istream& is, LinkedList<Traits>& list) {
    using value_type = typename LinkedList<Traits>::value_type;
    string line;
    getline(is, line);
    for (char& c : line) {
        if (c == '[' || c == ']' || c == '(' || c == ')' || c == ',')
            c = ' ';
    }
    value_type value;
    Ref ref;
    stringstream ss(line);
    while (ss >> value >> ref)
        list.push_back(value, ref);
    return is;
}

#endif // __LINKEDLIST_H__