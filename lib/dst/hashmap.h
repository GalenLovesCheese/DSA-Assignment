#ifndef HASHMAP_H
#define HASHMAP_H

#include "dst/linkedlist.h"
#include <cstdint>

template <typename K, typename V>
class HashMap
{
private:
    // Entry struct to store key-value pairs in linked lists
    struct Entry
    {
        K key;
        V value;
        Entry(const K &k, const V &v) : key(k), value(v) {}

        bool operator==(const Entry &other) const {
            return key == other.key;
        }
    };

    // Array of linked lists for collision resolution
    LinkedList<Entry> *table;
    int capacity;
    int size;

    // Hash function
    unsigned int hash(const K &key) const
    {
        return hashCode(key) % capacity;
    }

    // Helper function to resize the hash map
    void resize()
    {
        int oldCapacity = capacity;
        capacity *= 2;
        LinkedList<Entry> *oldTable = table;

        // Allocate new table
        table = new LinkedList<Entry>[capacity];

        // Rehash existing entries
        size = 0;
        for (int i = 0; i < oldCapacity; ++i)
        {
            for (auto &entry : oldTable[i])
            {
                insert(entry.key, entry.value);
            }
        }

        delete[] oldTable;
    }

    // Fallback hash function for generic types
    template <typename T>
    unsigned int hashCode(const T &key) const
    {
        return static_cast<unsigned int>(reinterpret_cast<uintptr_t>(&key));
    }

    // Specialized hash functions for primitive types
    unsigned int hashCode(int key) const { return static_cast<unsigned int>(key); }
    unsigned int hashCode(unsigned int key) const { return key; }
    unsigned int hashCode(long key) const { return static_cast<unsigned int>(key); }
    unsigned int hashCode(const char *key) const
    {
        unsigned int hash = 0;
        while (*key)
        {
            hash = hash * 31 + *key;
            ++key;
        }
        return hash;
    }
    unsigned int hashCode(const std::string &key) const
    {
        return hashCode(key.c_str());
    }

public:
    // Constructor
    HashMap(int initialCapacity = 16) : capacity(initialCapacity), size(0)
    {
        table = new LinkedList<Entry>[capacity];
    }

    // Destructor
    ~HashMap()
    {
        delete[] table;
    }

    // Insert or update a key-value pair
    void insert(const K &key, const V &value)
    {
        // Resize if load factor exceeds 0.75
        if (static_cast<double>(size) / capacity >= 0.75)
        {
            resize();
        }

        unsigned int index = hash(key);

        // Check if key already exists in the list
        for (auto &entry : table[index])
        {
            if (entry.key == key)
            {
                entry.value = value;
                return;
            }
        }

        // If key doesn't exist, add new entry
        table[index].push_back(Entry(key, value));
        ++size;
    }

    // Retrieve a value by key
    V *get(const K &key)
    {
        unsigned int index = hash(key);

        // Search through the linked list at this index
        for (auto &entry : table[index])
        {
            if (entry.key == key)
            {
                return &entry.value;
            }
        }

        return nullptr;
    }
    // Remove a key-value pair
    bool remove(const K &key)
    {
        unsigned int index = hash(key);
    
        // Search through the linked list at this index
        for (auto &entry : table[index])
        {
            if (entry.key == key)
            {
                // Use the LinkedList's remove method directly with the entry
                table[index].remove(entry);
                --size;
                return true;
            }
        }
    
        return false;
    }

    // Get current size of the hash map
    int getSize() const
    {
        return size;
    }

    // Check if hash map is empty
    bool isEmpty() const
    {
        return size == 0;
    }
};

#endif // HASHMAP_H