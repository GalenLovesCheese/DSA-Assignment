#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <cstring>

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

template <typename KeyType, typename ValueType, int ORDER = 4096>
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

    struct PathEntry {
        Node* parent;
        int index;
        PathEntry* next;
        PathEntry(Node* p, int i, PathEntry* n) : parent(p), index(i), next(n) {}
    };

    void borrow_from_left_leaf(Node* node, Node* left_sibling, Node* parent, int parent_key_index) {
        // Move the last element of left_sibling to the front of node
        node->key_count++;
        for (int i = node->key_count - 1; i > 0; --i) {
            node->keys[i] = node->keys[i - 1];
            node->values[i] = node->values[i - 1];
        }
        node->keys[0] = left_sibling->keys[left_sibling->key_count - 1];
        node->values[0] = left_sibling->values[left_sibling->key_count - 1];
        left_sibling->key_count--;
        parent->keys[parent_key_index] = node->keys[0];
    }

    void borrow_from_right_leaf(Node* node, Node* right_sibling, Node* parent, int parent_key_index) {
        // Move the first element of right_sibling to the end of node
        node->keys[node->key_count] = right_sibling->keys[0];
        node->values[node->key_count] = right_sibling->values[0];
        node->key_count++;
        // Shift remaining elements in right_sibling
        for (int i = 0; i < right_sibling->key_count - 1; ++i) {
            right_sibling->keys[i] = right_sibling->keys[i + 1];
            right_sibling->values[i] = right_sibling->values[i + 1];
        }
        right_sibling->key_count--;
        parent->keys[parent_key_index] = right_sibling->keys[0];
    }

    void borrow_from_left_internal(Node* node, Node* left_sibling, Node* parent, int parent_key_index) {
        // Take the last key from left_sibling and parent's key
        node->key_count++;
        for (int i = node->key_count - 1; i > 0; --i) {
            node->keys[i] = node->keys[i - 1];
        }
        node->children[node->key_count] = node->children[node->key_count - 1];
        for (int i = node->key_count - 1; i > 0; --i) {
            node->children[i] = node->children[i - 1];
        }
        node->keys[0] = parent->keys[parent_key_index];
        node->children[0] = left_sibling->children[left_sibling->key_count];
        parent->keys[parent_key_index] = left_sibling->keys[left_sibling->key_count - 1];
        left_sibling->key_count--;
    }

    void borrow_from_right_internal(Node* node, Node* right_sibling, Node* parent, int parent_key_index) {
        // Take the first key from right_sibling and parent's key
        node->keys[node->key_count] = parent->keys[parent_key_index];
        node->children[node->key_count + 1] = right_sibling->children[0];
        node->key_count++;
        parent->keys[parent_key_index] = right_sibling->keys[0];
        // Shift remaining elements in right_sibling
        for (int i = 0; i < right_sibling->key_count - 1; ++i) {
            right_sibling->keys[i] = right_sibling->keys[i + 1];
        }
        for (int i = 0; i < right_sibling->key_count; ++i) {
            right_sibling->children[i] = right_sibling->children[i + 1];
        }
        right_sibling->key_count--;
    }

    void merge_leaves(Node* left, Node* right, Node* parent, int parent_key_index) {
        // Copy all keys and values from right to left
        for (int i = 0; i < right->key_count; ++i) {
            left->keys[left->key_count + i] = right->keys[i];
            left->values[left->key_count + i] = right->values[i];
        }
        left->key_count += right->key_count;
        left->next_leaf = right->next_leaf;
        delete right;
        // Parent's key at parent_key_index is now redundant
    }

    void merge_internal_nodes(Node* left, Node* right, Node* parent, int parent_key_index) {
        // Bring down the parent's key
        left->keys[left->key_count] = parent->keys[parent_key_index];
        left->key_count++;
        // Copy keys and children from right to left
        for (int i = 0; i < right->key_count; ++i) {
            left->keys[left->key_count + i] = right->keys[i];
            left->children[left->key_count + i] = right->children[i];
        }
        left->children[left->key_count + right->key_count] = right->children[right->key_count];
        left->key_count += right->key_count;
        delete right;
    }

    void handle_underflow(Node* node, PathEntry*& stack) {
        while (node->key_count < MIN_KEYS) {
            if (stack == nullptr) {
                // Handle root underflow
                if (node->key_count == 0 && !node->is_leaf) {
                    root = node->children[0];
                    delete node;
                }
                break;
            }

            Node* parent = stack->parent;
            int index = stack->index;
            PathEntry* old_entry = stack;
            stack = stack->next;
            delete old_entry;

            Node* left_sibling = (index > 0) ? parent->children[index - 1] : nullptr;
            Node* right_sibling = (index < parent->key_count) ? parent->children[index + 1] : nullptr;

            // Try to borrow from left sibling
            if (left_sibling && left_sibling->key_count > MIN_KEYS) {
                if (node->is_leaf) {
                    borrow_from_left_leaf(node, left_sibling, parent, index - 1);
                } else {
                    borrow_from_left_internal(node, left_sibling, parent, index - 1);
                }
                break;
            }
            // Try to borrow from right sibling
            else if (right_sibling && right_sibling->key_count > MIN_KEYS) {
                if (node->is_leaf) {
                    borrow_from_right_leaf(node, right_sibling, parent, index);
                } else {
                    borrow_from_right_internal(node, right_sibling, parent, index);
                }
                break;
            }
            // Merge with sibling
            else {
                Node* merged_node;
                if (left_sibling) {
                    if (node->is_leaf) {
                        merge_leaves(left_sibling, node, parent, index - 1);
                    } else {
                        merge_internal_nodes(left_sibling, node, parent, index - 1);
                    }
                    merged_node = left_sibling;
                    // Remove the parent's key at index - 1
                    for (int i = index - 1; i < parent->key_count - 1; ++i) {
                        parent->keys[i] = parent->keys[i + 1];
                    }
                    for (int i = index; i < parent->key_count; ++i) {
                        parent->children[i] = parent->children[i + 1];
                    }
                } else {
                    if (node->is_leaf) {
                        merge_leaves(node, right_sibling, parent, index);
                    } else {
                        merge_internal_nodes(node, right_sibling, parent, index);
                    }
                    merged_node = node;
                    // Remove the parent's key at index
                    for (int i = index; i < parent->key_count - 1; ++i) {
                        parent->keys[i] = parent->keys[i + 1];
                    }
                    for (int i = index + 1; i < parent->key_count; ++i) {
                        parent->children[i] = parent->children[i + 1];
                    }
                }
                parent->key_count--;

                if (parent->key_count >= MIN_KEYS || parent == root) {
                    if (parent == root && parent->key_count == 0) {
                        root = merged_node;
                        delete parent;
                    }
                    break;
                } else {
                    node = parent;
                }
            }
        }
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

    // Remove a key-value pair
    bool remove(const KeyType& key) {
        PathEntry* stack = nullptr;
        Node* current = root;
        Node* parent = nullptr;
        int index_in_parent = -1;

        // Traverse to the leaf node
        while (!current->is_leaf) {
            int i = 0;
            while (i < current->key_count && Compare<KeyType>::less(current->keys[i], key)) {
                i++;
            }
            stack = new PathEntry(parent, index_in_parent, stack);
            parent = current;
            index_in_parent = i;
            current = current->children[i];
        }

        // Find the key in the leaf node
        int pos = -1;
        for (int i = 0; i < current->key_count; ++i) {
            if (Compare<KeyType>::equal(current->keys[i], key)) {
                pos = i;
                break;
            }
        }
        if (pos == -1) {
            // Cleanup stack
            while (stack) {
                PathEntry* temp = stack;
                stack = stack->next;
                delete temp;
            }
            return false;
        }

        // Delete the key from the leaf
        delete current->values[pos];
        for (int i = pos; i < current->key_count - 1; ++i) {
            current->keys[i] = current->keys[i + 1];
            current->values[i] = current->values[i + 1];
        }
        current->key_count--;

        if (current == root) {
            if (current->key_count == 0) {
                delete root;
                root = create_leaf_node();
            }
            // Cleanup stack
            while (stack) {
                PathEntry* temp = stack;
                stack = stack->next;
                delete temp;
            }
            return true;
        }

        // Check if underflow occurred
        if (current->key_count < MIN_KEYS) {
            stack = new PathEntry(parent, index_in_parent, stack);
            handle_underflow(current, stack);
        }

        // Cleanup remaining stack
        while (stack) {
            PathEntry* temp = stack;
            stack = stack->next;
            delete temp;
        }

        return true;
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

    // Bulk load sorted keys and values
    void bulk_load(const KeyType *keys, const ValueType *values, size_t count)
    {
        // Clear existing tree
        destroy_tree(root);
        root = create_leaf_node();
        Node *current_leaf = root;

        // Fill the leaves
        for (size_t i = 0; i < count; ++i)
        {
            if (current_leaf->key_count == MAX_KEYS)
            {
                Node *new_leaf = create_leaf_node();
                current_leaf->next_leaf = new_leaf;
                current_leaf = new_leaf;
            }
            current_leaf->keys[current_leaf->key_count] = keys[i];
            current_leaf->values[current_leaf->key_count] = new ValueType(values[i]);
            current_leaf->key_count++;
        }

        // Collect all leaves into an array
        size_t num_leaves = 0;
        Node *leaf = root;
        while (leaf != nullptr)
        {
            num_leaves++;
            leaf = leaf->next_leaf;
        }

        Node **leaves = new Node *[num_leaves];
        leaf = root;
        for (size_t i = 0; i < num_leaves; ++i)
        {
            leaves[i] = leaf;
            leaf = leaf->next_leaf;
        }

        // Build internal levels
        size_t current_level_count = num_leaves;
        Node **current_level = leaves;

        while (current_level_count > 1)
        {
            size_t parent_count = (current_level_count + ORDER - 1) / ORDER;
            Node **parents = new Node *[parent_count];
            size_t parent_idx = 0;

            for (size_t i = 0; i < current_level_count;)
            {
                Node *parent = create_internal_node();
                int child_count = 0;

                while (child_count < ORDER && i < current_level_count)
                {
                    parent->children[child_count] = current_level[i];
                    if (child_count > 0)
                    {
                        parent->keys[child_count - 1] = current_level[i]->keys[0];
                    }
                    child_count++;
                    i++;
                }

                parent->key_count = child_count - 1;
                parents[parent_idx++] = parent;
            }

            delete[] current_level;
            current_level = parents;
            current_level_count = parent_idx;
        }

        root = current_level[0];
    }
};

#endif // BPLUSTREE_H