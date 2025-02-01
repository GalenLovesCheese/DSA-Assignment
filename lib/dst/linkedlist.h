#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdexcept>

template <typename T>
class LinkedList
{
private:
    // Node structure for the linked list
    struct Node
    {
        T data;
        Node *next;

        // Constructor to initialize node
        Node(const T &value) : data(value), next(nullptr) {}
    };

    Node *head;
    Node *tail;
    int size;

public:
    // Constructor
    LinkedList() : head(nullptr), tail(nullptr), size(0) {}

    // Destructor to free memory
    ~LinkedList()
    {
        clear();
    }

    // Copy constructor
    LinkedList(const LinkedList &other) : head(nullptr), tail(nullptr), size(0)
    {
        Node *current = other.head;
        while (current)
        {
            push_back(current->data);
            current = current->next;
        }
    }

    // Copy assignment operator
    LinkedList &operator=(const LinkedList &other)
    {
        if (this != &other)
        {
            // Clear existing list
            clear();

            // Copy elements from other list
            Node *current = other.head;
            while (current)
            {
                push_back(current->data);
                current = current->next;
            }
        }
        return *this;
    }

    // Add element to the end of the list
    void push_back(const T &value)
    {
        Node *newNode = new Node(value);

        if (!head)
        {
            head = tail = newNode;
        }
        else
        {
            tail->next = newNode;
            tail = newNode;
        }

        ++size;
    }

    // Add element to the beginning of the list
    void push_front(const T &value)
    {
        Node *newNode = new Node(value);

        if (!head)
        {
            head = tail = newNode;
        }
        else
        {
            newNode->next = head;
            head = newNode;
        }

        ++size;
    }

    // Remove first element
    void pop_front()
    {
        if (!head)
            return;

        Node *temp = head;
        head = head->next;
        delete temp;

        --size;

        // If list becomes empty
        if (!head)
        {
            tail = nullptr;
        }
    }

    bool contain(T &target){
        Node* current = head;
        while(current){
            if(current->data == target){
                return true;
            }
            current = current->next;
        }
        return false;
    }

    bool remove(T target){
        if (!head) {
            return false;
        }

        if (head->data == target) {
            Node* temp = head;
            head = head->next;
            delete temp;
            if (!head) {
                tail = nullptr;
            }
            --size;
            return true;
        }

        Node* current = head;
        while (current->next) {
            if (current->next->data == target) {
                Node* toDelete = current->next;
                current->next = toDelete->next;
                if (toDelete == tail) {
                    tail = current;
                }
                delete toDelete;
                --size;
                return true;
            }
            current = current->next;
        }

        return false;
    }

    // Get first element
    T &front()
    {
        if (!head)
        {
            throw std::runtime_error("List is empty");
        }
        return head->data;
    }

    // Get last element
    T &back()
    {
        if (!tail)
        {
            throw std::runtime_error("List is empty");
        }
        return tail->data;
    }

    // Clear entire list
    void clear()
    {
        while (head)
        {
            Node *temp = head;
            head = head->next;
            delete temp;
        }
        head = tail = nullptr;
        size = 0;
    }

    // Check if list is empty
    bool empty() const
    {
        return size == 0;
    }

    // Get current size of list
    int getSize() const
    {
        return size;
    }

    // Iterator support for range-based for loops
    class Iterator
    {
    private:
        Node *current;

    public:
        Iterator(Node *node) : current(node) {}

        T &operator*()
        {
            return current->data;
        }

        Iterator &operator++()
        {
            current = current->next;
            return *this;
        }

        bool operator!=(const Iterator &other)
        {
            return current != other.current;
        }
    };

    // Begin iterator
    Iterator begin()
    {
        return Iterator(head);
    }

    // End iterator
    Iterator end()
    {
        return Iterator(nullptr);
    }
};

#endif // LINKED_LIST_H