#ifndef FCTree_H
#define FCTree_H

#include "Graph/MultilayerGraph.h"


// Test file. Use the tree structure to store the core result.
// Given the k and lmd return the length and the core.

struct coreNode{

    unsigned int k{0};
    unsigned int lmd{0};
    unsigned int length{0};
    uint *core{nullptr};

    coreNode* left;
    coreNode* right;

    coreNode(){};

    coreNode(unsigned int _k, unsigned int _lmd, unsigned int _length){
        k = _k;
        lmd = _lmd;
        length = _length;
    };
     
};

class FCTree{
private:
    coreNode *node;
public:
    FCTree(/* args */);
    ~FCTree();

    FCTree(unsigned int k, unsigned int lmd, unsigned int length){
       node = new coreNode(k, lmd, length);
    }

    // get the root Node
    inline coreNode* getNode() const {
        return node;
    }

    coreNode* getCoreByKAndLmd(coreNode *node, int k, unsigned int lmd);

    void saveCoreToLocal(string dataset, ll_uint *id2vtx, coreNode *node);

};





#endif