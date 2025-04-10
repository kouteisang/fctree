#include "CoreIndex.h"

CoreIndex::CoreIndex(/* args */){
}

CoreIndex::~CoreIndex(){
}


struct LayerDegree2{
    uint id;
    float degree;
};

bool cmp(const LayerDegree2 &a, const LayerDegree2 &b){
    return a.degree > b.degree;
}


bool reverseSort(uint a, uint b) {
    return a > b; 
}

void print(std::vector<std::vector<uint>> B, uint n_vertex){
    cout << "==========Round==========" << endl;
   for(uint k = 1; k <= n_vertex; k ++){
        cout << "k = " << k << ": ";
        for(uint j = 0; j < B[k].size(); j ++){
            cout << B[k][j] << " ";
        }
        cout << endl;
    }
}

uint findLmdthLargest(uint *deg, uint lmd, uint n_layer){

    uint *deg_sort = new uint[n_layer];
    memcpy(deg_sort, deg, sizeof(uint) * n_layer);


    sort(deg_sort, deg_sort+n_layer, reverseSort);

    return deg_sort[lmd-1];
}

uint* get_index(MultilayerGraph &mg, uint **degs, uint lmd, ll_uint *id2vtx){
     

    uint n_layer = mg.getLayerNumber();
    uint n_vertex = mg.GetN(); 
    uint *i_v = new uint[n_vertex];
    uint *core = new uint[n_vertex];

    std::vector<std::set<uint>> B(n_vertex+1);

    bool visit[n_vertex];
    memset(visit, false, sizeof(bool) * n_vertex);


    for(uint v = 0; v < n_vertex; v ++){
        i_v[v] = findLmdthLargest(degs[v], lmd, n_layer); // This is where the problem happens
        B[i_v[v]].insert(v);
    }

    
    for(uint k = 0; k <= n_vertex; k ++){
        while(!B[k].empty()){ 
            auto v = B[k].begin();
            uint vv = *v;
            B[k].erase(vv);
            core[vv] = k;
            visit[vv] = true;
            std::set<uint> N;
            for(uint l = 0; l < n_layer; l ++){
                uint **adj_lst = mg.GetGraph(l).GetAdjLst();
                for(uint u = 1; u <= adj_lst[vv][0]; u ++){
                    uint nb_u = adj_lst[vv][u]; // node v's neighbourhood in layer l
                    if(i_v[nb_u] <= k) continue;
                    if(visit[nb_u]) continue;
                    degs[nb_u][l] = degs[nb_u][l] - 1;
                    if(degs[nb_u][l] + 1 == i_v[nb_u]){
                        N.insert(nb_u);
                    }
                }
            }
            
            for(const uint& u : N){
                B[i_v[u]].erase(u);
                i_v[u] = findLmdthLargest(degs[u], lmd, n_layer); 
                B[std::max(i_v[u], k)].insert(u);
            }

        }

    }

    return core;
}


void CoreIndex::Execute(MultilayerGraph &mg, ll_uint *id2vtx){
    cout << "I am the CoreIndex" << endl;
    uint n_layer = mg.getLayerNumber();
    uint n_vertex = mg.GetN();

    cout << "n_layer = " << n_layer << endl;
    cout << "n_vertex = " << n_vertex << endl;

    uint **degs;
    degs = new uint*[n_vertex];
    // degs_sort = new uint*[n_vertex];
    
    // Parallel init the degree information
    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for(int v = 0; v < n_vertex; v ++){
             degs[v] = new uint[n_layer];
        } 

        #pragma omp for schedule(static) collapse(2)
        for(int v = 0; v < n_vertex; v ++){
            for(int l = 0; l < n_layer; l ++){
                degs[v][l] = mg.GetGraph(l).GetAdjLst()[v][0];
            }
        }
    }




    for(uint lmd = 1; lmd <= n_layer; lmd ++){

        // Copy the degs degree
        uint **degs_copy = new uint*[n_vertex];
        for(uint v = 0; v < n_vertex; v ++){
            degs_copy[v] = new uint[n_layer];
            memcpy(degs_copy[v], degs[v], n_layer * sizeof(uint));
        }

        uint *core = get_index(mg, degs_copy, lmd, id2vtx);

        
        cout << "lmd = " << lmd << endl;
        cout << "==========" << endl;
        std::ofstream outFile("/home/cheng/fctree/s1/s1-"+std::to_string(lmd)+".txt");
        for(uint v = 0; v < n_vertex; v ++){
            outFile << id2vtx[v] << ": " << core[v] << endl;
            // cout << v << ": " << core[v] << endl;
        }
        outFile.close();
    }

}


// The following two algorithms are getting weight denset subgraph algorithm

void calculateWeightDenestSubgraphCommon(uint k, uint lmd, MultilayerGraph &mg, std::unordered_set<uint> valid_node_set, float &maximum_density, float beta, uint &res_k, uint &res_lmd){
    
    uint n_layers = mg.getLayerNumber();
    uint n_vertex = mg.GetN();
    uint n_node = valid_node_set.size();

    LayerDegree2 *layer_degree = new LayerDegree2[n_layers];

    for(uint l = 0; l < n_layers; l ++){
        uint **adj = mg.GetGraph(l).GetAdjLst();
        float n_edge = 0;
        for (const auto& vv:valid_node_set){
            for(uint u = 1; u <= adj[vv][0]; u ++){
                auto uu = adj[vv][u];
                if(valid_node_set.find(uu) != valid_node_set.end()){
                    n_edge ++;
                }
            }
        }
        layer_degree[l].id = l;
        layer_degree[l].degree = n_edge/2;
    }

    float *maximum_average_degree = new float[n_layers];
    sort(layer_degree, layer_degree+n_layers, cmp);

    for(uint l = 0; l < n_layers; l ++){
        auto lid = layer_degree[l].id;
        maximum_average_degree[lid] = layer_degree[lid].degree / n_node;
        float layer_density = maximum_average_degree[lid] * pow((l+1), beta); 
        if(layer_density >= maximum_density){
            maximum_density = layer_density;
            res_k = k;
            res_lmd = lmd;
        }
    }
    
    delete[] layer_degree;

}

uint* get_wds(MultilayerGraph &mg, uint **degs, uint lmd, ll_uint *id2vtx, float &maximum_density, float beta, uint &res_k, uint &res_lmd){
     

    uint n_layer = mg.getLayerNumber();
    uint n_vertex = mg.GetN(); 
    uint *i_v = new uint[n_vertex];
    uint *core = new uint[n_vertex];

    std::vector<std::set<uint>> B(n_vertex+1);

    bool visit[n_vertex];
    memset(visit, false, sizeof(bool) * n_vertex);
    
    // valid set
    std::unordered_set<uint> valid_node_set;

    for(uint v = 0; v < n_vertex; v ++){
        i_v[v] = findLmdthLargest(degs[v], lmd, n_layer); // This is where the problem happens
        B[i_v[v]].insert(v);
        valid_node_set.insert(v);
    }



    for(uint k = 0; k <= n_vertex; k ++){
        std::unordered_set<uint> t_node_set;
        while(!B[k].empty()){ 
            auto v = B[k].begin();
            uint vv = *v;
            B[k].erase(vv);
            core[vv] = k;
            visit[vv] = true;
            t_node_set.insert(vv);
            std::set<uint> N;
            for(uint l = 0; l < n_layer; l ++){
                uint **adj_lst = mg.GetGraph(l).GetAdjLst();
                for(uint u = 1; u <= adj_lst[vv][0]; u ++){
                    uint nb_u = adj_lst[vv][u]; // node v's neighbourhood in layer l
                    if(i_v[nb_u] <= k) continue;
                    if(visit[nb_u]) continue;
                    degs[nb_u][l] = degs[nb_u][l] - 1;
                    if(degs[nb_u][l] + 1 == i_v[nb_u]){
                        N.insert(nb_u);
                    }
                }
            }
            
            for(const uint& u : N){
                B[i_v[u]].erase(u);
                i_v[u] = findLmdthLargest(degs[u], lmd, n_layer); 
                B[std::max(i_v[u], k)].insert(u);
            }

        }

        if(k >= 1 && !valid_node_set.empty()){
            calculateWeightDenestSubgraphCommon(k, lmd, mg, valid_node_set, maximum_density, beta, res_k, res_lmd);
        }

        for(const auto& ele : t_node_set){
           valid_node_set.erase(ele); 
        }

    }

    return core;
}

void CoreIndex::WdsCoreIndex(MultilayerGraph &mg, ll_uint *id2vtx, float beta){
    cout << "I am the WDS CoreIndex" << endl;
    uint n_layer = mg.getLayerNumber();
    uint n_vertex = mg.GetN();

    cout << "n_layer = " << n_layer << endl;
    cout << "n_vertex = " << n_vertex << endl;

    uint **degs;
    degs = new uint*[n_vertex];

    // degs_sort = new uint*[n_vertex];
    
    // Parallel init the degree information
    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for(int v = 0; v < n_vertex; v ++){
             degs[v] = new uint[n_layer];
        } 

        #pragma omp for schedule(static) collapse(2)
        for(int v = 0; v < n_vertex; v ++){
            for(int l = 0; l < n_layer; l ++){
                degs[v][l] = mg.GetGraph(l).GetAdjLst()[v][0];
            }
        }
    }

    float maximum_density = 0.0f;
    uint res_k = 0;
    uint res_lmd = 0;


    for(uint lmd = 1; lmd <= n_layer; lmd ++){

        // Copy the degs degree
        uint **degs_copy = new uint*[n_vertex];
        for(uint v = 0; v < n_vertex; v ++){
            degs_copy[v] = new uint[n_layer];
            memcpy(degs_copy[v], degs[v], n_layer * sizeof(uint));
        }

        get_wds(mg, degs_copy, lmd, id2vtx, maximum_density, beta, res_k, res_lmd);
    }

    cout << "maximum_density = " << maximum_density << endl;
    cout << "res_k = " << res_k << endl;
    cout << "res_lmd = " << res_lmd << endl;
     
}
