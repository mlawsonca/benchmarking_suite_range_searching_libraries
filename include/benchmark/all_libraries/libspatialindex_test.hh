#ifndef LIBSPATIALINDEX_TEST_HH
#define LIBSPATIALINDEX_TEST_HH

#include <spatialindex/tools/Tools.h>
#include <spatialindex/SpatialIndex.h>
#include <spatialindex/capi/sidx_impl.h>

using namespace std;

class SIDX_DLL MyIdVisitor : public SpatialIndex::IVisitor
{
private:
    std::vector<uint64_t> *m_vector;
    uint64_t nResults;

public:

    MyIdVisitor(std::vector<uint64_t> &vect) : nResults(0), m_vector(&vect) {
    }
    ~MyIdVisitor() {
    };

    uint64_t GetResultCount() const { return nResults; }
    std::vector<uint64_t>& GetResults()  { return *m_vector; }
    
    void visitNode(const SpatialIndex::INode& )
    {

    }

    void visitData(const SpatialIndex::IData& d)
    {
        nResults += 1;
        
        m_vector->push_back(d.getIdentifier());
    }

    void visitData(std::vector<const SpatialIndex::IData*>& )
    {
    }
};

class MyDataStream : public SpatialIndex::IDataStream
{
public:
    int index = 0;
    vector<point> my_vec;
    uint8_t *p_data=0;
    bool is_pts = true;

    MyDataStream(const vector<point> &vect, bool points) 
    {
        my_vec = vect;
        is_pts = points;
    }

    ~MyDataStream() override
    {
    }

    SpatialIndex::IData* getNext() override
    {
        if (index >= my_vec.size()) {
            return nullptr;
        }
        if(is_pts) {
            SpatialIndex::Region region(&my_vec[index][0], &my_vec[index][0], my_vec[index].size());
            //constructor only takes a region
            return(new SpatialIndex::RTree::Data(0, NULL, region, index++));           
        }
        else { //is regions expressed as a flattened 2D vector of points ( e.g. bbox = (pts[i],pts[i+1]) )
            size_t orig_index = index;
            SpatialIndex::Region region(&my_vec[index][0], &my_vec[index+1][0], my_vec[index].size());
            //constructor only takes a region
            index += 2;
            return(new SpatialIndex::RTree::Data(0, NULL, region, orig_index/2));   
        }


    }

    bool hasNext() override
    {
        return (index < my_vec.size());
    }

    uint32_t size() override
    {
        return my_vec.size();
    }

    void rewind() override
    {
        index = 0;
    }
};


class TestLibspatialindex : public BboxIntersectionTest {
    private:
        SpatialIndex::ISpatialIndex* tree;
        SpatialIndex::IStorageManager* storage_manager;

        void _build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, bool is_pts = true, 
            size_t interior_node_capacity = NUM_ELEMS_PER_NODE, 
            size_t leaf_capacity = NUM_ELEMS_PER_NODE, SpatialIndex::RTree::RTreeVariant rtree_variant = SpatialIndex::RTree::RV_RSTAR,
            float fill_factor = .5)
        {
            if(!is_pts && (pts.size() % 2) !=0) {
                std::cerr << "error. your point list size has to be even to insert bounding boxes" << std::endl;
                return;
            }

            size_t num_dims = 0;
            if(pts.size() > 0) {
                num_dims = pts[0].size();
            }
            storage_manager = SpatialIndex::StorageManager::createNewMemoryStorageManager();
            SpatialIndex::id_type indexIdentifier;
            MyDataStream data_stream =  MyDataStream(pts, is_pts);

            //bulk load method, idatastream, storage manger, fill factor, index capacity, leaf capacity, num_dims, rtree variant, index identifier
            tree = SpatialIndex::RTree::createAndBulkLoadNewRTree(
                SpatialIndex::RTree::BLM_STR, data_stream, *storage_manager, fill_factor, interior_node_capacity, 
                    leaf_capacity, num_dims,rtree_variant, indexIdentifier);

            bool ret = tree->isIndexValid();
            if (ret == false) std::cerr << "ERROR: Structure is invalid!" << std::endl;
            // else std::cerr << "The stucture seems O.K." << std::endl;
        }

    public:

        bool intersections_exact() { return true; } //bounding box search

        TestLibspatialindex() {}

        ~TestLibspatialindex() {
            delete tree;
        }


        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            _build_tree(pts, indices, true);
        }

        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size) {
            //set both interior node capacity and leaf capacity to bucket size
            _build_tree(pts, indices, true, bucket_size, bucket_size);
        }

        void build_tree_linear(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size=NUM_ELEMS_PER_NODE) {
            //set both interior node capacity and leaf capacity to bucket size
            _build_tree(pts, indices, true, bucket_size, bucket_size, SpatialIndex::RTree::RV_LINEAR);
        }

        void build_tree_quadratic(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size=NUM_ELEMS_PER_NODE) {
            //set both interior node capacity and leaf capacity to bucket size
            _build_tree(pts, indices, true, bucket_size, bucket_size, SpatialIndex::RTree::RV_QUADRATIC);
        }

        void build_tree_bbox(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size=NUM_ELEMS_PER_NODE) {
            _build_tree(pts, indices, false, bucket_size, bucket_size);
        }

        void build_tree_bbox_linear(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size=NUM_ELEMS_PER_NODE) {
            //set both interior node capacity and leaf capacity to bucket size
            _build_tree(pts, indices, false, bucket_size, bucket_size, SpatialIndex::RTree::RV_LINEAR);
        }

        void build_tree_bbox_quadratic(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size=NUM_ELEMS_PER_NODE) {
            //set both interior node capacity and leaf capacity to bucket size
            _build_tree(pts, indices, false, bucket_size, bucket_size, SpatialIndex::RTree::RV_QUADRATIC);
        }

        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
            SpatialIndex::Region query_region(&my_bbox.first[0], &my_bbox.second[0], my_bbox.first.size());
            MyIdVisitor vis(intersections_indices);
            tree->intersectsWithQuery(query_region, vis);
             
        }
};

#endif //LIBSPATIALINDEX_TEST_HH

