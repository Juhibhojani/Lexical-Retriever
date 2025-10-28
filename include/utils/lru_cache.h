#pragma once
#include <map>
#include <pthread.h>
#include <optional>


// defining node for doubly-linked list
template <typename KeyType, typename ValueType>
struct Node {
    KeyType key;
    ValueType value;
    Node* prev;
    Node* next;

    // initialization of node
    Node(const KeyType& k, const ValueType& v)
        : key(k), value(v), prev(nullptr), next(nullptr) {}
};

// defining an LRU template class
template <typename KeyType, typename ValueType>
class LRUCache {
private:
    int capacity;
    int current_capacity;
    // cacheMap stores (key,pointer to double LL)
    std::map<KeyType, Node<KeyType, ValueType>*> cacheMap;
    // pointer to head of LL (most recently accessed element is stored after head)
    Node<KeyType, ValueType>* head; 
    // pointer to tail of LL (remove nodes from tail of LL)
    Node<KeyType, ValueType>* tail;
    // lock for multiple access
    pthread_mutex_t lock; 

    void addNode(Node<KeyType, ValueType>* node);
    void removeNode(Node<KeyType, ValueType>* node);
    void moveToFront(Node<KeyType, ValueType>* node);

public:
    explicit LRUCache(int cap);
    ~LRUCache();

    // Core methods
    // get will return the value corresponding to the key
    std::optional<ValueType> get(const KeyType& key);
    // adding node to cacheMap
    void put(const KeyType& key, const ValueType& value);
    // removnig node from cacheMap
    bool remove(const KeyType& key);
    // flush the entire cache
    void clear();
};


template <typename KeyType, typename ValueType>
LRUCache<KeyType, ValueType>::LRUCache(int cap)
{
    capacity = cap;
    current_capacity = 0;
    // initialize a head and tail
    head = new Node<KeyType, ValueType>(KeyType(), ValueType());
    tail = new Node<KeyType, ValueType>(KeyType(), ValueType());
    head->next = tail;
    tail->prev = head;

    // initialize lock
    pthread_mutex_init(&lock, nullptr);
}

template <typename KeyType, typename ValueType>
LRUCache<KeyType, ValueType>::~LRUCache()
{

    // free memory i.e. delete all nodes
    Node<KeyType, ValueType> *curr = head;
    while (curr)
    {
        Node<KeyType, ValueType> *next = curr->next;
        delete curr;
        curr = next;
    }

    // delete mutex
    pthread_mutex_destroy(&lock);
}

template <typename KeyType, typename ValueType>
void LRUCache<KeyType, ValueType>::addNode(Node<KeyType, ValueType> *node)
{
    // node will always be added at the head
    node->next = head->next;
    node->prev = head;
    head->next->prev = node;
    head->next = node;
}

template <typename KeyType, typename ValueType>
void LRUCache<KeyType, ValueType>::removeNode(Node<KeyType, ValueType> *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

template <typename KeyType, typename ValueType>
void LRUCache<KeyType, ValueType>::moveToFront(Node<KeyType, ValueType> *node)
{
    removeNode(node);
    addNode(node);
}

// implementing public methods
template <typename KeyType, typename ValueType>
std::optional<ValueType> LRUCache<KeyType, ValueType>::get(const KeyType &key)
{
    // acquire lock and do all the operations
    pthread_mutex_lock(&lock);

    // if key exists then get it else return false
    if (cacheMap.find(key) == cacheMap.end())
    {
        // doesn't exist in cache
        pthread_mutex_unlock(&lock);
        return std::nullopt;
    }

    Node<KeyType, ValueType> *valueNode = cacheMap[key];
    // while getting the key, also move it to the front of the list
    moveToFront(valueNode);
    ValueType val = valueNode->value;

    pthread_mutex_unlock(&lock);
    return val;
}

template <typename KeyType, typename ValueType>
void LRUCache<KeyType, ValueType>::put(const KeyType &key, const ValueType &value)
{
    // acquire lock and do all the operations
    pthread_mutex_lock(&lock);

    // if key doesn't exists then only append
    auto it = cacheMap.find(key);
    if (it == cacheMap.end())
    {
        // capacity full
        if (current_capacity >= capacity)
        {
            // erase a node first
            Node<KeyType, ValueType> *last = tail->prev;
            cacheMap.erase(last->key);
            removeNode(last);
            delete last;
            current_capacity--;
        }
        Node<KeyType, ValueType> *new_node = new Node<KeyType, ValueType>(key, value);
        addNode(new_node);
        cacheMap[key] = new_node;
        current_capacity++;
    }
    else
    {
        // if exists then move to front
        Node<KeyType, ValueType> *node = it->second;
        node->value = value;
        moveToFront(node);
    }
    pthread_mutex_unlock(&lock);
}

template <typename KeyType, typename ValueType>
bool LRUCache<KeyType, ValueType>::remove(const KeyType &key)
{
    // acquire lock and do all the operations
    pthread_mutex_lock(&lock);

    // if key exists then only i can remove
    if (cacheMap.find(key) != cacheMap.end())
    {

        Node<KeyType, ValueType> *node_to_remove = cacheMap[key];
        removeNode(node_to_remove);
        cacheMap.erase(key);
        delete node_to_remove;
        current_capacity--;
        pthread_mutex_unlock(&lock);
        return true;
    }
    pthread_mutex_unlock(&lock);
    return false;
}

template <typename KeyType, typename ValueType>
void LRUCache<KeyType, ValueType>::clear()
{
    pthread_mutex_lock(&lock);

    // Free linked list nodes
    Node<KeyType, ValueType> *curr = head->next;
    while (curr != tail)
    {
        Node<KeyType, ValueType> *next = curr->next;
        delete curr;
        curr = next;
    }

    // Reset empty list structure
    head->next = tail;
    tail->prev = head;
    cacheMap.clear();
    current_capacity = 0;

    pthread_mutex_unlock(&lock);
}
