#pragma once

/*  Example usage:

DynamicLinkedList<int> linkedList;

// Adding elements
linkedList.push_back(1);  // Fast
linkedList.push_back(2);
linkedList.push_back(3);
auto targetElement = linkedList.push_back(4);
linkedList.push_back(5);

// Removing elements
linkedList.remove(*targetElement); // Fast
linkedList.remove(5);              // Slow

// Iterating over all elements
int i=0;
for (auto it : linkedList) {
    std::cout << "linkedList["<<i++<<"] = " << it << std::endl;
}
*/

#ifndef PC
#   include <sdk/os/mem.hpp>
#else
#   include <cstdlib>
#   include <iostream>
#endif

template <typename T>
class DynamicLinkedList {
private:
    struct Node {
        T data;
        Node* next;
    };

    Node* head;
    unsigned size;  // Added size variable

public:
    // Iterator for DynamicLinkedList
    class Iterator {
    private:
        Node* current;

    public:
        Iterator(Node* node) : current(node) {}

        T& operator*() const {
            return current->data;
        }

        Iterator& operator++() {
            current = current->next;
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return current != other.current;
        }
    };

    // Iterator for DynamicLinkedList Nodes
    class NodeIterator {
    private:
        Node* currentNode;

    public:
        NodeIterator(Node* node) : currentNode(node) {}

        Node* operator*() const {
            return currentNode;
        }

        NodeIterator& operator++() {
            if (currentNode) {
                currentNode = currentNode->next;
            }
            return *this;
        }

        bool operator!=(const NodeIterator& other) const {
            return currentNode != other.currentNode;
        }
    };

    // Function to get the NodeIterator for the beginning of the list
    NodeIterator node_begin() const {
        return NodeIterator(head);
    }

    // Function to get the NodeIterator for the end of the list
    NodeIterator node_end() const {
        return NodeIterator(nullptr);
    }

    DynamicLinkedList() : head(nullptr), size(0) {}  // Initialize size to 0

    // Push back and return a reference to the new node
    Node* push_back(const T& value) {
        Node* newNode = static_cast<Node*>(malloc(sizeof(Node)));
        if (!newNode)
            return nullptr;

        newNode->data = value;
        newNode->next = nullptr;

        if (!head) {
            // If the list is empty, set the new node as the head
            head = newNode;
        } else {
            // Find the last node and set its next to the new node
            Node* current = head;
            while (current->next)
                current = current->next;
            current->next = newNode;
        }

        size++;  // Increment size
        return newNode;
    }

    // Remove a node by its reference
    void remove(Node& node) {
        if (!head)
            return;

        if (&node == head) {
            // If the node to remove is the head, update the head to the next node
            head = head->next;
        } else {
            // Find the previous node to the one to be removed
            Node* current = head;
            while (current && current->next != &node)
                current = current->next;
            // Update the next pointer of the previous node to skip the node to be removed
            if (current)
                current->next = node.next;
        }
        // Free the memory of the removed node
        free(&node);
        size--;  // Decrement size
    }

    // Remove a node by its value
    void remove(const T& value) {
        if (!head)
            return;
        // Check if the value to remove is in the head
        if (head->data == value) {
            Node* temp = head;
            head = head->next;
            free(temp);
            size--;  // Decrement size
            return;
        }

        // Find the node with the value to be removed
        Node* current = head;
        while (current->next && current->next->data != value) {
            current = current->next;
        }

        if (current->next) {
            // Update the next pointer to skip the node with the value to be removed
            Node* temp = current->next;
            current->next = current->next->next;
            free(temp);
            size--;  // Decrement size
        }
    }

    // Function to remove a node by its index
    void remove(unsigned index) {
        if (index >= size || !head)
            return;  // Invalid index or empty list

        if (index == 0) {
            // If the index is 0, remove the head
            Node* temp = head;
            head = head->next;
            free(temp);
            size--;  // Decrement size
            return;
        }

        // Find the node at the specified index
        Node* current = head;
        for (unsigned i = 0; i < index - 1 && current->next; i++) {
            current = current->next;
        }

        if (current->next) {
            // Update the next pointer to skip the node at the specified index
            Node* temp = current->next;
            current->next = current->next->next;
            free(temp);
            size--;  // Decrement size
        }
    }

    // Function to sort the linked list using a custom comparison function
    void sort(bool (*comp)(const T&, const T&)) {
        if (!head || !head->next)
            return;  // One elemenet or empty list

        bool swapped;
        Node* last = nullptr;
        Node* current;

        do {
            swapped = false;
            current = head;

            while (current->next != last) {
                if (comp(current->data, current->next->data)) {
                    // Swap nodes
                    T temp = current->data;
                    current->data = current->next->data;
                    current->next->data = temp;
                    swapped = true;
                }

                current = current->next;
            }

            last = current;
        } while (swapped);
    }

    // Iterator functions
    Iterator begin() const {
        return Iterator(head);
    }

    Iterator end() const {
        return Iterator(nullptr);
    }

    // Function to get the size
    unsigned getSize() const {
        return size;
    }

    ~DynamicLinkedList() {
        // Free all nodes in the list
        while (head) {
            Node* temp = head;
            head = head->next;
            free(temp);
        }
    }

};
