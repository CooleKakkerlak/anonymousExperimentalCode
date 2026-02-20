cmake_minimum_required(VERSION 3.12...4.2)
project( chromatic_k_nearest_neighbours )


set(EXECUTABLE_OUTPUT_PATH "bin")
set(CMAKE_CXX_STANDARD 17)
find_package(CGAL REQUIRED)

add_executable(chromatic_k_nearest_neighbours
src/amc/grid/cell.cpp src/amc/grid/cell.hpp src/amc/grid/grid.cpp src/amc/grid/grid.hpp src/amc/grid/range_trees.cpp src/amc/grid/range_trees.hpp
src/amc/grid/variant0.cpp src/amc/grid/variant1.cpp src/amc/grid/variant2.cpp src/amc/grid/variant3.cpp src/amc/grid/variant4.cpp
src/amc/amc.cpp src/amc/amc.h

src/myRangeTree/myRangeTree.cpp src/myRangeTree/myRangeTree.h

src/shared/defs.cpp src/shared/defs.h

src/tests/dataReader.h src/tests/dataReader.cpp src/tests/testFuncs.cpp src/tests/testFuncs.h
src/tests/1d.cpp src/tests/1d.h src/tests/2d.cpp src/tests/2d.h

src/1D/arr_print.h src/1D/blocksFuncs.h src/1D/blocksFuncs.cpp 
src/1D/mode_arrangement.cpp src/1D/mode_arrangement.h src/1D/mode_chan.cpp src/1D/mode_chan.h src/1D/mode_rangeTree.h src/1D/mode_rangeTree.cpp src/1D/range_query.cpp src/1D/range_query.h

src/2D/mode_naive.h src/2D/mode_naive.cpp src/2D/mode_query.cpp  src/2D/mode_query.h src/2D/range_query.cpp src/2D/range_query.h
    
src/main.cpp
src/ranking.cpp src/ranking.hpp
)

target_link_libraries(chromatic_k_nearest_neighbours PUBLIC CGAL::CGAL)