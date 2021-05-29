#ifndef PCL_TEST_HH
#define PCL_TEST_HH

#include <math.h>       /* cbrt, round */
#include <pcl/point_cloud.h>
#include <pcl/octree/octree_search.h>
#include <pcl/kdtree/kdtree_flann.h>

#ifdef USE_GPU
    #include <pcl/gpu/octree/octree.hpp>
    #include <pcl/gpu/containers/impl/device_array.hpp>
    #include <pcl/memory.h>
    #include <pcl/point_types.h>
    #include <set>
#endif

#ifndef NUM_ELEMS_PER_NODE
    #error Your need to define NUM_ELEMS_PER_NODE in a common header file
#endif 


using namespace std;


namespace TestPCL {

        class Octree : public BboxIntersectionTest {
            private:
                pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> *tree;
                pcl::PointCloud<pcl::PointXYZ>::Ptr cloud;

            public: 
                bool intersections_exact() { return true ;} //get_intersections is exact, but get radius (a bonus function we won't need to use) isn't

                Octree() {}
                ~Octree() {
                    cloud->points.clear ();
                    tree->deleteTree();
                    delete tree;
                    // delete cloud;
                }

                void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, double leaf_volume, size_t bucket_size=0) {
                    if(leaf_volume <= 0) {
                        cerr << "TestPCL::Octree.build_tree error. leaf_volume must be a positive number" << endl;
                        exit(-1);
                    }
                    cloud =  pcl::PointCloud<pcl::PointXYZ>::Ptr(new pcl::PointCloud<pcl::PointXYZ>());
                    //indicates there will be no infinite or NaN values
                    cloud->is_dense = true;

                    for(const point &pt : pts) {
                        cloud->push_back(pcl::PointXYZ(pt[0],pt[1],pt[2]));
                    }
                    cloud ->push_back(pcl::PointXYZ(0,0,0));
                    tree = new pcl::octree::OctreePointCloudSearch<pcl::PointXYZ>(std::round(std::cbrt(leaf_volume)));

                    tree->setInputCloud(cloud);
                    
                    if(bucket_size != 0) {
                        //throws an error when trying to use that the developers could not explain on git
                        // tree->defineBoundingBox (); 
                        tree->enableDynamicDepth(bucket_size);
                    }
                    tree->addPointsFromInputCloud();
                }


                void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
                    build_tree(pts, indices, NUM_ELEMS_PER_NODE);
                }

                void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
                    Eigen::Vector3f bbox_min = Eigen::Vector3f (my_bbox.first[0], my_bbox.first[1], my_bbox.first[2]);
                    Eigen::Vector3f bbox_max = Eigen::Vector3f (my_bbox.second[0], my_bbox.second[1], my_bbox.second[2]);

                    //has to be ints not size_t
                    std::vector<int> indices;
                    tree->boxSearch (bbox_min, bbox_max, indices);
                    std::copy(indices.begin(), indices.end(), std::back_inserter(intersections_indices));
                }

                void get_radius(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {

                    pcl::PointXYZ searchPoint;
                    point mid_pt;
                    double radius_search_bound = 0;
                    //uses radius not squared radius
                    get_max_radius(my_bbox, mid_pt, radius_search_bound);
                    //needs slight tolerance to find points exactly on the edge
                    radius_search_bound += DEFAULT_TOLERANCE;

                    searchPoint.x = mid_pt[0];
                    searchPoint.y = mid_pt[1];
                    searchPoint.z = mid_pt[2];

                    //has to be ints and floats, not size_t and doubles
                    std::vector<int> indices;
                    std::vector<float> distances;
  
                    tree->radiusSearch (searchPoint, radius_search_bound, indices, distances);
                    std::copy(indices.begin(), indices.end(), std::back_inserter(intersections_indices));
                }
        };

        //is just a thin wrapper around FLANN. no reason to think it'll be better
        class KDTree : public BboxIntersectionTest {
            private:
                pcl::KdTreeFLANN<pcl::PointXYZ> *tree;

            public: 
                bool intersections_exact() { return false ;} //circular radius with tolerance is not exact

                KDTree() {}
                ~KDTree() {
                    delete tree;
                }

                //library doesn't support setting a leaf size
                void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
                    bool sort = false;
                    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
                    //indicates there will be no infinite or NaN values
                    cloud->is_dense = true;
                    for(const point &pt : pts) {
                        cloud->push_back(pcl::PointXYZ(pt[0],pt[1],pt[2]));
                    }
                    tree = new pcl::KdTreeFLANN<pcl::PointXYZ>(sort);
                    tree->setInputCloud(cloud);
                }

                void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {

                    pcl::PointXYZ searchPoint;
                    point mid_pt;
                    double radius_search_bound = 0;
                    //uses radius not squared radius
                    get_max_radius(my_bbox, mid_pt, radius_search_bound);
                    //needs slight tolerance to find points exactly on the edge
                    radius_search_bound += DEFAULT_TOLERANCE;

                    searchPoint.x = mid_pt[0];
                    searchPoint.y = mid_pt[1];
                    searchPoint.z = mid_pt[2];

                    //has to be ints and floats, not size_t and doubles
                    std::vector<int> indices;
                    std::vector<float> distances;
  
                    tree->radiusSearch (searchPoint, radius_search_bound, indices, distances);
                    std::copy(indices.begin(), indices.end(), std::back_inserter(intersections_indices));
                }
        };

    #ifdef USE_GPU
        class OctreeGPU : public BboxIntersectionTest {
            private:
                pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> *tree;
                pcl::gpu::Octree *octree_device;
                size_t tree_size;

            public: 
                bool intersections_exact() { return false ;} //circular radius with tolerance is not exact

                OctreeGPU() {}
                ~OctreeGPU() {
                    delete tree;
                    delete octree_device;
                }

                //doesn't allow you to set the leaf size when using a gpu
                void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
                    tree_size = pts.size();
                    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
                    //indicates there will be no infinite or NaN values
                    cloud->is_dense = true;
                    for(const point &pt : pts) {
                        cloud->push_back(pcl::PointXYZ(pt[0],pt[1],pt[2]));
                    }

                    pcl::gpu::Octree::PointCloud cloud_device;
                    cloud_device.upload(cloud->points);

                    octree_device = new pcl::gpu::Octree();
                    octree_device->setCloud(cloud_device);
                    octree_device->build();
                }


                //note - gpu octre doesn't support box search like the non-gpu variant does
                //by default we won't do query domain decomposition
                void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices, 
                    size_t num_sub_queries, size_t x_queries, size_t y_queries, size_t z_queries) {
                    if(x_queries*y_queries*z_queries != num_sub_queries) {
                        std::cerr << "error. you have provided a query domain decompoistion where x_queries*y_queries*z_queries != total_queries" << std::endl;
                        return;
                    }
       
                    std::vector<pcl::PointXYZ> query_host;
                    query_host.resize (num_sub_queries);

                    vector<double> subquery_bbox_len = {
                        (my_bbox.second[0]-my_bbox.first[0])/x_queries, 
                        (my_bbox.second[1]-my_bbox.first[1])/y_queries, 
                        (my_bbox.second[2]-my_bbox.first[2])/z_queries
                    };

                    for(size_t i = 0; i < num_sub_queries; i++) {
                        size_t x_pos = i % x_queries;
                        size_t y_pos = (i / x_queries) % y_queries; 
                        size_t z_pos = ((i / (x_queries * y_queries)) % z_queries);

                        double x_offset = x_pos * subquery_bbox_len[0] + my_bbox.first[0];
                        double y_offset = y_pos * subquery_bbox_len[1] + my_bbox.first[1];
                        double z_offset = z_pos * subquery_bbox_len[2] + my_bbox.first[2];

                        //want the midpoints
                        query_host[i].x = x_offset + subquery_bbox_len[0]*.5;
                        query_host[i].y = y_offset + subquery_bbox_len[1]*.5;
                        query_host[i].z = z_offset + subquery_bbox_len[2]*.5;
                    }
                    pcl::gpu::Octree::Queries queries_device;
                    queries_device.upload(query_host);  

                    point mid_pt;
                    double radius_search_bound = 0;
                    bbox sub_bbox = bbox(my_bbox.first, 
                        point({
                            my_bbox.first[0]+subquery_bbox_len[0],                        
                            my_bbox.first[1]+subquery_bbox_len[1],
                            my_bbox.first[2]+subquery_bbox_len[2]
                        })
                    );
                    //uses radius not squared radius
                    get_max_radius(sub_bbox, mid_pt, radius_search_bound);
                    radius_search_bound += DEFAULT_TOLERANCE;

                    std::vector<float> radius;
                    for(int i = 0; i < num_sub_queries; i++) {
                        radius.push_back(radius_search_bound);
                    }
                    pcl::gpu::Octree::Radiuses radiuses_device;
                    radiuses_device.upload(radius);

                    //every point could conceivably match
                    const int max_answers = tree_size;

                    // Output buffer on the device
                    pcl::gpu::NeighborIndices result_device(queries_device.size(), max_answers);

                    // Do the actual search
                    octree_device->radiusSearch(queries_device, radiuses_device, max_answers, result_device);

                    std::vector<int> sizes, data;
                    result_device.sizes.download(sizes);
                    result_device.data.download(data);

                    if(num_sub_queries > 1) {
                        //use a set since there could be duplicates when performing multuple sub-queries in parallel
                        std::set<size_t> result_indices;
                        for (std::size_t i = 0; i < sizes.size (); ++i) {
                            for (std::size_t j = 0; j < sizes[i]; ++j) {
                                //have to use a set since there could be overlap in the results
                                result_indices.insert(data[j+ i * max_answers]);
                            }
                        }
                        std::copy(result_indices.begin(), result_indices.end(), std::back_inserter(intersections_indices));
                    }
                    else {
                        for (std::size_t i = 0; i < sizes.size (); ++i) {
                            for (std::size_t j = 0; j < sizes[i]; ++j) {
                                intersections_indices.push_back(data[j+ i * max_answers]);
                            }
                        }
                    }
                }

                void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
                    size_t num_sub_queries=1, x_queries=1, y_queries=1, z_queries=1;
                    get_intersections(my_bbox, intersections_indices, num_sub_queries, x_queries, y_queries, z_queries);
                }

        };
    #endif
}

#endif //PCL_TEST_HH

 