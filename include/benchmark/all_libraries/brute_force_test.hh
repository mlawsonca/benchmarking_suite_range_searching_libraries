#ifndef BRUTE_FORCE_HH
#define BRUTE_FORCE_HH

using namespace std;

class TestBruteForce : public BboxIntersectionTest {


    private:
        vector<point> tree;

        bool point_intersect(const bbox &query, const point &pt) {
            return(
                   query.first[0] <= pt[0] && pt[0] <= query.second[0] 
                && query.first[1] <= pt[1] && pt[1] <= query.second[1] 
                && query.first[2] <= pt[2] && pt[2] <= query.second[2] 
            );
        }

        bool bbox_intersect(const bbox &query, const point &min_corner, const point &max_corner) {
            //make sure the query min is less than the box's max and the box's min is less than the query max
            return(
                   query.first[0] <= max_corner[0] && min_corner[0] <= query.second[0] 
                && query.first[1] <= max_corner[1] && min_corner[1] <= query.second[1] 
                && query.first[2] <= max_corner[2] && min_corner[2] <= query.second[2] 
            );
        }


    public:

        bool intersections_exact() { return true; } //brute force check

        TestBruteForce() {}
        ~TestBruteForce() {
        }


        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            tree = pts;
            size_t total_size = 0;
            for(int i = 0; i < pts.size(); i++) {
                for(int j = 0; j < pts[i].size(); j++) {
                    total_size += sizeof(pts[i][j]);
                }
            }
        }

        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
            intersections_indices.reserve(tree.size());
            for(int i = 0; i < tree.size(); i++) {
                if(point_intersect(my_bbox, tree[i])) {
                    intersections_indices.push_back(i);
                }
            }
        }

        void get_intersections_bboxes(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
            intersections_indices.reserve(tree.size());
            for(int i = 0; i < tree.size(); i+=2) {
                if(bbox_intersect(my_bbox, tree[i], tree[i+1])) {
                    //indices were inserted as i/2 (since there are two points to a bbox)
                    intersections_indices.push_back(i/2);
                }
            }
        }

};

#endif //BRUTE_FORCE_HH


