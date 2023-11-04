### Overview
Implementation of Red-Black search tree. 

<img src="https://github.com/RustamSubkhankulov/cpp-course/blob/main/rbtree/images/example.png" alt="tree" width="800">

### Cloning & Building
 - Firstly, copy source files or clone this repository: <code>git clone git@github.com:RustamSubkhankulov/cpp-course.git</code>.
 - Secondly, change current working directory: 
 <code>cd rbtree</code>
 - Thirdly, build tests:
   - <code> cmake -B build </code>
   - <code> cmake --build build [--target <target_name>] </code>, where <code><target_name></code> is available test option (see below).
   Omitting <code>--target</code> option will cause all targets to build. 
   Executables are located in <code>./build/bin</code> subdirectory.

### Testing
Available test options are:
 1. <code>interactive</code>: interactive test - builds executable in <code>./build/bin</code> subdirectory, that should be executed explicitly by user. Interactive test reads input from stdin and performs range queries of two types: 'k' - insertion  - and 'q' - distance. Insertion query takes one int argument - this argument is inserted in tree. Distance query takes two arguments and calculates distance between two elements in tree. Second element must be greater or equal than the first. Result is written out to stdout. 
 Example of input: <code>k 10 k 20 k 30 q 10 30 q 20 20 q 10 20</code>
 Output in this case will be: <code>2 0 1 </code>

 Options, available for configuring interactive testing:
 - <code>STDDIST</code> - enables use of std::distance for q-queries instead of fast distance implementation.
 - <code>MEASURE_TIME</code> - disables output of results on interactive test and enables time measurement with std::chrono features. Instead of result of queries, test will show elapsed time. Results of queries are written to file <code>res.txt</code>. Example of output: <code> Elapsed time: 1 ms 527 µs 166 ns </code>

 2. <code>unit</code> and <code>query</code> - these tests are implemented with GoogleTest. To run tests, following commands should be run after building targets: 
  - <code>cd build</code>
  - <code>ctest</code>

  Unit testing performs separate test for each interface method of RBTREE::rbtree.
  Query testings tests queries methods, that are used in interactive testing mode.

### Debug features
1. Graphical dump. To make graphical dump, use <code>graph_dump()</code> RBTREE::rbtree method. This method takes one argument - name of the output image file, relative to the current working directory.
2. Debug compilation flags. Enabled by option <code>'DEBUG_GLAGS'</code>. Enables additional warnings during compilation. Forcefully disabled with <code>CMAKE_BUILD_TYPE=RELEASE</code>.

### Performance comparison
RBTREE::rbtree provides additional features for fast computing of distance between two nodes. Tree method <code>distance()</code> takes two arguments - two iterators to elements in tree or two keys. This method uses subtree sizes, stored in nodes of the tree for faster calculation of distance comparing to std::distance. 

For comparison, python script, that generates queries sequences, is written (scripts/query_gen.py). This scripts takes several args: number of elements in tree, number of repetitions - how many times distance between the smallest and the biggest elements in tree is calculated - and name of the output file, where queries will be written. For example, if first arg is 10 and second is 5, 10 insertion k-queries for numbers from 0 to 9 including and 5 distance q-queries for calculating distance between 0 and 9 will be generated. 

Script does not generate random sequence of queries, that include both insertion and distance mixed up. Methods for generating a sequence of queries were chosen in such a way as to most clearly show the difference between std::distance and RBTREE::rbtree::distance.

For time measurement <code>std::chrono::steady_clock</code>> was used. 

Results for 50000 elements and 10000 calculations of distance are:

| №       | RBTREE::rbtree::distance | std::distance |
|---------|--------------------------|---------------|
| 1       | 14,397417458             | 49,430048042  |
| 2       | 14,591262333             | 56,628411958  |
| 3       | 14,864323542             | 54,779358625  |
| 4       | 13,976344458             | 55,036148542  |
| 5       | 14,236744833             | 56,205813708  |
| 6       | 13,076909209             | 56,708945917  |
| average | 14,190500306             | 54,798121132  |

RBTREE::rbtree::distance has shown result nearly 3,9 times better than std::distance.
