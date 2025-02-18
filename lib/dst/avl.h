#include <iostream>

// Template class for AVLNode
template <typename T>
class AVLTree
{
private:
    struct Node
    {
        T key;       // value of AVLNode
        Node *left;  // pointer to left node
        Node *right; // pointer to right node
        int height;
    };
    // pointer to track root node of AVL tree
    Node *root;

    // get height of node
    int height(Node *node)
    {
        if (!node)
        {
            return 0;
        }
        else
        {
            return node->height;
        }
    }

    // obtain balance factor of node
    int getBalance(Node *node)
    {
        if (!node)
        {
            return 0;
        }
        return height(node->left) - height(node->right); // BF = LN - RN
    }

    // rotate right
    Node *rightRotate(Node *current)
    {
        Node *newRoot = current->left;  // new root node
        current->left = newRoot->right; // move previous left->left node to be new root's left node
        newRoot->right = current;       // assign new right node to be previous root node

        // update heights
        current->height = 1 + std::max(height(current->left), height(current->right));
        newRoot->height = 1 + std::max(height(newRoot->left), height(newRoot->right));

        return newRoot;
    }

    // rotate left
    Node *leftRotate(Node *current)
    {
        Node *newRoot = current->right;
        current->right = newRoot->left;
        newRoot->left = current;

        // update heights
        current->height = 1 + std::max(height(current->left), height(current->right));
        newRoot->height = 1 + std::max(height(newRoot->left), height(newRoot->right));

        return newRoot;
    }

    bool search(Node *root, T key)
    {
        if (!root)
        {
            return false;
        }
        if (root->key == key)
        {
            return true;
        }
        if (key < root->key)
        {
            return search(root->left, key);
        }
        return search(root->right, key);
    }

    // function to insert new key
    Node *insert(Node *current, T key)
    {
        // accept only unique keys
        if (search(current, key))
        {
            return current;
        }
        if (!current)
        {
            Node *newNode = new Node;
            newNode->key = key;
            newNode->left = newNode->right = nullptr;
            newNode->height = 1;
            return newNode;
        }

        if (key < current->key)
        {
            current->left = insert(current->left, key);
        }
        else if (key > current->key)
        {
            current->right = insert(current->right, key);
        }
        else
        {
            return current; // Duplicate keys are not allowed
        }

        current->height = 1 + std::max(height(current->left), height(current->right));
        int balanceFactor = getBalance(current);

        // left-left-case
        if (balanceFactor > 1 && key < current->left->key)
        {
            return rightRotate(current);
        }
        // right-right-case
        if (balanceFactor < -1 && key > current->right->key)
        {
            return leftRotate(current);
        }
        // left-right-case
        if (balanceFactor > 1 && key > current->left->key)
        {
            current->left = leftRotate(current->left);
            return rightRotate(current);
        }
        // right-left-case
        if (balanceFactor < -1 && key < current->right->key)
        {
            current->right = rightRotate(current->right);
            return leftRotate(current);
        }

        // return unchanged node pointer
        return current;
    }

    // find node with minimum key value
    Node *minValueNode(Node *node)
    {
        Node *current = node;
        while (current->left != nullptr)
        {
            current = current->left;
        }
        return current;
    }

    Node *deleteNode(Node *node, T key)
    {
        // handle if avl tree is empty
        if (node == nullptr)
        {
            return node;
        }

        if (key < node->key)
        {
            node->left = deleteNode(node->left, key);
        }
        else if (key > node->key)
        {
            node->right = deleteNode(node->right, key);
        }
        else
        {
            // node with only one child or no child
            if ((node->left == nullptr) || (node->right == nullptr))
            {
                Node *temp = node->left ? node->left : node->right;

                // no child case
                if (temp == nullptr)
                {
                    temp = node;
                    node = nullptr;
                }
                else
                {                  // single child node
                    *node = *temp; // copy the contents of the non-empty child
                }
                delete temp;
            }
            else
            {
                // node with two children: Get the inorder successor (smallest in the right subtree)
                Node *temp = minValueNode(node->right);

                // copy the inorder successor's data to this node
                node->key = temp->key;

                // delete the inorder successor
                node->right = deleteNode(node->right, temp->key);
            }
        }

        // return if tree has only one node
        if (node == nullptr)
        {
            return node;
        }
      
        // update height of the current node
        node->height = 1 + std::max(height(node->left), height(node->right));

        // obtain balance factor if node
        int balanceFactor = getBalance(node);

        // handle unbalanced tree after deletion
        // left-left-case
        if (balanceFactor > 1 && getBalance(node->left) >= 0)
        {
            return rightRotate(node);
        }

        // left-right-case
        if (balanceFactor > 1 && getBalance(node->left) < 0)
        {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        // right-right-case
        if (balanceFactor < -1 && getBalance(node->right) <= 0)
        {
            return leftRotate(node);
        }

        // right-left-case
        if (balanceFactor < -1 && getBalance(node->right) > 0)
        {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

public:
    // avltree constructor
    AVLTree() : root(nullptr) {}

    void insertNode(T key)
    {
        root = insert(root, key);
    }

    void deleteNode(T key)
    {
        root = deleteNode(root, key);
    }

    class Iterator
    {
    private:
        struct StackNode
        {
            Node *treeNode;
            StackNode *next;
            StackNode(Node *node, StackNode *nxt) : treeNode(node), next(nxt) {}
        };

        StackNode *stackTop;

        void push(Node *node)
        {
            stackTop = new StackNode(node, stackTop);
        }

        void pop()
        {
            if (stackTop)
            {
                StackNode *temp = stackTop;
                stackTop = stackTop->next;
                delete temp;
            }
        }

        Node *top() const
        {
            return stackTop ? stackTop->treeNode : nullptr;
        }

        bool empty() const
        {
            return stackTop == nullptr;
        }

    public:
        Iterator(Node *root) : stackTop(nullptr)
        {
            while (root)
            {
                push(root);
                root = root->left;
            }
        }

        Iterator() : stackTop(nullptr) {}

        Iterator(const Iterator &other) : stackTop(nullptr)
        {
            StackNode **current = &stackTop;
            StackNode *otherCurrent = other.stackTop;
            while (otherCurrent)
            {
                *current = new StackNode(otherCurrent->treeNode, nullptr);
                current = &((*current)->next);
                otherCurrent = otherCurrent->next;
            }
        }

        Iterator &operator=(const Iterator &other)
        {
            if (this != &other)
            {
                while (!empty())
                    pop();
                StackNode **current = &stackTop;
                StackNode *otherCurrent = other.stackTop;
                while (otherCurrent)
                {
                    *current = new StackNode(otherCurrent->treeNode, nullptr);
                    current = &((*current)->next);
                    otherCurrent = otherCurrent->next;
                }
            }
            return *this;
        }

        ~Iterator()
        {
            while (!empty())
                pop();
        }

        T &operator*() const
        {
            return top()->key;
        }

        Iterator &operator++()
        {
            if (!empty())
            {
                Node *current = top();
                pop();
                Node *right = current->right;
                while (right)
                {
                    push(right);
                    right = right->left;
                }
            }
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator &other) const
        {
            StackNode *a = stackTop;
            StackNode *b = other.stackTop;
            while (a && b)
            {
                if (a->treeNode != b->treeNode)
                    return false;
                a = a->next;
                b = b->next;
            }
            return !a && !b;
        }

        bool operator!=(const Iterator &other) const
        {
            return !(*this == other);
        }
    };

    Iterator begin()
    {
        return Iterator(root);
    }

    Iterator end()
    {
        return Iterator();
    }
};
