#ifndef THREEDTK_TEST_HH
#define THREEDTK_TEST_HH

#include "slam6d/kdparams.h"
#include "3DTK_kdTreeImpl_modified.hh"


//don't want to use threading
#undef WITH_OPENMP_KD


struct IndexAccessor {
    inline point operator() (const std::vector<point> &data, size_t index) {
        return data[index];
    }
};


struct ParamAccessor {
    inline size_t operator() (const std::vector<point> &data, size_t index) {
        return index;
    }
};


template<class PointData, class AccessorData, class AccessorFunc, class PointType, class ParamFunc>
KDParams<PointType> KDTreeImpl<PointData, AccessorData, AccessorFunc, PointType, ParamFunc>::params[MAX_OPENMP_NUM_THREADS];
class KDtreeIndexed : protected KDTreeImpl<std::vector<point>, size_t, IndexAccessor, size_t, ParamAccessor>
{
public:

    KDtreeIndexed() {}

    KDtreeIndexed(const std::vector<point> &pts, const std::vector<size_t> &indices, int bucketSize = 20) 
        : m_indices(indices) //if we just copy the ref it will change the indices values below
    {
        m_data = pts;
        create(pts, (size_t*)&m_indices[0], pts.size(), bucketSize);
    }

    ~KDtreeIndexed() { }

    std::vector<size_t> AABBSearch(point pt0, point pt1, vector<size_t> &result)
    {
        if (pt0[0] > pt1[0] || pt0[1] > pt1[1] || pt0[2] > pt1[2]) {
            throw std::logic_error("invalid bbox");
        }

        int threadNum = 0;
        params[threadNum].p = &pt0[0];
        params[threadNum].p0 = &pt1[0];
        params[threadNum].range_neighbors.clear();
        _AABBSearch(m_data, threadNum);

        std::copy(params[threadNum].range_neighbors.begin(), params[threadNum].range_neighbors.end(), std::back_inserter(result)); 

      return result;
    }


protected:
  std::vector<point> m_data;
  std::vector<size_t> m_indices;



};


class Test3DTK : public BboxIntersectionTest {
    private:
        KDtreeIndexed *tree;

    public:
        bool intersections_exact() { return true; } //bounding box search

        Test3DTK() {}
        ~Test3DTK() {
            delete tree;
        }
        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            tree = new KDtreeIndexed(pts, indices);
        }
        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size) {
            tree = new KDtreeIndexed(pts, indices, bucket_size);
        }
        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
            tree->AABBSearch((point)my_bbox.first, (point)my_bbox.second, intersections_indices);

        }
};

#endif

