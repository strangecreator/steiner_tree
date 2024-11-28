#include <algorithm>
#include <vector>
#include <stdexcept>


template <typename CompType>  // for example long long
class BinaryHeap {
  public:
    struct HeapNode {
        CompType value;
        int id;
    };

    struct Node {
        int index;
    };

  protected:
    std::vector<HeapNode> heap;
    std::vector<Node> nodes;

    int ParentIndex(int index) const {
        if (index == 0) { return -1; }
        return (index - 1) / 2;
    }

    int LeftChildIndex(int index) const {
        return index * 2 + 1;
    }

    int RightChildIndex(int index) const {
        return index * 2 + 2;
    }

    void Up(int id) {
        int index = nodes[id].index;
        if (index == 0) { return; }

        int parent_index = ParentIndex(nodes[id].index);
        if (heap[index].value >= heap[parent_index].value) { return; }
        
        std::swap(heap[parent_index], heap[index]);
        nodes[heap[parent_index].id].index = parent_index;
        nodes[heap[index].id].index = index;

        return Up(id);
    }

    void Down(int id) {
        int index = nodes[id].index;
        int left_index = LeftChildIndex(index);
        int right_index = RightChildIndex(index);

        if (left_index >= static_cast<int>(heap.size())) { return; }

        int index_to_consider = left_index;
        if (right_index < static_cast<int>(heap.size()) && heap[right_index].value < heap[left_index].value) {
            index_to_consider = right_index;
        }

        if (heap[index_to_consider].value < heap[index].value) {
            std::swap(heap[index_to_consider], heap[index]);
            nodes[heap[index].id].index = index;
            nodes[heap[index_to_consider].id].index = index_to_consider;

            return Down(id);
        }
    }

  public:
    BinaryHeap() {}

    int Add(CompType value) {  // returns an id of this newly created node
        int id = nodes.size();
        nodes.push_back(Node{static_cast<int>(heap.size())});
        heap.push_back(HeapNode{value, id});
        Up(id);
        return id;
    }

    CompType GetById(int id) const {
        if (id < 0 || nodes.size() <= id) {
            throw std::runtime_error("Invalid node argument!");
        }
        if (nodes[id].index == -1) {
            throw std::runtime_error("This node was deleted!");
        }

        return heap[nodes[id].index].value;
    }

    void ChangeById(int id, CompType new_value) {
        if (id < 0 || static_cast<int>(nodes.size()) <= id) {
            throw std::runtime_error("Invalid node argument!");
        }
        if (nodes[id].index == -1) {
            throw std::runtime_error("This node was deleted!");
        }

        if (heap[nodes[id].index].value > new_value) {
            heap[nodes[id].index].value = new_value;
            Up(id);
        } else {
            heap[nodes[id].index].value = new_value;
            Down(id);
        }
    }

    HeapNode GetMin() const {
        if (heap.empty()) {
            throw std::runtime_error("The heap is empty! You cannot get min.");
        }
        return heap[0];
    }

    HeapNode Pop() {
        if (heap.empty()) {
            throw std::runtime_error("The heap is empty! You cannot pop.");
        }
        int id = heap[0].id;
        HeapNode result(heap[0]);

        if (heap.size() > 1) {
            std::swap(heap[0], heap[heap.size() - 1]);
            nodes[heap[0].id].index = 0;
        }

        heap.pop_back();
        nodes[id].index = -1;  // means deleted

        if (heap.size() > 1) {
            Down(heap[0].id);
        }

        return result;
    }
};