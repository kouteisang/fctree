#ifndef FCTreeBuilderPathParallelBylmd_H
#define FCTreeBuilderPathParallelBylmd_H

#include "Graph/MultilayerGraph.h"
#include "FCTree.h"

class FCTreeBuilderPathParallelBylmd
{
private:
    /* data */
public:
    FCTreeBuilderPathParallelBylmd(/* args */);
    ~FCTreeBuilderPathParallelBylmd();

    static void Execute(MultilayerGraph &mg, FCTree &tree);
    // ========= Common Method ========
    static uint peel(MultilayerGraph &mg, uint **degs, uint* klmd, uint *core, uint *pos, uint s, uint e);
    static bool check(uint **degs, uint u, uint* klmd, uint n_layers);
    static void constructCore(uint** degs, uint *klmd, uint *pos, uint *core, uint n_vertex, uint n_layer, coreNode *node, uint new_e, bool serial);
    static void PrintCoreInfor(uint *klmd, uint *core, uint new_e, uint n_vertex);
    // ========== Path Serial ==========
    static void PathSerial(MultilayerGraph& mg, uint* klmd, uint** degs, coreNode *node, uint* core, uint* pos, uint e, uint &count);
    // ========== Path Parallel ========== 
    static void PathParallel(MultilayerGraph &mg, coreNode *node, uint** degs, uint *pos, uint *core, uint e);
};


#endif

