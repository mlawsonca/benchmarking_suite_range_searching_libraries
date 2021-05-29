#ifndef NANOFLANN_TEST_HH
#define NANOFLANN_TEST_HH

#include <nanoflann.hpp>
using namespace std;

/** A simple vector-of-vectors adaptor for nanoflann, without duplicating the storage.
  *  The i'th vector represents a point in the state space.
  *
  *  \tparam DIM If set to >0, it specifies a compile-time fixed dimensionality for the points in the data set, allowing more compiler optimizations.
  *  \tparam double The type of the point coordinates (typically, double or float).
  *  \tparam Distance The distance metric to use: nanoflann::metric_L1, nanoflann::metric_L2, nanoflann::metric_L2_Simple, etc.
  *  \tparam IndexType The type for indices in the KD-tree index (typically, size_t of int)
  */
template <class VectorOfVectorsType, typename num_t = double, int DIM = -1, class Distance = nanoflann::metric_L2, typename IndexType = size_t>
struct KDTreeVectorOfVectorsAdaptor
{
    typedef KDTreeVectorOfVectorsAdaptor<VectorOfVectorsType,num_t,DIM,Distance> self_t;
    typedef typename Distance::template traits<num_t,self_t>::distance_t metric_t;
    typedef nanoflann::KDTreeSingleIndexAdaptor< metric_t,self_t,DIM,IndexType>  index_t;

    index_t* index; //! The kd-tree index for the user to call its methods as usual with any other FLANN index.

    /// Constructor: takes a const ref to the vector of vectors object with the data points
    KDTreeVectorOfVectorsAdaptor(const size_t /* dimensionality */, const VectorOfVectorsType &mat, const int leaf_max_size = 10) : m_data(mat)
    {
        assert(mat.size() != 0 && mat[0].size() != 0);
        const size_t dims = mat[0].size();
        if (DIM>0 && static_cast<int>(dims) != DIM)
            throw std::runtime_error("Data set dimensionality does not match the 'DIM' template argument");
        index = new index_t( static_cast<int>(dims), *this /* adaptor */, nanoflann::KDTreeSingleIndexAdaptorParams(leaf_max_size ) );
        index->buildIndex();
    }

    ~KDTreeVectorOfVectorsAdaptor() {
        delete index;
    }

    const VectorOfVectorsType &m_data;

    const self_t & derived() const {
        return *this;
    }
    self_t & derived()       {
        return *this;
    }

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const {
        return m_data.size();
    }

    // Returns the dim'th component of the idx'th point in the class:
    inline num_t kdtree_get_pt(const size_t idx, const size_t dim) const {
        return m_data[idx][dim];
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox(BBOX & /*bb*/) const {
        return false;
    }

    /** @} */

}; // end of KDTreeVectorOfVectorsAdaptor


struct MyPointCloud
{

    std::vector<point>  pts;

    MyPointCloud(const vector<point> &points) {
        pts = points;
    }

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const { return pts.size(); }

    // Returns the dim'th component of the idx'th point in the class:
    // Since this is inlined and the "dim" argument is typically an immediate value, the
    //  "if/else's" are actually solved at compile time.
    inline double kdtree_get_pt(const size_t idx, const size_t dim) const
    {
        return pts[idx][dim];
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }

};


struct pair_iterator : std::vector<std::pair<size_t,double>>::const_iterator
{
    using base_class = std::vector<std::pair<size_t,double>>::const_iterator;
    using value_type = size_t;

    using base_class::base_class;

    value_type operator*() const {
        auto const& src = base_class::operator*();
        return (src.first);
    }
};



class TestNanoflann : public BboxIntersectionTest {
    typedef std::vector<point> pt_vector;
    typedef KDTreeVectorOfVectorsAdaptor< pt_vector, double>  my_kdtree;
    typedef nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Adaptor<double, MyPointCloud>, MyPointCloud, NUM_DIMS> kdtree_duplicated_storage;

    private:

        my_kdtree *tree;
        kdtree_duplicated_storage *tree_duplicated_storage;
        MyPointCloud *cloud;
        bool use_duplicated_storage = false;

        void _build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size = NUM_ELEMS_PER_NODE, bool duplicate_storage = false)
        {
            use_duplicated_storage = duplicate_storage;
            if(use_duplicated_storage) {
                // MyPointCloud cloud(pts);
                cloud = new MyPointCloud(pts);
                tree_duplicated_storage = new kdtree_duplicated_storage(NUM_DIMS, *cloud, nanoflann::KDTreeSingleIndexAdaptorParams(bucket_size));
                tree_duplicated_storage->buildIndex();
            }
            else {
                tree = new my_kdtree(NUM_DIMS, pts, bucket_size);
                tree->index->buildIndex();
            }
        }

    public:

        bool intersections_exact() { return false; } //circular radius is not exact

        TestNanoflann() {}

        ~TestNanoflann() {
            if(use_duplicated_storage) {
                delete tree_duplicated_storage;
                delete cloud;                
            }
            else {
                delete tree;
            }
        }


        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            _build_tree(pts, indices);
        }

        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size, bool duplicate_storage = false) {
            //set both interior node capacity and leaf capacity to bucket size
            _build_tree(pts, indices, bucket_size, duplicate_storage);
        }


        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
            nanoflann::SearchParams params;
            params.sorted = false;

            point mid_pt;
            double squared_radius_search_bound = 0;
            get_max_squared_radius(my_bbox, mid_pt, squared_radius_search_bound);
            squared_radius_search_bound += DEFAULT_TOLERANCE;

            std::vector<std::pair<size_t,double>>   ret_matches;
            if(use_duplicated_storage) {
                tree_duplicated_storage->radiusSearch(&mid_pt[0], squared_radius_search_bound, ret_matches, params);
            }
            else {
                tree->index->radiusSearch(&mid_pt[0], squared_radius_search_bound, ret_matches, params);
            }
            intersections_indices.reserve(ret_matches.size());
            for(int i = 0; i < ret_matches.size(); i++) {
                intersections_indices.push_back(ret_matches[i].first);
            }


                 
        }
};

#endif //NANOFLANN_TEST_HH


