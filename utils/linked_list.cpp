// #include "linked_list.hpp"

// // --- Constructors & destructor ---
// template <typename T>
// linked_list<T>::linked_list() {
//     head = nullptr;
//     tail = nullptr;
//     sz = 0;
// }

// template <typename T>
// linked_list<T>::~linked_list() {
//     while (!empty()) {
//         pop_back();
//     }
// }

// // --- Modifiers ---
// template <typename T>
// void linked_list<T>::push_back(const T& value) {
//     Node* node = new Node(value);
//     if (!head) {
//         head = tail = node;
//     } else {
//         tail->next = node;
//         node->prev = tail;
//         tail = node;
//     }
//     sz++;
// }

// template <typename T>
// void linked_list<T>::push_front(const T& value) {
//     Node* node = new Node(value);
//     if (!head) {
//         head = tail = node;
//     } else {
//         node->next = head;
//         head->prev = node;
//         head = node;
//     }
//     sz++;
// }

// template <typename T>
// void linked_list<T>::pop_back() {
//     if (!tail) throw std::out_of_range("List is empty");
//     Node* node = tail;
//     tail = tail->prev;
//     if (tail) tail->next = nullptr;
//     else head = nullptr;
//     delete node;
//     sz--;
// }

// template <typename T>
// void linked_list<T>::pop_front() {
//     if (!head) throw std::out_of_range("List is empty");
//     Node* node = head;
//     head = head->next;
//     if (head) head->prev = nullptr;
//     else tail = nullptr;
//     delete node;
//     sz--;
// }

// // --- Insert ---
// template <typename T>
// typename linked_list<T>::iterator linked_list<T>::insert(iterator pos, const T& value) {
//     if (pos.current == nullptr) {  // insert at end
//         push_back(value);
//         return Iterator(tail);
//     }

//     Node* node = new Node(value);
//     Node* current = pos.current;

//     node->next = current;
//     node->prev = current->prev;

//     if (current->prev) {
//         current->prev->next = node;
//     } else {
//         head = node;  // inserting at the beginning
//     }

//     current->prev = node;
//     sz++;
//     return Iterator(node);
// }

// // --- Erase ---
// template <typename T>
// typename linked_list<T>::iterator linked_list<T>::erase(iterator pos) {
//     if (!pos.current) throw std::out_of_range("Invalid iterator");

//     Node* current = pos.current;
//     Node* nextNode = current->next;

//     if (current->prev) {
//         current->prev->next = current->next;
//     } else {
//         head = current->next;
//     }

//     if (current->next) {
//         current->next->prev = current->prev;
//     } else {
//         tail = current->prev;
//     }

//     sz--;

//     return Iterator(nextNode);
// }

// // --- Access ---
// template <typename T>
// T& linked_list<T>::front() {
//     if (!head) throw std::out_of_range("List is empty");
//     return head->data;
// }

// template <typename T>
// T& linked_list<T>::back() {
//     if (!tail) throw std::out_of_range("List is empty");
//     return tail->data;
// }

// // --- Capacity ---
// template <typename T>
// std::size_t linked_list<T>::size() const {
//     return sz;
// }

// template <typename T>
// bool linked_list<T>::empty() const {
//     return sz == 0;
// }
