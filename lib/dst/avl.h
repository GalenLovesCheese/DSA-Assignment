#include <cstddef>

template <typename T>
class AVLTree
{
private:
    struct Node
    {
        T data;
        Node *left;
        Node *right;
        int height;

        Node(const T &value) : data(value), left(nullptr), right(nullptr), height(1) {}
    };

    Node *root;
    size_t tree_size;

    int get_height(Node *node)
    {
        return node ? node->height : 0;
    }

    int get_balance(Node *node)
    {
        return node ? get_height(node->left) - get_height(node->right) : 0;
    }

    void update_height(Node *node)
    {
        if (node)
        {
            int left_height = get_height(node->left);
            int right_height = get_height(node->right);
            node->height = 1 + (left_height > right_height ? left_height : right_height);
        }
    }

    Node *rotate_right(Node *y)
    {
        if (!y || !y->left)
            return y;

        Node *x = y->left;
        Node *T2 = x->right;

        x->right = y;
        y->left = T2;

        update_height(y);
        update_height(x);

        return x;
    }

    Node *rotate_left(Node *x)
    {
        if (!x || !x->right)
            return x;

        Node *y = x->right;
        Node *T2 = y->left;

        y->left = x;
        x->right = T2;

        update_height(x);
        update_height(y);

        return y;
    }

    Node *insert_internal(Node *node, const T &value, bool &inserted)
    {
        if (!node)
        {
            inserted = true;
            return new Node(value);
        }

        // Avoid comparisons if values are equal
        if (!(value < node->data) && !(node->data < value))
        {
            inserted = false;
            return node; // Duplicate value
        }

        if (value < node->data)
        {
            node->left = insert_internal(node->left, value, inserted);
        }
        else
        {
            node->right = insert_internal(node->right, value, inserted);
        }

        if (!inserted)
            return node; // If no insertion happened, no need to balance

        update_height(node);

        int balance = get_balance(node);

        // Left Left Case
        if (balance > 1 && value < node->left->data)
        {
            return rotate_right(node);
        }

        // Right Right Case
        if (balance < -1 && node->right->data < value)
        {
            return rotate_left(node);
        }

        // Left Right Case
        if (balance > 1 && node->left->data < value)
        {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }

        // Right Left Case
        if (balance < -1 && value < node->right->data)
        {
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }

        return node;
    }

    Node *find_min(Node *node)
    {
        while (node && node->left)
        {
            node = node->left;
        }
        return node;
    }

    Node *delete_internal(Node *node, const T &value, bool &deleted)
    {
        if (!node)
        {
            deleted = false;
            return nullptr;
        }

        if (value < node->data)
        {
            node->left = delete_internal(node->left, value, deleted);
        }
        else if (node->data < value)
        {
            node->right = delete_internal(node->right, value, deleted);
        }
        else
        {
            deleted = true;
            if (!node->left || !node->right)
            {
                Node *temp = node->left ? node->left : node->right;
                if (!temp)
                {
                    temp = node;
                    node = nullptr;
                }
                else
                {
                    *node = *temp;
                }
                delete temp;
            }
            else
            {
                Node *temp = find_min(node->right);
                node->data = temp->data;
                node->right = delete_internal(node->right, temp->data, deleted);
            }
        }

        if (!node)
            return nullptr;
        if (!deleted)
            return node; // If no deletion happened, no need to balance

        update_height(node);

        int balance = get_balance(node);

        // Left Left Case
        if (balance > 1 && get_balance(node->left) >= 0)
        {
            return rotate_right(node);
        }

        // Left Right Case
        if (balance > 1 && get_balance(node->left) < 0)
        {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }

        // Right Right Case
        if (balance < -1 && get_balance(node->right) <= 0)
        {
            return rotate_left(node);
        }

        // Right Left Case
        if (balance < -1 && get_balance(node->right) > 0)
        {
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }

        return node;
    }

    void clear_internal(Node *node)
    {
        if (node)
        {
            clear_internal(node->left);
            clear_internal(node->right);
            delete node;
        }
    }

public:
    class Iterator
    {
    private:
        Node *current;
        Node **stack;
        int stack_size;
        int top;

        void push_left_branch(Node *node)
        {
            while (node)
            {
                stack[++top] = node;
                node = node->left;
            }
        }

    public:
        Iterator(Node *root, size_t tree_size) : current(nullptr),
                                                 stack(new Node *[tree_size + 1]), stack_size(tree_size), top(-1)
        {
            push_left_branch(root);
            if (top >= 0)
            {
                current = stack[top];
            }
        }

        ~Iterator()
        {
            delete[] stack;
        }

        bool has_next() const
        {
            return top >= 0;
        }

        T &operator*()
        {
            return current->data;
        }

        Iterator &operator++()
        {
            if (top >= 0)
            {
                Node *node = stack[top--];
                if (node->right)
                {
                    push_left_branch(node->right);
                }
                current = top >= 0 ? stack[top] : nullptr;
            }
            return *this;
        }
    };

    AVLTree() : root(nullptr), tree_size(0) {}

    ~AVLTree()
    {
        clear_internal(root);
    }

    // Copy constructor
    AVLTree(const AVLTree &other) : root(nullptr), tree_size(0)
    {
        root = copyTree(other.root);
        tree_size = other.tree_size;
    }

    // Assignment operator
    AVLTree &operator=(const AVLTree &other)
    {
        if (this != &other)
        {
            clear();
            root = copyTree(other.root);
            tree_size = other.tree_size;
        }
        return *this;
    }

    // Helper method to recursively copy nodes
    Node *copyTree(Node *node)
    {
        if (!node)
            return nullptr;
        Node *newNode = new Node(node->data);
        newNode->left = copyTree(node->left);
        newNode->right = copyTree(node->right);
        newNode->height = node->height;
        return newNode;
    }

    void insert(const T &value)
    {
        bool inserted = false;
        root = insert_internal(root, value, inserted);
        if (inserted)
            tree_size++;
    }

    void remove(const T &value)
    {
        bool deleted = false;
        root = delete_internal(root, value, deleted);
        if (deleted)
            tree_size--;
    }

    bool contains(const T &value) const
    {
        Node *current = root;
        while (current)
        {
            if (value < current->data)
            {
                current = current->left;
            }
            else if (current->data < value)
            {
                current = current->right;
            }
            else
            {
                return true;
            }
        }
        return false;
    }

    size_t size() const
    {
        return tree_size;
    }

    bool empty() const
    {
        return tree_size == 0;
    }

    void clear()
    {
        clear_internal(root);
        root = nullptr;
        tree_size = 0;
    }

    Iterator begin()
    {
        return Iterator(root, tree_size);
    }
};