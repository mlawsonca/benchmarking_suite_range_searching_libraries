#ifndef BOOST_TEST_HH
#define BOOST_TEST_HH

#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometry.hpp>


typedef boost::geometry::model::point<double, NUM_DIMS, boost::geometry::cs::cartesian> boost_point;
typedef std::pair<boost_point, size_t> boost_point_w_index;
typedef boost::geometry::model::box<boost_point> boost_bbox;
typedef std::pair<boost_bbox, size_t> boost_bbox_w_index;

inline boost_point make_boost_point(const point &pt) {
    return boost_point(pt[0], pt[1], pt[2]);
}

inline boost_bbox make_boost_bbox(const point &pt0, const point &pt1) {
    return boost_bbox(make_boost_point(pt0),make_boost_point(pt1));
}

static void print_boost_box(boost_bbox my_box) {
    std::cout << "(" << my_box.min_corner().get<0>() << " " << my_box.min_corner().get<1>() << ", " << my_box.min_corner().get<2>() << ")-";
    std::cout << "(" << my_box.max_corner().get<0>() << " " << my_box.max_corner().get<1>() << ", " << my_box.max_corner().get<2>() << ")" << std::endl;
}

static void print_boost_point(boost_point my_pt, bool suppress_newline = true) {
    std::cout << "(" << my_pt.get<0>() << " " << my_pt.get<1>() << ", " << my_pt.get<2>() << ")";
    if(! suppress_newline) {
        std::cout << std::endl;
    }
}



template <class BuildAlg>
class TestBoost : public BboxIntersectionTest {


    private:
        boost::geometry::index::rtree<boost_point_w_index, BuildAlg> *tree;
        boost::geometry::index::rtree<boost_bbox_w_index, BuildAlg> *tree_boxes;
        bool uses_boxes = false;

        struct boost_point_w_index_iterator : std::vector<boost_point_w_index>::const_iterator
        {
            using base_class = std::vector<boost_point_w_index>::const_iterator;
            using value_type = size_t;

            using base_class::base_class;

            value_type operator*() const {
                auto const& src = base_class::operator*();
                return (src.second );
            }
        };

        struct boost_bbox_w_index_iterator : std::vector<boost_bbox_w_index>::const_iterator
        {
            using base_class = std::vector<boost_bbox_w_index>::const_iterator;
            using value_type = size_t;

            using base_class::base_class;

            value_type operator*() const {
                auto const& src = base_class::operator*();
                return (src.second);
            }
        };
    public:
        bool intersections_exact() { return true; } //bounding box search

        TestBoost() {}
        ~TestBoost() {
            delete tree;
            delete tree_boxes;
        }

        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            std::vector<boost_point_w_index> data;
            for(int i = 0; i < pts.size(); i++) {
                data.push_back(std::make_pair(make_boost_point(pts[i]),indices[i]));
            }
             tree = new boost::geometry::index::rtree<boost_point_w_index, BuildAlg>(data.begin(), data.end());
        }     

        void build_tree_bbox(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            if((pts.size() % 2) !=0) {
                std::cerr << "error. your point list size has to be even to insert bounding boxes" << std::endl;
                return;
            }
            uses_boxes = true; 
            std::vector<boost_bbox_w_index> data;
            for(int i = 0; i < pts.size(); i += 2) {
                data.push_back(std::make_pair(make_boost_bbox(pts[i], pts[i+1]),indices[i/2]));
            }
             tree_boxes = new boost::geometry::index::rtree<boost_bbox_w_index, BuildAlg>(data.begin(), data.end());
        }      

        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
            if(uses_boxes) {
                std::vector<boost_bbox_w_index> results;
                tree_boxes->query(boost::geometry::index::intersects(make_boost_bbox(my_bbox.first, my_bbox.second)), std::back_inserter(results));
                intersections_indices.assign(boost_bbox_w_index_iterator(results.begin()),
                                           boost_bbox_w_index_iterator(results.end()));
            }
            else {
                std::vector<boost_point_w_index> results;
                tree->query(boost::geometry::index::intersects(make_boost_bbox(my_bbox.first, my_bbox.second)), std::back_inserter(results));
                intersections_indices.assign(boost_point_w_index_iterator(results.begin()),
                                           boost_point_w_index_iterator(results.end()));
            }
        }
};

#endif //BOOST_TEST_HH