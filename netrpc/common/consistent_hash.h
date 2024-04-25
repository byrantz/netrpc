#pragma once

#include <map>

namespace netrpc {

class ConsistentHash
{
public:
    ConsistentHash(int node_num, int virtual_node_num);
    ~ConsistentHash();

    void Initialize();
    size_t GetServerIndex(const char *key);

    void DeleteNode(const int index);
    void AddNewNode(const int index);

private:
    std::map<uint32_t, size_t> server_nodes_; //虚拟节点,key是哈希值，value是机器的index
    int node_num_;                       //真实机器节点个数
    int virtual_node_num_;               //每个机器节点关联的虚拟节点个数
};

}