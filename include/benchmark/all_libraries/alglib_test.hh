#ifndef ALGLIB_TEST_HH
#define ALGLIB_TEST_HH


#include "alglibmisc.h"

class TestAlglib : public BboxIntersectionTest {
    private:
        //can't do new/pointer, all construction is handled by the library internally
        alglib::kdtree tree;
    public:
        bool intersections_exact() { return true; } //bounding box search

        TestAlglib() {}
        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            int num_rows = pts.size();
            int num_cols = pts[0].size();
            int num_extra_values = 0;
            alglib::ae_int_t normtype = 2;

            alglib::integer_1d_array tags;
            tags.setcontent(indices.size(), (long*)&indices[0]);

            alglib::real_2d_array array;
            array.setlength(num_rows, num_cols);
            for (int i = 0; i < num_rows; i++ ) {
                for (int j = 0; j < num_cols; j++ ) {
                    array(i,j) = pts[i][j];
               }
            }
            alglib::kdtreebuildtagged(array, tags, num_rows, num_cols, num_extra_values, normtype, tree);

        }
        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
            alglib::real_1d_array lower_corner;
            alglib::real_1d_array upper_corner;
            alglib::integer_1d_array result_tags;
            alglib::ae_int_t num_results;

            lower_corner.setcontent(my_bbox.first.size(), &my_bbox.first[0]);
            upper_corner.setcontent(my_bbox.second.size(), &my_bbox.second[0]);

            num_results = alglib::kdtreequerybox(tree, lower_corner, upper_corner);
            alglib::kdtreequeryresultstags(tree, result_tags);
            // intersections_indices.reserve(num_results);
            intersections_indices.assign(result_tags.getcontent(), result_tags.getcontent()+num_results);
        }
};

#endif //ALGLIB_TEST_HH