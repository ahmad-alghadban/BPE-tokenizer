#ifndef LINKED_LIST_HPP
#define LINKED_LIST_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>
#include <utility>

// A minimal doubly linked list template
template <typename T>
class linked_list
{
private:
    struct Node
    {
        T data;
        Node *prev;
        Node *next;
        bool deleted; // marked true by erase(); node is kept alive until the list dies
        Node(const T &val) : data(val), prev(nullptr), next(nullptr), deleted(false) {}
    };

    Node *head;
    Node *tail;
    std::size_t sz;
    // Erased nodes are unlinked but kept alive (stale iterators elsewhere may still
    // read their `deleted` flag), then freed together when the list is destroyed.
    std::vector<Node *> graveyard;

public:
    // --- Iterator ---
    class iterator
    {
    private:
        Node *current;

    public:
        iterator(Node *node) : current(node) {}

        // True if this node was removed via erase() but is still retained (see graveyard).
        bool is_deleted() const { return current->deleted; }

        T &operator*() { return current->data; }
        T *operator->() { return &(current->data); }

        iterator &operator++()
        { // pre-increment
            if (current)
                current = current->next;
            else
                throw std::out_of_range("Iterator is not valid");
            return *this;
        }
        iterator operator++(int)
        { // post-increment
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        iterator &operator--()
        {
            if (!current)
                throw std::out_of_range("Iterator is not valid");
            else
                current = current->prev;
            return *this;
        }
        iterator operator--(int)
        { // post-decrement
            iterator temp = *this;
            --(*this);
            return temp;
        }

        const T &operator*() const { return current->data; } // for const iterator

        bool operator==(const iterator &other) const { return current == other.current; }
        bool operator!=(const iterator &other) const { return current != other.current; }

        friend class linked_list;
    };

    // --- Constructors & destructor ---
    linked_list();
    linked_list(const linked_list &other);
    // noexcept move ctor: lets std::vector relocate without copying, keeping node
    // addresses stable so stored Node* iterators stay valid across reallocation.
    linked_list(linked_list &&other) noexcept;
    // Unified copy/move assignment via copy-and-swap: `other` is built by copy or
    // move, then swapped in; the old resources leave with `other`'s destructor.
    linked_list &operator=(linked_list other);
    void swap(linked_list &other) noexcept;
    ~linked_list();

    // --- Modifiers ---
    void push_back(const T &value);
    void push_front(const T &value);
    void pop_back();
    void pop_front();

    iterator insert(iterator pos, const T &value);
    iterator erase(iterator pos);

    // --- Access ---
    T &front();
    T &back();

    // --- Capacity ---
    std::size_t size() const;
    bool empty() const;
    iterator back_iter();

    // --- Iterators ---
    iterator begin() { return iterator(head); }
    iterator end() { return iterator(nullptr); }

    // --- usage ---
    std::vector<T> export_as_vector();
};

// --- Constructors & destructor ---
template <typename T>
linked_list<T>::linked_list()
{
    head = nullptr;
    tail = nullptr;
    sz = 0;
}

template <typename T>
linked_list<T>::linked_list(const linked_list &other)
{
    head = tail = nullptr;
    sz = 0;
    for (Node *cur = other.head; cur != nullptr; cur = cur->next)
    {
        push_back(cur->data);
    }
}

template <typename T>
linked_list<T>::linked_list(linked_list &&other) noexcept
    : head(other.head), tail(other.tail), sz(other.sz), graveyard(std::move(other.graveyard))
{
    // Steal the nodes; leave `other` empty so its destructor frees nothing.
    other.head = nullptr;
    other.tail = nullptr;
    other.sz = 0;
}

template <typename T>
void linked_list<T>::swap(linked_list &other) noexcept
{
    std::swap(head, other.head);
    std::swap(tail, other.tail);
    std::swap(sz, other.sz);
    graveyard.swap(other.graveyard);
}

template <typename T>
linked_list<T> &linked_list<T>::operator=(linked_list other)
{
    swap(other);
    return *this;
}

template <typename T>
linked_list<T>::~linked_list()
{
    while (!empty())
    {
        pop_back();
    }
    // Reclaim the erased-but-retained nodes.
    for (Node *dead : graveyard)
        delete dead;
    graveyard.clear();
}

// --- Modifiers ---
template <typename T>
void linked_list<T>::push_back(const T &value)
{
    Node *node = new Node(value);
    if (!head)
    {
        head = tail = node;
    }
    else
    {
        tail->next = node;
        node->prev = tail;
        tail = node;
    }
    sz++;
}

template <typename T>
void linked_list<T>::push_front(const T &value)
{
    Node *node = new Node(value);
    if (!head)
    {
        head = tail = node;
    }
    else
    {
        node->next = head;
        head->prev = node;
        head = node;
    }
    sz++;
}

template <typename T>
void linked_list<T>::pop_back()
{
    if (!tail)
        throw std::out_of_range("List is empty");
    Node *node = tail;
    tail = tail->prev;
    if (tail)
        tail->next = nullptr;
    else
        head = nullptr;
    delete node;
    sz--;
}

template <typename T>
void linked_list<T>::pop_front()
{
    if (!head)
        throw std::out_of_range("List is empty");
    Node *node = head;
    head = head->next;
    if (head)
        head->prev = nullptr;
    else
        tail = nullptr;
    delete node;
    sz--;
}

// --- Insert ---
template <typename T>
typename linked_list<T>::iterator linked_list<T>::insert(iterator pos, const T &value)
{
    if (pos.current == nullptr)
    { // insert at end
        push_back(value);
        return iterator(tail);
    }

    Node *node = new Node(value);
    Node *current = pos.current;

    node->next = current;
    node->prev = current->prev;

    if (current->prev)
    {
        current->prev->next = node;
    }
    else
    {
        head = node; // inserting at the beginning
    }

    current->prev = node;
    sz++;
    return iterator(node);
}

// --- Erase ---
template <typename T>
typename linked_list<T>::iterator linked_list<T>::erase(iterator pos)
{
    if (!pos.current)
        throw std::out_of_range("Invalid iterator");

    Node *current = pos.current;
    Node *nextNode = current->next;

    if (current->prev)
    {
        current->prev->next = current->next;
    }
    else
    {
        head = current->next;
    }

    if (current->next)
    {
        current->next->prev = current->prev;
    }
    else
    {
        tail = current->prev;
    }

    sz--;
    current->deleted = true;
    graveyard.push_back(current); // freed later in the destructor

    return iterator(nextNode);
}

// --- Access ---
template <typename T>
T &linked_list<T>::front()
{
    if (!head)
        throw std::out_of_range("List is empty");
    return head->data;
}

template <typename T>
T &linked_list<T>::back()
{
    if (!tail)
        throw std::out_of_range("List is empty");
    return tail->data;
}

template <typename T>
typename linked_list<T>::iterator linked_list<T>::back_iter() {
    if (!tail)
        throw std::out_of_range("List is empty");
    return iterator(tail);
}

// --- Capacity ---
template <typename T>
std::size_t linked_list<T>::size() const
{
    return sz;
}

template <typename T>
bool linked_list<T>::empty() const
{
    return sz == 0;
}

// --- usage ---
template <typename T>
std::vector<T> linked_list<T>::export_as_vector()
{
    std::vector<T> result;
    result.reserve(sz);
    for (auto itr = this->begin(); itr != this->end(); itr++)
    {
        result.push_back(*itr);
    }
    return result;
}

#endif // LINKED_LIST_HPP