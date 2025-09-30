#include <cstddef>
#include <stdexcept>

// A minimal doubly linked list template
template <typename T>
class linked_list
{
private:
    

public:
    // --- Iterator ---

    struct Node
    {
        T data;
        Node *prev;
        Node *next;
        Node(const T &val) : data(val), prev(nullptr), next(nullptr) {}
    };

    Node *head;
    Node *tail;
    std::size_t sz;

    class iterator
    {
    private:
    public:
        Node *current;
        // iterator(Node *ptr = nullptr) : current(ptr) {}
        iterator(Node *node) : current(node){}

        Node* getCurrent(){
            return current;
        }
        T &operator*() { return current->data; }
        T *operator->() { return &(current->data); }

        iterator &operator++()
        { // pre-increment
            if (current)
                current = current->next;
            else 
                std::out_of_range("Iterator is not valid");
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
                std::out_of_range("Iterator is not valid");
            else
                current = current->prev;
            return *this;
        }
        iterator operator--(int)
        { // post-increment
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
    vector<T> export_as_vector();
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
linked_list<T>::~linked_list()
{
    while (!empty())
    {
        pop_back();
    }
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
    // delete current;

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
vector<T> linked_list<T>::export_as_vector()
{
    vector<T> result;
    result.reserve(sz);
    for(auto itr = this->begin(); itr != this->end(); itr++){
        result.push_back(*itr);
    }
    return result;
}