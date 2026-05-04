#pragma once

#include <thread>
#include <atomic>

template<typename T>
class LockFreeStack
{
private:
    struct Node
    {
        Node(T const& data)
            : data(std::make_shared<T>(data))
        {

        }
        std::shared_ptr<T> data;
        Node* next = nullptr;
    };

    std::atomic<Node*> mHead;
    std::atomic<Node*> mToBeDeleted;        // nodes to be deleted when safe
    std::atomic<unsigned> mThreadsInPop;    // number of threads in the process of popping from the stack

public:
    // returns true if stack is empty
    bool Empty()
    {
        return mHead.load() == nullptr;
    }
    void Push(T const& data)
    {
        Node* const newNode(new Node(data));
        newNode->next = mHead.load();
        while (!mHead.compare_exchange_weak(newNode->next, newNode))
        {
            // empty
        }
    }
    std::shared_ptr<T> Pop()
    {
        ++mThreadsInPop;
        Node* oldHead(mHead.load());
        while (oldHead != nullptr && !mHead.compare_exchange_weak(oldHead, oldHead->next))
        {
            //empty;
        }

        std::shared_ptr<T> ret;
        if (oldHead != nullptr)
        {
            ret.swap(oldHead->data);
            TryReclaim(oldHead);
        }
        return ret;
    }
    void TryReclaim(Node* oldHead)
    {
        if (mThreadsInPop == 1)
        {
            // first check if it is ok to delete other nodes which are waiting to be deleted
            Node* nodesToDelete = mToBeDeleted.exchange(nullptr);
            if (!--mThreadsInPop)
            {
                // still the only thread in pop, so delete all nodes waiting to be deleted
                DeleteNodes(nodesToDelete); 
            }
            else if (nodesToDelete != nullptr)
            {
                ChainPendingNodes(nodesToDelete);
            }
            // next reclaim the memory of the oldHead
            delete oldHead;
        }
        else
        {
            ChainPendingANode(oldHead);
            --mThreadsInPop;
        }
    }

    void ChainPendingNodes(Node* nodes)
    {
        Node* last(nodes);
        while (Node* const next = last->next)
        {
            last = next;
        }
        EnlistPendingNodes(nodes, last);
    }

    void EnlistPendingNodes(Node* first, Node* last)
    {
        last->next = mToBeDeleted;
        while (!mToBeDeleted.compare_exchange_weak(last->next, first))
        {
            // empty
        }
    }
    void ChainPendingANode(Node* n)
    {
        EnlistPendingNodes(n, n);
    }

    // delete all the ndoes in thes node to delete the list
    static void DeleteNodes(Node* nodes)
    {
        while (nodes != nullptr)
        {
            Node* next(nodes->next);
            delete nodes;
            nodes = next;
        }
    }
};