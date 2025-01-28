#ifndef BPLUSTREE_H
#define BPLUSTREE_H

// Forward declaration to allow template specialization for comparisons
template <typename T>
class Compare
{
public:
    static bool less(const T &a, const T &b)
    {
        return a < b;
    }
    static bool equal(const T &a, const T &b)
    {
        return a == b;
    }
};

template <>
class Compare<const char *>
{
public:
    static bool less(const char *a, const char *b)
    {
        return strcmp(a, b) < 0;
    }
    static bool equal(const char *a, const char *b)
    {
        return strcmp(a, b) == 0;
    }
};

template <typename KeyType, typename ValueType, int ORDER = 8> // TODO: Change order size
class BPlusTree
{
private:
    // Maximum number of keys in a node
    static const int MAX_KEYS = ORDER - 1;
    // Minimum number of keys in a node
    static const int MIN_KEYS = (ORDER + 1) / 2 - 1;

    // Node structure for B+ Tree
    struct Node
    {
        bool is_leaf;
        int key_count;
        KeyType keys[MAX_KEYS + 1];      // +1 for easy splitting
        Node *children[ORDER + 1];       // One more than max keys
        ValueType *values[MAX_KEYS + 1]; // Only used in leaf nodes
        Node *next_leaf;                 // Pointer to next leaf for range traversal

        Node() : is_leaf(false), key_count(0), next_leaf(nullptr)
        {
            for (int i = 0; i < ORDER + 1; ++i)
            {
                children[i] = nullptr;
                if (i < MAX_KEYS + 1)
                {
                    values[i] = nullptr;
                }
            }
        }

        ~Node()
        {
            // Free values in leaf nodes
            if (is_leaf)
            {
                for (int i = 0; i < key_count; ++i)
                {
                    delete values[i];
                }
            }
        }
    };

    Node *root;

    // Helper methods
    Node *create_leaf_node()
    {
        Node *node = new Node();
        node->is_leaf = true;
        return node;
    }

    Node *create_internal_node()
    {
        Node *node = new Node();
        node->is_leaf = false;
        return node;
    }

    // Split a full node during insertion
    Node *split_node(Node *node)
    {
        int mid = node->key_count / 2;
        Node *new_node = node->is_leaf ? create_leaf_node() : create_internal_node();

        // Copy second half of keys and children/values
        for (int i = mid; i < node->key_count; ++i)
        {
            new_node->keys[new_node->key_count] = node->keys[i];

            if (node->is_leaf)
            {
                new_node->values[new_node->key_count] = node->values[i];
                node->values[i] = nullptr;
            }
            else
            {
                new_node->children[new_node->key_count] = node->children[i];
                node->children[i] = nullptr;
            }

            new_node->key_count++;
        }

        // For internal nodes, move the middle key up
        if (!node->is_leaf)
        {
            new_node->children[new_node->key_count] = node->children[node->key_count];
            node->children[node->key_count] = nullptr;
        }

        // Update original node's key count
        node->key_count = mid;

        // For leaf nodes, link leaves
        if (node->is_leaf)
        {
            new_node->next_leaf = node->next_leaf;
            node->next_leaf = new_node;
        }

        return new_node;
    }

    // Recursive insertion
    bool insert_non_full(Node *node, const KeyType &key, ValueType *value)
    {
        int i = node->key_count - 1;

        if (node->is_leaf)
        {
            // Find insertion point in leaf
            while (i >= 0 && Compare<KeyType>::less(key, node->keys[i]))
            {
                node->keys[i + 1] = node->keys[i];
                node->values[i + 1] = node->values[i];
                i--;
            }

            // Insert new key and value
            node->keys[i + 1] = key;
            node->values[i + 1] = value;
            node->key_count++;
            return true;
        }

        // Find child to recurse into
        while (i >= 0 && Compare<KeyType>::less(key, node->keys[i]))
        {
            i--;
        }
        i++;

        // If child is full, split it
        if (node->children[i]->key_count == MAX_KEYS)
        {
            Node *split_child = split_node(node->children[i]);

            // Insert middle key into parent
            for (int j = node->key_count; j > i; j--)
            {
                node->keys[j] = node->keys[j - 1];
                node->children[j + 1] = node->children[j];
            }

            node->keys[i] = split_child->keys[0];
            node->children[i + 1] = split_child;
            node->key_count++;

            // Decide which child to recurse into
            if (Compare<KeyType>::less(split_child->keys[0], key))
            {
                i++;
            }
        }

        return insert_non_full(node->children[i], key, value);
    }

    // Recursive search
    ValueType *search_recursive(Node *node, const KeyType &key) const
    {
        if (node->is_leaf)
        {
            for (int i = 0; i < node->key_count; ++i)
            {
                if (Compare<KeyType>::equal(node->keys[i], key))
                {
                    return node->values[i];
                }
            }
            return nullptr;
        }

        // Find appropriate child
        int i = 0;
        while (i < node->key_count && (Compare<KeyType>::less(node->keys[i], key) || Compare<KeyType>::equal(node->keys[i], key)))
        {
            i++;
        }

        return search_recursive(node->children[i], key);
    }

    // Free all nodes recursively
    void destroy_tree(Node *node)
    {
        if (node == nullptr)
            return;

        if (!node->is_leaf)
        {
            for (int i = 0; i <= node->key_count; ++i)
            {
                destroy_tree(node->children[i]);
            }
        }
        delete node;
    }

public:
    BPlusTree() : root(create_leaf_node()) {}

    ~BPlusTree()
    {
        destroy_tree(root);
    }

    // Insert a key-value pair
    void insert(const KeyType &key, const ValueType &value)
    {
        ValueType *value_ptr = new ValueType(value);

        // If root is full, create new root
        if (root->key_count == MAX_KEYS)
        {
            Node *new_root = create_internal_node();
            Node *old_root = root;

            root = new_root;
            new_root->children[0] = old_root;

            // Split the old root
            Node *split_child = split_node(old_root);
            new_root->keys[0] = split_child->keys[0];
            new_root->children[1] = split_child;
            new_root->key_count = 1;
        }

        insert_non_full(root, key, value_ptr);
    }

    // Search for a value by key
    ValueType *search(const KeyType &key) const
    {
        return search_recursive(root, key);
    }

    // Range query iterator
    class RangeIterator
    {
    private:
        typename BPlusTree<KeyType, ValueType, ORDER>::Node *current_leaf;
        int current_index;
        KeyType end_key;

    public:
        RangeIterator(typename BPlusTree<KeyType, ValueType, ORDER>::Node *leaf,
                      int index, const KeyType &end)
            : current_leaf(leaf), current_index(index), end_key(end) {}

        // Check if iterator is valid
        bool has_next() const
        {
            return current_leaf != nullptr &&
                   current_index < current_leaf->key_count &&
                   !Compare<KeyType>::less(end_key, current_leaf->keys[current_index]);
        }

        // Get current value and advance
        ValueType *next()
        {
            if (!has_next())
                return nullptr;

            ValueType *result = current_leaf->values[current_index];
            current_index++;

            // Move to next leaf if needed
            if (current_index >= current_leaf->key_count)
            {
                current_leaf = current_leaf->next_leaf;
                current_index = 0;
            }

            return result;
        }
    };

    // Range query method
    RangeIterator range_query(const KeyType &start, const KeyType &end) const
    {
        // Find the first leaf node that might contain start key
        Node *current = root;
        while (!current->is_leaf)
        {
            int i = 0;
            while (i < current->key_count &&
                   Compare<KeyType>::less(current->keys[i], start))
            {
                i++;
            }
            current = current->children[i];
        }

        // Find first key >= start
        int start_index = 0;
        while (start_index < current->key_count &&
               Compare<KeyType>::less(current->keys[start_index], start))
        {
            start_index++;
        }

        // Return iterator at first valid point
        return RangeIterator(current, start_index, end);
    }
};

#endif // BPLUSTREE_H