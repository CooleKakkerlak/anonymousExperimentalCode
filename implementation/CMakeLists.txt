cmake_minimum_required(VERSION 3.12...4.2)
project( chromatic_k_nearest_neighbours )


set(EXECUTABLE_OUTPUT_PATH "bin")
set(CMAKE_CXX_STANDARD 17)
find_package(CGAL REQUIRED)

add_executable(chromatic_k_nearest_neighbours
src/myRangeTree/myRangeTree.cpp src/myRangeTree/myRangeTree.h

src/shared/defs.cpp src/shared/defs.h

src/tests/dataReader.h src/tests/dataReader.cpp src/tests/testFuncs.cpp src/tests/testFuncs.h
src/tests/1d.cpp src/tests/1d.h src/tests/2d.cpp src/tests/2d.h

src/1D/arr_print.h src/1D/blocksFuncs.h src/1D/blocksFuncs.cpp 
src/1D/mode_arrangement.cpp src/1D/mode_arrangement.h src/1D/mode_chan.cpp src/1D/mode_chan.h src/1D/mode_rangeTree.h src/1D/mode_rangeTree.cpp src/1D/range_query.cpp src/1D/range_query.h

src/2D/mode_naive.h src/2D/mode_naive.cpp    src/2D/range_query.cpp src/2D/range_query.h
    
src/main.cpp
 src/2D/grid_simple.h src/2D/grid_simple.cpp )

target_link_libraries(chromatic_k_nearest_neighbours PUBLIC CGAL::CGAL)