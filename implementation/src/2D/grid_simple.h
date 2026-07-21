#pragma once
#include <vector>
#include "../shared/defs.h"
#include "../myRangeTree/myRangeTree.h"

struct GridIndex {
public:
	int xi, xj, yi, yj;

	GridIndex(int xi, int xj, int yi, int yj);

	bool operator==(const GridIndex& other) const {
		return xi == other.xi &&
			xj == other.xj &&
			yi == other.yi &&
			yj == other.yj;
	}
};

namespace std {
	template <>
	struct hash<GridIndex> {
		size_t operator()(const GridIndex& g) const {
			size_t h = std::hash<int>{}(g.xi);
			h ^= std::hash<int>{}(g.xj) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<int>{}(g.yi) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<int>{}(g.yj) + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};
}

struct AAS2DModeGridDS : AAS2DModeDS {
private:
	std::vector<MyRangeTree> trees;
	std::unordered_map<GridIndex, Color_> gridModeColor;
	std::vector<double> xslabBoundaries, yslabBoundaries; //xslabBoundaries[i] < points in slab i <= xslabBoundaries[i+1]
	std::vector<std::vector<ColoredPoint_2>> xslabPoints, yslabPoints;
public:
	AAS2DModeGridDS(std::vector<Point_2>& points, std::vector<Color_>& colors, int s, int nrColors);

	ColorCount queryMode(const Range2D& query) override;
	long getDSMemoryUsage() override;
	std::string getName() override;
};