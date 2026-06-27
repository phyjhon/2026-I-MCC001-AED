#include "containers/linkedlist.h"
#include "containers/doublelinkedlist.h"
#include "containers/Clinkedlist.h" 
#include "containers/CDoublelinkedlist.h" 
#include "containers/Heap.h"
#include <fstream>
#include <iostream>

template <typename Node>
void Print(Node &node, ostream& os){//
    os << node << ",";
}

template <typename Node>
void AddX(Node &node, typename Node::value_type value){
    node.getDataRef() += value;
}

template <typename Node>
void AddY(Node &node, typename Node::value_type value1, typename Node::value_type value2){
    node.getDataRef() += value1 + value2;
}

template <typename Node, typename T>
bool IsGreaterThan(Node &node, T x){
    return node.getDataRef() > x;
}

void LinkedListDemo(){
    // 
    LinkedList<DescendingLinkedListTrait<TI>> list1;
    list1.insert(6, 15);
    list1.insert(2, 25);
    list1.insert(9, 35);
    list1.insert(1, 45);
    list1.insert(7, 55);
    cout << "Lista descendente: " << list1 << endl;

    LinkedList<AscendingLinkedListTrait<TI>> list2;
    using LI = LinkedList<AscendingLinkedListTrait<TI>>::Node;
    list2.insert(6, 15);
    list2.insert(2, 25);
    list2.insert(9, 35);
    list2.insert(1, 45);
    list2.insert(7, 55);
    cout << "Lista ascendente : " << list2 << endl;

    list2.ForEach(AddX<LI>, 3);
    cout << "Prueba ForEach + 3: " << endl;
    list2.ForEach(Print<LI>, cout);
    list2.ForEach(AddY<LI>, 10, 11);
    cout << "Prueba ForEach + 10 + 11: " << endl;
    list2.ForEach(Print<LI>, cout);
    
    cout << "Prueba First That: " << endl;
    auto it = list2.FirstThat(IsGreaterThan<LI, TI>, 6);
    if (it != list2.end())
        cout << "Primer mayor a 6   : " << *it << endl;
    cout << "Fin recorrido con iteradores" << endl;

    cout << "Prueba PushFront: " << endl;
    list1.push_front(5, 5);
    cout << "Lista ascendente 1: " << list1 << endl;

    cout << "Prueba PopFront: " << endl;
    list1.pop_front();
    cout << "Lista ascendente 1: " << list1 << endl;

    LinkedList<AscendingLinkedListTrait<TI>> list3;
    cout << "Prueba PushBack: " << endl;
    list3.push_back(1, 10);
    list3.push_back(2, 20);
    list3.push_back(3, 30);
    cout << "Lista ascendente 3: " << list3 << endl;

    cout << "Prueba PopBack: " << endl;
    list3.pop_back();
    cout << "Lista ascendente 3: " << list3 << endl;

    list3.push_back(4, 40);
    cout << "Prueba Copy Constructor: " << endl;
    LinkedList<AscendingLinkedListTrait<TI>> list4(list3);
    cout << "Lista ascendente 4: " << list4 << endl;

    list3.push_back(5, 50);
    cout << "Prueba Move Constructor: " << endl;
    LinkedList<AscendingLinkedListTrait<TI>> list5 = move(list3);
    cout << "Lista ascendente 5: " << list5 << endl;

    //cout << "Prueba del Destructor: " << endl;
    //list4.~LinkedList();
    //cout << "Lista ascendente 4: " << list4 << endl;

    cout << "Prueba del operador >>: "<<endl;
    ofstream ofs;
    ofs.open("lista1.txt");
    ofs <<  list1 <<   endl;
    ofs.close();

    ifstream file("lista1.txt");
    LinkedList<AscendingLinkedListTrait<TI>> list6;
    file >> list6;
    cout << "Lista 6 creada con datos de la lista 1: " << list6 << endl;

    cout << "Prueba operador []: " << endl;
    cout << "Lista5 [2]: " << list5[2] << endl;
}

void DoubleLinkedListDemo(){
    DoubleLinkedList<DescendingDoubleLinkedListTrait<TI>> list1;
    list1.insert(6, 15);
    list1.insert(2, 25);
    list1.insert(9, 35);
    list1.insert(1, 45);
    list1.insert(7, 55);
    cout << "Lista descendente DLL: " << list1 << endl;

    DoubleLinkedList<AscendingDoubleLinkedListTrait<TI>> list2;
    list2.insert(6, 15);
    list2.insert(2, 25);
    list2.insert(9, 35);
    list2.insert(1, 45);
    list2.insert(7, 55);
    cout << "Lista ascendente DLL: " << list2 << endl;
}


void CLinkedListDemo() {
    cout << "\n=== CLinkedList (circular) ===\n";
    CLinkedList<AscendingLinkedListTrait<TI>> list;
    list.insert(6, 15);
    list.insert(2, 25);
    list.insert(9, 35);
    list.insert(1, 45);
    list.insert(7, 55);
    cout << "Lista circular ascendente: " << list << endl;

    // Probar push_front, pop_back, etc.
    list.push_front(0, 99);
    cout << "Después de push_front(0): " << list << endl;

    auto [val, ref] = list.pop_back();
    cout << "pop_back() devuelve (" << val << "," << ref << "), lista: " << list << endl;

    // Probar ForEach
    list.ForEach([](auto& node) { node.getDataRef() += 10; });
    cout << "Después de sumar 10 a cada elemento: " << list << endl;
}
void CDoubleLinkedListDemo() {
    cout << "\n=== CDoubleLinkedList (doble circular) ===\n";
    CDoubleLinkedList<AscendingDoubleLinkedListTrait<TI>> list;
    list.insert(6, 15);
    list.insert(2, 25);
    list.insert(9, 35);
    list.insert(1, 45);
    list.insert(7, 55);
    cout << "Lista doble circular ascendente: " << list << endl;

    list.push_front(0, 99);
    cout << "Después de push_front(0): " << list << endl;

    auto [val, ref] = list.pop_back();
    cout << "pop_back() devuelve (" << val << "," << ref << "), lista: " << list << endl;

    // Prueba de recorrido inverso
    cout << "Recorrido inverso (ReverseForEach): ";
    list.ReverseForEach(Print<decltype(*list.begin())>, cout);
    cout << endl;

    // Prueba de FirstThat hacia adelante y hacia atrás
    auto it = list.FirstThat(IsGreaterThan<decltype(*list.begin()), TI>, 5);
    if (it != list.end())
        cout << "Primer elemento mayor que 5: " << *it << endl;

    auto rit = list.ReverseFirstThat(IsGreaterThan<decltype(*list.begin()), TI>, 5);
    if (rit != list.rend())
        cout << "Primer elemento desde atrás mayor que 5: " << *rit << endl;

    list.ForEach([](auto& node) { node.getDataRef() += 10; });
    cout << "Después de sumar 10 a cada elemento: " << list << endl;
}

void HeapDemo() {
    cout << "\n=== Heap Demo ===\n";
    
    // Heap ascendente (mínimo)
    Heap<AscendingHeapTrait<TI>> minHeap;
    minHeap.insert(5, 100);
    minHeap.insert(3, 200);
    minHeap.insert(7, 300);
    minHeap.insert(1, 400);
    minHeap.insert(9, 500);
    
    cout << "Heap ascendente: " << minHeap << endl;
    cout << "Tamaño: " << minHeap.size() << endl;
    cout << "Mínimo (dato): " << minHeap.peek_min() << endl;
    
    cout << "Extrayendo todos: ";
    while (!minHeap.empty()) {
        auto node = minHeap.extract();
        cout << node.GetData() << "(" << node.GetRef() << ") ";
    }
    cout << endl;
    cout << "¿Vacío? " << (minHeap.empty() ? "Sí" : "No") << endl;
    
    // Heap descendente (máximo)
    Heap<DescendingHeapTrait<TI>> maxHeap;
    maxHeap.insert(5, 100);
    maxHeap.insert(3, 200);
    maxHeap.insert(7, 300);
    maxHeap.insert(1, 400);
    maxHeap.insert(9, 500);
    
    cout << "\nHeap descendente: " << maxHeap << endl;
    cout << "Máximo (dato): " << maxHeap.peek_min() << endl;
    cout << "Extrayendo todos: ";
    while (!maxHeap.empty()) {
        auto node = maxHeap.extract();
        cout << node.GetData() << "(" << node.GetRef() << ") ";
    }
    cout << endl;
}

void ListsDemo(){

    HeapDemo(); 
}


