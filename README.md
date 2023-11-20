### Overview
Implementation of Red-Black search tree. 

<img src="https://github.com/RustamSubkhankulov/cpp-course/blob/main/rbtree/images/example.png" alt="tree" width="800">

### Cloning & Building
 - Firstly, copy source files or clone this repository: <code>git clone git@github.com:RustamSubkhankulov/rbtree.git</code>.
 - Secondly, change current working directory: 
 <code>cd rbtree</code>
 - Thirdly, build tests:
   - <code> cmake -B build </code>
   - <code> cmake --build build [--target <target_name>] </code>, where <code><target_name></code> is available test option (see below).
   Omitting <code>--target</code> option will cause all targets to build. 
   Executables are located in <code>./build/bin</code> subdirectory.

### Testing
Available test options are:
 1. Interactive end-to-end tests: <code>custom_query</code> and <code>sdtset_query</code> - builds executables in <code>./build/bin</code> subdirectory, that should be executed explicitly by user. Interactive test reads input from stdin and performs range queries of two types: 'k' - insertion  - and 'q' - distance. Insertion query takes one int argument - this argument is inserted in tree. Distance query takes two arguments and calculates distance between two elements in tree. Second element must be greater or equal than the first. Result is written out to stdout. RBTREE::rbtree and std::set are used in <code>custom_query</code> and <code>stdset_query</code> tests accordingly.
 Example of input: <code>k 10 k 20 k 30 q 10 30 q 20 20 q 10 20</code>
 Output in this case will be: <code>2 0 1 </code>

 Options, available for configuring interactive testing:
 - <code>STDDIST</code> - enables use of std::distance for q-queries instead of fast distance implementation in <code>custom_query</code> target.
 - <code>MEASURE_TIME</code> - disables output of results on interactive test and enables time measurement with std::chrono features. Works for both <code>custom_query</code> and <code>stdset_query</code> tests. Instead of result of queries, test will show elapsed time. Results of queries are written to file <code>res.txt</code>. Example of output: <code> Elapsed time: 1 ms 527 µs 166 ns </code>

 2. <code>unit</code> and <code>query</code> - these tests are implemented with GoogleTest. To run tests, following commands should be run after building targets: 
  - <code>cd build</code>
  - <code>ctest</code>

  Unit testing performs separate test for each interface method of RBTREE::rbtree.
  Query testings tests queries methods, that are used in interactive testing mode.

### Debug features
1. Graphical dump. To make graphical dump, use <code>graph_dump()</code> RBTREE::rbtree method. This method is overloaded. One its overlod takes one argument - name of the output image file, relative to the current working directory, another - std::basic_ostream, where dot graphical dump will be written to.
2. Debug compilation flags. Enabled by option <code>'DEBUG_GLAGS'</code>. Enables additional warnings during compilation. Forcefully disabled with <code>CMAKE_BUILD_TYPE=RELEASE</code>.

### Performance comparison
RBTREE::rbtree provides additional features for fast computing of distance between two nodes. Tree method <code>distance()</code> takes two arguments - two iterators to elements in tree or two keys. This method uses subtree sizes, stored in nodes of the tree for faster calculation of distance comparing to std::distance. 

For comparison, python script, that generates queries sequences, is written (scripts/query_gen.py). This scripts takes several args: number of elements in tree and name of the output file, where queries will be written. For example, if first arg is 10, 10 insertion k-queries for numbers from 0 to 9 including and distance q-queries for every pair of elements (except pairs where first and last are equal) will be generated. 

Script does not generate random sequence of queries, that include both insertion and distance mixed up. Methods for generating a sequence of queries were chosen in such a way as to most clearly show the difference between std::distance and RBTREE::rbtree::distance.

For measuring time of queries executing <code>std::chrono::steady_clock</code>> was used. 
For measuting total execution time cli <code>time</code> util was used.

Results of comparison:

| N, number of elements | RBTREE::rbtree::distance | std::distance |
|-----------------------|--------------------------|---------------|
| 200                   | 0,072                    | 0,128         |
| 500                   | 0,633                    | 0,743         |
| 750                   | 1,047                    | 1,286         |
| 1.000                 | 1,833                    | 2,498         |
| 1.500                 | 4,290                    | 6,185         |
| 2.000                 | 8,010                    | 12,626        |
| 2.500                 | 12,299                   | 21,509        |
| 3.000                 | 18,234                   | 37,474        |
| 3.500                 | 24,756                   | 58,494        |
| 5.000                 | 50,709                   | 158,41        |
| 7.500                 | 175,15                   | 513,59        |

<img src="https://github.com/RustamSubkhankulov/rbtree/blob/main/images/graph.png" alt="graph" width="800">
