#ifndef PERFORM_QUERIES
#define PERFORM_QUERIES

#include "common.hh"

void perform_queries(BboxIntersectionTest *test, const std::string &test_name,
    const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config, QueryType query_type=STANDARD,
    const std::vector<std::vector<size_t>> &element_node_ids = std::vector<std::vector<size_t>>());

#endif //PERFORM_QUERIES