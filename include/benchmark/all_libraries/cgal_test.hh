#ifndef CGAL_TEST_HH
#define CGAL_TEST_HH

#include <CGAL/Kd_tree.h>
#include <CGAL/Fuzzy_iso_box.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Search_traits_3.h>
#include <CGAL/Search_traits_adapter.h>
#include <CGAL/property_map.h>
#include <CGAL/Range_segment_tree_traits.h>
#include <CGAL/Segment_tree_k.h> // needed for segment tree
#include <CGAL/Range_tree_k.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>


#ifndef NUM_ELEMS_PER_NODE
    #error Your need to define NUM_ELEMS_PER_NODE in a common header file
#endif 

using namespace std;


typedef CGAL::Simple_cartesian<double> cgal_kernel;
typedef cgal_kernel::Point_3 cgal_point;
//allows you to use data types other than double
// typedef cgal_kernel::Iso_cuboid_3 cgal_bbox; 
// typedef cgal_kernel::Bbox cgal_bbox;
typedef CGAL::Bbox_3 cgal_bbox;
typedef cgal_kernel::Iso_cuboid_3 cgal_isobbox; 


static cgal_point make_cgal_point(const point &pt) {
    return(cgal_point(pt[0],pt[1],pt[2]));
}

static cgal_isobbox make_cgal_isobbox(const point &pt0, const point &pt1) {
    return(cgal_isobbox(pt0[0],pt0[1],pt0[2], pt1[0],pt1[1],pt1[2]));
}

static void print_cgal_point(const cgal_point &pt) {
    cout << "pt: (" << pt.x() << ", " << pt.y() << ", " << pt.z() << ")" << endl;
}


namespace TestCGAL {
    class KDTree : public BboxIntersectionTest {
        typedef boost::tuple<cgal_point,uint32_t>              cgal_point_with_index;
        typedef CGAL::Search_traits_3<cgal_kernel> cgal_traits_base;
        typedef CGAL::Search_traits_adapter<cgal_point_with_index, CGAL::Nth_of_tuple_property_map<0, cgal_point_with_index>, cgal_traits_base> Traits;
        typedef CGAL::Kd_tree<Traits> cgal_kd_tree;
        typedef CGAL::Fuzzy_iso_box<Traits> cgal_fuzzy_iso_box;
        typedef CGAL::Sliding_midpoint<Traits> cgal_sliding_midpoint;

        private:
            cgal_kd_tree *tree;

            struct boost_tuple_iterator : std::vector<cgal_point_with_index>::const_iterator
            {
                using base_class = std::vector<cgal_point_with_index>::const_iterator;
                using value_type = size_t;

                using base_class::base_class;

                value_type operator*() const {
                    auto const& src = base_class::operator*();
                    return (boost::get<1>(src) );
                }
            };

        public:
            bool intersections_exact() { return true; } //bounding box search

            KDTree() {}
            ~KDTree() {
                delete tree;
            }

            //could enable it to take different plitting techniques, but all studies point to sliding midpoint being best
            void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
                build_tree(pts, indices, NUM_ELEMS_PER_NODE);  
            }

            void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size) 
            {
                cgal_sliding_midpoint sliding_midpoint = cgal_sliding_midpoint(bucket_size);
                tree = new cgal_kd_tree(sliding_midpoint);

                for(int i = 0; i < pts.size(); i++){
                    tree->insert(boost::make_tuple(make_cgal_point(pts[i]),i));
                }
            }

            void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
                size_t index = 0; //dummy, don't need our query points to have indices
                cgal_point_with_index lower_pt =  boost::make_tuple(make_cgal_point(my_bbox.first), index);
                cgal_point_with_index upper_pt = boost::make_tuple(make_cgal_point(my_bbox.second), index);
                std::vector<cgal_point_with_index> matching_points;
                //the search isn't "fuzzy" since we don't define an error term/epsilon
                tree->search(std::back_inserter(matching_points), cgal_fuzzy_iso_box(lower_pt, upper_pt));
                intersections_indices.reserve(matching_points.size());
                intersections_indices.assign(boost_tuple_iterator(matching_points.begin()),
                                           boost_tuple_iterator(matching_points.end()));
            }
    };

     

    class RangeTree : public BboxIntersectionTest {
        typedef CGAL::Range_tree_map_traits_3<cgal_kernel, size_t> Traits;
        typedef CGAL::Range_tree_3<Traits> Range_tree_3_type;
        typedef Traits::Key Key;
        typedef Traits::Pure_key Point;
        typedef Traits::Interval Interval;

        private:
            // Range_tree_3_type tree;
            Range_tree_3_type *tree;
            double tolerance = DEFAULT_TOLERANCE; 


            struct key_iterator : std::vector<Key>::const_iterator
            {
                using base_class = std::vector<Key>::const_iterator;
                using value_type = size_t;

                using base_class::base_class;

                value_type operator*() const {
                    auto const& src = base_class::operator*();
                    return (src.second);
                }
            };

        public:
            RangeTree() {}
            ~RangeTree() {
                delete tree;
            }

            bool intersections_exact() { return true; } //bounding box search

            void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
                std::vector<Key> InputList ;
                for(int i = 0; i < pts.size(); i++) {
                    InputList.push_back(Key(Point(pts[i][0],pts[i][1],pts[i][2]), indices[i]));
                }
                tree = new Range_tree_3_type(InputList.begin(),InputList.end());

            }

            void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
                std::vector<Key> OutputList;
                size_t index = 0; //dummy, we don't need the search interval to have an index
                //need to add a tolerance since window query doens't include points directly on the end of the interval
                Interval win(Interval(Key(Point(my_bbox.first[0]-tolerance,my_bbox.first[1]-tolerance,my_bbox.first[2]-tolerance), index), 
                    Key(Point(my_bbox.second[0]+tolerance, my_bbox.second[1]+tolerance,my_bbox.second[2]+tolerance), index)));
                tree->window_query(win, std::back_inserter(OutputList));
                intersections_indices.assign(key_iterator(OutputList.begin()),
                                           key_iterator(OutputList.end()));
            }
    };





    class SegmentTree : public BboxIntersectionTest {
        typedef CGAL::Segment_tree_map_traits_3<cgal_kernel, size_t> Traits;
        typedef CGAL::Segment_tree_3<Traits > Segment_tree_3_type;
        typedef Traits::Interval Interval;
        typedef Traits::Pure_interval Pure_interval;
        typedef Traits::Key Key;

        private:
            Segment_tree_3_type *tree;
            double tolerance = DEFAULT_TOLERANCE; 

            struct interval_iterator : std::vector<Interval>::const_iterator
            {
                using base_class = std::vector<Interval>::const_iterator;
                using value_type = size_t;

                using base_class::base_class;

                value_type operator*() const {
                    auto const& src = base_class::operator*();
                    return (src.second);
                }
            };
        public:
            SegmentTree() {}
            ~SegmentTree() {
                delete tree;
            }

            bool intersections_exact() { return true; } //bounding box search


            //IMPORTANT NOTE: have to make sure each interval you add is actually 3D (has non-zero length in x, y, and z dimension). 
            //Otherwise it won't show up in intersection queries
            void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
                if((pts.size() % 2) !=0) {
                    std::cerr << "error. your point list size has to be even to insert bounding boxes" << std::endl;
                    return;
                }
                std::vector<Interval> InputList;
                for(int i = 0; i < pts.size(); i += 2) {
                    InputList.push_back(Interval(Pure_interval(Key(pts[i][0],pts[i][1],pts[i][2]), Key(pts[i+1][0],pts[i+1][1],pts[i+1][2])), indices[i/2]));
                }
                tree = new Segment_tree_3_type(InputList.begin(),InputList.end());
            }


            void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {

                std::vector<Interval> OutputList;
                size_t index = 0; //dummy, we don't need the search interval to have an index
                //need to add a tolerance since window query doens't include points directly on the end of the interval
                Interval win(Pure_interval(
                    Key(my_bbox.first[0]-tolerance,my_bbox.first[1]-tolerance,my_bbox.first[2]-tolerance),
                    Key(my_bbox.second[0]+tolerance, my_bbox.second[1]+tolerance,my_bbox.second[2]+tolerance)), 
                    index
                );

                tree->window_query(win, std::back_inserter(OutputList));
                intersections_indices.assign(interval_iterator(OutputList.begin()),
                                           interval_iterator(OutputList.end()));
            }
    };



    namespace AABBTree {
        class Triangles: public BboxIntersectionTest {
            typedef cgal_kernel::Point_3 Point;
            typedef cgal_kernel::Triangle_3 Datum; // CGAL 3D triangle type
            typedef CGAL::Bbox_3 Bbox_3;

            struct My_triangle {
                Point m_pa;
                Point m_pb;
                Point m_pc;
                size_t index;

                My_triangle(Point pa,
                    Point pb,
                    Point pc,
                    size_t idx
                ) : m_pa(pa), m_pb(pb), m_pc(pc), index(idx) {
                }
            };

            // the custom triangles are stored into a vector
            typedef std::vector<My_triangle>::const_iterator Iterator;

            // The following primitive provides the conversion facilities between
            // the custom triangle and CGAL ones
            struct My_triangle_primitive {
                public:
                    // this is the type of data that the queries returns
                    typedef const My_triangle* Id;
                    typedef cgal_kernel::Point_3 Point;
                    typedef cgal_kernel::Triangle_3 Datum; // CGAL 3D triangle type

                private:
                    Id m_pt; // this is what the AABB tree stores internally

                public:
                    My_triangle_primitive() {} // default constructor needed
                    // the following constructor is the one that receives the iterators from the
                    // iterator range given as input to the AABB_tree
                    My_triangle_primitive(Iterator it)
                        : m_pt(&(*it)) {}

                    const Id& id() const { return m_pt; }

                    // on the fly conversion from the internal data to the CGAL types
                    Datum datum() const
                    {
                        return Datum((m_pt->m_pa),
                            (m_pt->m_pb),
                            (m_pt->m_pc));
                    }

                    // returns a reference point which must be on the primitive
                    Point reference_point() const
                    { return (m_pt->m_pa); }
            };

            typedef CGAL::AABB_traits<cgal_kernel, My_triangle_primitive> My_AABB_traits;
            typedef CGAL::AABB_tree<My_AABB_traits> Tree;

            private:
                Tree *tree;
                std::vector<My_triangle> *triangles;

                struct primitive_iterator : std::vector<Tree::Primitive_id>::const_iterator
                {
                    using base_class = std::vector<Tree::Primitive_id>::const_iterator;
                    using value_type = size_t;

                    using base_class::base_class;

                    value_type operator*() const {
                        auto const& src = base_class::operator*();
                        return (src->index);
                    }
                };
            public:
                Triangles() {}
                ~Triangles() {
                    delete tree;
                    delete triangles;
                }

                bool intersections_exact() { return true; } //intersection search

                void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
                    if((pts.size() % 3) !=0) {
                        std::cerr << "error. your point list size has to be divisible by 3 to insert triangles" << std::endl;
                        return;
                    }

                    //segfaults if we don't allocate it on the heap
                    triangles = new std::vector<My_triangle>();
                    for(int i = 0; i < pts.size(); i += 3) {
                        //need the points to vary in the x, y, and z dimensions
                        triangles->push_back(My_triangle(make_cgal_point(pts[i]),make_cgal_point(pts[i+1]),make_cgal_point(pts[i+2]),indices[i/3]));
                    }
                    tree = new Tree(triangles->begin(),triangles->end());
                }

                void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {

                    Bbox_3 bbox(my_bbox.first[0], my_bbox.first[1], my_bbox.first[2], 
                        my_bbox.second[0], my_bbox.second[1], my_bbox.second[2]);

                    std::vector<Tree::Primitive_id> primitives;
                    tree->all_intersected_primitives(bbox, std::back_inserter(primitives));
                    intersections_indices.assign(primitive_iterator(primitives.begin()),
                                               primitive_iterator(primitives.end()));
                }

        };


        class Bboxes : public BboxIntersectionTest {
            typedef boost::tuple<cgal_isobbox,uint32_t> cgal_isobbox_with_index;

            // the bboxes are stored into a vector
            typedef std::vector<cgal_isobbox_with_index>::const_iterator Iterator;

            // The following primitive provides the conversion facilities between
            // the custom triangle and CGAL ones
            struct My_bbox_primitive {
                public:
                    // this is the type of data that the queries returns
                    typedef const cgal_isobbox_with_index* Id;
                    typedef cgal_kernel::Point_3 Point;
                    typedef cgal_isobbox Datum; // CGAL 3D triangle type

                private:
                    Id m_pt; // this is what the AABB tree stores internally

                public:
                    My_bbox_primitive() {} // default constructor needed
                    // the following constructor is the one that receives the iterators from the
                    // iterator range given as input to the AABB_tree
                    My_bbox_primitive(Iterator it)
                        : m_pt(&(*it)) {}

                    const Id& id() const { return m_pt; }

                    // on the fly conversion from the internal data to the CGAL types
                    Datum datum() const
                    {
                        return (boost::get<0>(*m_pt) );
                    }

                    // returns a reference point which must be on the primitive
                    Point reference_point() const
                    { return (Point(boost::get<0>(*m_pt).xmin(),boost::get<0>(*m_pt).ymin(), boost::get<0>(*m_pt).zmin() )); }

                    Datum bbox() const 
                    {
                        return (boost::get<0>(*m_pt));
                    }
            };

            typedef CGAL::AABB_traits<cgal_kernel, My_bbox_primitive> My_AABB_traits;
            typedef CGAL::AABB_tree<My_AABB_traits> Tree;

            private:
                Tree *tree;
                std::vector<cgal_isobbox_with_index> *bboxes;

                struct primitive_iterator : std::vector<Tree::Primitive_id>::const_iterator
                {
                    using base_class = std::vector<Tree::Primitive_id>::const_iterator;
                    using value_type = size_t;

                    using base_class::base_class;

                    value_type operator*() const {
                        auto const& src = base_class::operator*();
                        return (boost::get<1>(*src) );
                    }
                };
            public:
                Bboxes() {}
                ~Bboxes() {
                    delete tree;
                    delete bboxes;
                }

                bool intersections_exact() { return true; } //intersection search

                void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
                    if((pts.size() % 2) !=0) {
                        std::cerr << "error. your point list size has to be divisible by 2 to insert bboxes" << std::endl;
                        return;
                    }

                    //segfaults if we don't allocate it on the heap, this is mentioned in the documentation (the iterator must be kept available for the lifetime of the tree)
                    bboxes = new std::vector<cgal_isobbox_with_index>();
                    for(int i = 0; i < pts.size(); i += 2) {
                        //need the points to vary in the x, y, and z dimensions
                        bboxes->push_back(boost::make_tuple(make_cgal_isobbox(pts[i], pts[i+1]),indices[i/2]));
                    }
                    tree = new Tree(bboxes->begin(),bboxes->end());
                }


                void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {

                    cgal_bbox bbox(my_bbox.first[0], my_bbox.first[1], my_bbox.first[2], 
                        my_bbox.second[0], my_bbox.second[1], my_bbox.second[2]);

                    std::vector<Tree::Primitive_id> primitives;
                    tree->all_intersected_primitives(bbox, std::back_inserter(primitives));
                    intersections_indices.assign(primitive_iterator(primitives.begin()),
                                               primitive_iterator(primitives.end()));
                }

        };
    }
}

#endif //CGAL_TEST_HH

