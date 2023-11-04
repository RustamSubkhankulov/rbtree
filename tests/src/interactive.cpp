#include <ios>
#include <set>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <istream>
#include <iterator>
#include <iostream>
#include <algorithm>

#include "rbtree.hpp"
#include "query.hpp"

#include "interactive_conf.hpp"

using namespace RBTREE;

using tree = rbtree<int>;
using diff_t = typename tree::difference_type;

using namespace std::chrono;

namespace {
  
  class query_seq {

    private:

      std::vector<int> vec;
      unsigned long ind = 0;

    public:

      bool ind_check() { return (ind < vec.size()); }

      /* Get next query type from qeury sequence. */
      bool get_query_type(query_type& arg);

      /* Get next query arg from qeury sequence. */
      bool get_query_arg(int& arg);

      friend std::istream& operator>>(std::istream& is, query_seq& q_seq);
  };

  #if defined(MEASURE_TIME)
    /* Pretty print for duration.  */
    using duration_type = decltype(high_resolution_clock::now() - high_resolution_clock::now());
    void print_elapsed(std::ostream& os, duration_type elapsed);
  #endif 

  void print_results(std::ostream& os, const std::vector<diff_t>& results) {
    std::copy(results.begin(), results.end(), std::ostream_iterator<diff_t>(os, " "));
  }

}; /* anonymous namespace */

int main() {

  /* 
   * Construct empty tree - RBTREE::rbtree.
   */
  RBTREE:rbtree<int> set;

  /* Read sequence of queries. */
  query_seq q_seq;
  if (!(std::cin >> q_seq) && !std::cin.eof()) {

    std::cerr << "Invalid input. Input format: ( q|k (int)* )* \n";
    return EXIT_FAILURE;
  }

  /* Vector for results of queries. */
  std::vector<diff_t> results;

  /* Start time measurement of total elapsed time period if option is enabled. */
  #if defined(MEASURE_TIME)
    auto begin = steady_clock::now();
  #endif

  while(q_seq.ind_check()) {

    query_type qtype;
    if (!q_seq.get_query_type(qtype)) {
      return EXIT_FAILURE;
    }

    switch (qtype) {

      /* Insertion query - 'k'. */
      case query_type::K_INSERT: {

        /* Read k-query arg. */
        int key;
        if (!q_seq.get_query_arg(key)) {
          return EXIT_FAILURE;
        }

        /* Perform query. */
        query_k_insert(set, key);
        break;
      }

      /* Distance query - 'q'. */
      case query_type::Q_DISTANCE: {

        /* Read q-query two args. */
        int first, second;
        if (!(q_seq.get_query_arg(first) && q_seq.get_query_arg(second))) {
          return EXIT_FAILURE;
        }

        diff_t res;

        /* Perform query and write out result. */
        #if defined(STDDIST)
          res = query_q_distance(set, first, second);
        #else 
          res = query_q_distance_fast(set, first, second);
        #endif

        results.push_back(res);        
        break;
      }

      default: {
        std::cerr << "Invalid query type. "
                  << "Supported are 'k' and 'q'. \n";
        return EXIT_FAILURE;
      }
    }
  }

  /* Print elapsed time if option is enabled. */
  #if defined(MEASURE_TIME)
    print_elapsed(std::cout, steady_clock::now() - begin);
  #endif

  #if !defined(MEASURE_TIME)
    print_results(std::cout, results);
  #else 
    std::ofstream results_file("res.txt");
    if (!results_file.is_open()) {
      std::cerr << "Failed to open file for writing results. " << std::endl;
    } else {
      print_results(results_file, results);
    }
  #endif

  return EXIT_SUCCESS;
}

namespace {

std::istream& operator>>(std::istream& is, query_seq& q_seq) {

  while (!is.eof()) {

    char qtype;

    if (!(is >> qtype)) {
      return is;
    }

    q_seq.vec.push_back(qtype);

    switch(static_cast<query_type>(qtype)) {

      case query_type::K_INSERT: {

        int key;
        if (!(is >> key)) {
          return is;
        }

        q_seq.vec.push_back(key);
        break;
      }

      case query_type::Q_DISTANCE: {

        int first, second;

        if (!(is >> first >> second)) {
          return is;
        }

        q_seq.vec.push_back(first);
        q_seq.vec.push_back(second);
        break;
      }

      default: {
        is.clear(std::istream::failbit);
        return is;
      }
    }
  }

  return is;
}

bool query_seq::get_query_type(query_type& arg) {

  if (!ind_check()) {
    return false;
  }

  arg = static_cast<query_type>(vec[ind++]);
  return true;
}

bool query_seq::get_query_arg(int& arg) {

  if (!ind_check()) {
    return false;
  }

  arg = vec[ind++];
  return true;
}

#if defined(MEASURE_TIME)
  void print_elapsed(std::ostream& os, duration_type elapsed) {
    
    auto secnd = duration_cast<seconds>(elapsed);
    auto milli = duration_cast<milliseconds> (elapsed -= secnd);
    auto micro = duration_cast<microseconds>(elapsed -= milli);
    auto nano  = duration_cast<nanoseconds> (elapsed -= micro);

    os << "Elapsed time: ";

    if (secnd.count() != 0) {
      os << secnd.count() << " sec ";
    }

    if (milli.count() != 0) {
      os << milli.count() << " ms ";
    }

    if (micro.count() != 0) {
      os << micro.count() << " µs ";
    }

    os << nano.count() << " ns" << std::endl;
  }
#endif

}; /* Anonymous namespace. */