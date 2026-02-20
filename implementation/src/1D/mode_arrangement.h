#pragma once

#include "../shared/defs.h"
#include "mode_rangeTree.h"
#include "blocksFuncs.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Arr_linear_traits_2.h>
#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arr_dcel_base.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/basic.h>
#include <CGAL/Arr_batched_point_location.h>
#include <CGAL/Arr_trapezoid_ric_point_location.h>
#include <CGAL/Arr_naive_point_location.h>
#include <CGAL/Arr_landmarks_point_location.h>


// arrangement based implementation

struct ConflictList {
	std::set<int> conflictingRayIndices;
	std::set<Color_> conflictingPointColors;
};

struct FaceData {
	int faceIndex = - 1;
	ColorCount mode;
	ConflictList conflictList;
};

typedef CGAL::Arr_linear_traits_2<Kernel> Traits;
using Dcel = CGAL::Arr_extended_dcel<Traits, bool, bool, FaceData>;
typedef CGAL::Arrangement_2<Traits, Dcel> Arrangement_2;

using Trapezoid_pl = CGAL::Arr_trapezoid_ric_point_location<Arrangement_2>;
using Naive_pl = CGAL::Arr_naive_point_location<Arrangement_2>;
using Landmarks_pl = CGAL::Arr_landmarks_point_location<Arrangement_2, Kernel>;
typedef Trapezoid_pl PointLocation; //change this to switch point location strategy
using Point_location_result = CGAL::Arr_point_location_result<Arrangement_2>;
using Query_result = std::pair<Point_2, Point_location_result::Type>;

typedef Arrangement_2::Face_handle Face_handle;
typedef Arrangement_2::Face_const_handle Face_const_handle;
typedef Arrangement_2::Halfedge_handle Halfedge_handle;
typedef Arrangement_2::Halfedge_const_handle Halfedge_const_handle;
typedef Arrangement_2::Vertex_handle Vertex_handle;
typedef Arrangement_2::Vertex_const_handle Vertex_const_handle;


struct AAS1DModeArrangementDS : AAS1DModeDS {

private:
	std::vector<Color_> colors;
	std::vector<Ray_2> rays;
	Arrangement_2 arrangement;
	std::unique_ptr<PointLocation> pointLocation;
	int maxConflictListSize;
	std::vector<RangeTree1D> rangeTrees;

	void computeFunctionRays();
	void computeFaceData();
	bool isValidCutting(int s, double smudge_factor = 2, int constant_smudge = 5);

	void checkUnderRays(std::map<Color_, int>& underColorCounts, std::vector<bool>& currentlyUnder, Face_handle curFace);

public:
	ColorCount queryMode(const Range& query) override;
	long getDSMemoryUsage() override;
	std::string getName() override;
	AAS1DModeArrangementDS(std::vector<Color_> colors, int numColors, int s, std::mt19937& generator);
	AAS1DModeArrangementDS(AAS1DModeArrangementDS&&) = delete;
	AAS1DModeArrangementDS& operator=(AAS1DModeArrangementDS&&) = delete;
};