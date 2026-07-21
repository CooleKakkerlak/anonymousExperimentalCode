#include "mode_arrangement.h"

#include <CGAL/Arr_linear_traits_2.h>
#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arr_dcel_base.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Line_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/basic.h>
#include <CGAL/Arr_batched_point_location.h>
#include <CGAL/Arr_trapezoid_ric_point_location.h>
#include <CGAL/Arr_naive_point_location.h>
#include <CGAL/Arr_landmarks_point_location.h>
#include "../shared/defs.h"
#include "arr_print.h"

#include <iostream>
#include <random>
#include <chrono>

// arrangement based implementation

// transforms 1D points into 2D rays, representing the distance function from that point
void AAS1DModeArrangementDS::computeFunctionRays()
{
	for (int i = 0; i < colors.size(); i++)
	{
		rays.push_back(Ray_2(Point_2(i, 0), Point_2(i + 1, 1)));
		rays.push_back(Ray_2(Point_2(i, 0), Point_2(i - 1, 1)));
	}
}

// for each face of the arrangement, compute the conflict list, and the mode color below it
void AAS1DModeArrangementDS::computeFaceData()
{
	// save the conflictList for each face, indexed by faceIndex (since updating faceData's is annoying)
	std::vector<ConflictList> conflictLists;

	// set the faceIndex of each face to some arbitrary but unique value (note: this does NOT go over the fictitious face)
	int faceIndex = 0;
	for (Face_handle face = arrangement.faces_begin(); face != arrangement.faces_end(); ++face)
	{
		conflictLists.push_back(ConflictList());
		Range undefined;
		undefined.left = std::numeric_limits<int>::max();
		undefined.right = std::numeric_limits<int>::min();
		conflictLists[conflictLists.size() - 1].conflictingRayIndicesDownwards = undefined;
		conflictLists[conflictLists.size() - 1].conflictingRayIndicesUpwards = undefined;
		face->set_data(FaceData{ faceIndex++ });
	}
	int nrFaces = arrangement.number_of_faces();

	// compute the conflictlist of each face, by computing the zone of each ray (i.e. the set of faces that intersect it)
	for (int rayIndex = 0; rayIndex < rays.size(); rayIndex++)
	{
		const Ray_2& ray = rays[rayIndex];

		std::vector<CGAL::Object> zoneObjects;
		CGAL::zone(arrangement, ray, std::back_inserter(zoneObjects) /*, *pointLocation*/);
		for (CGAL::Object object : zoneObjects)
		{
			// slightly cumbersome, but we need to add the ray to the conflict list of all faces it intersects, also if it only intersects their edge/vertex
			Face_handle face;
			Halfedge_handle halfedge;
			Vertex_handle vertex;
			std::set<int> fIndices; // the index/indices of the face/faces corresponding to this object (all faces adjacent to an edge/vertex)
			if (CGAL::assign(face, object) && !face->is_fictitious())
			{
				fIndices.insert(face->data().faceIndex); // just the intersected face
			}
			else if (CGAL::assign(halfedge, object) && !halfedge->is_fictitious())
			{
				fIndices.insert(halfedge->face()->data().faceIndex); // faces on both sides of this edge
				fIndices.insert(halfedge->twin()->face()->data().faceIndex);
			}
			else if (CGAL::assign(vertex, object) && !vertex->is_at_open_boundary())
			{
				auto first = vertex->incident_halfedges(); // all faces around this vertex
				auto curr = first;
				do
				{
					fIndices.insert(curr->face()->data().faceIndex);
				} while (++curr != first);
			}
			// this ray intersects these faces; add it to their conflict lists
			for (int fInd : fIndices)
			{
				if (rayIndex % 2 == 0) {
					conflictLists[fInd].conflictingRayIndicesUpwards.left = std::min(rayIndex, conflictLists[fInd].conflictingRayIndicesUpwards.left);
					conflictLists[fInd].conflictingRayIndicesUpwards.right = std::max(rayIndex, conflictLists[fInd].conflictingRayIndicesUpwards.right);
				}
				if (rayIndex % 2 == 1) {
					conflictLists[fInd].conflictingRayIndicesDownwards.left = std::min(rayIndex, conflictLists[fInd].conflictingRayIndicesDownwards.left);
					conflictLists[fInd].conflictingRayIndicesDownwards.right = std::max(rayIndex, conflictLists[fInd].conflictingRayIndicesDownwards.right);
				}
			}
		}
	}

	//doesn't work for s = 1
	//lower face `intersects' everything. this messes with preprocessing time, so we clear it. Walking through the arrangement while maintaining the current intersecting lines still works, since no rays are strictly _under_ the lower face or any of its neighbors anyway
	bool notSeenLowerFace = true;
	for (auto& cl : conflictLists)
		if (cl.conflictingRayIndicesUpwards.left == 0 && cl.conflictingRayIndicesUpwards.right == colors.size() * 2 - 2 &&
			cl.conflictingRayIndicesDownwards.left == 1 && cl.conflictingRayIndicesDownwards.right == colors.size() * 2 - 1 && notSeenLowerFace) {
			Range undefined;
			undefined.left = std::numeric_limits<int>::max();
			undefined.right = std::numeric_limits<int>::min();
			cl.conflictingRayIndicesDownwards = undefined;
			cl.conflictingRayIndicesUpwards = undefined;
			notSeenLowerFace = false;
		}

	// compute mode color for each face by walking through the arrangement, maintaining the set of rays under the current face
	std::map<Color_, int> underColorCounts = std::map<Color_, int>();
	std::vector<bool> currentlyUnder(rays.size(), false); // whether rays[i] is under the current face

	std::vector<bool> added(faceIndex, false);	 // whether rays[i] has been added to the stack yet
	std::vector<bool> visited(faceIndex, false); // whether rays[i] has been popped off the stack yet (it can be added to the stack multiple times because of walking backwards)
	std::vector<Face_handle> faceStack;

	Face_handle firstFace = arrangement.faces_begin();
	if (conflictLists[firstFace->data().faceIndex].conflictingRayIndicesDownwards.left > conflictLists[firstFace->data().faceIndex].conflictingRayIndicesDownwards.right) //ensure we don't add the lower face as the first face
		firstFace++;
	faceStack.push_back(arrangement.faces_begin());

	std::set<int> raysToCheck; // the set of rays to check after each step (consists of the conflict list of the current and previous face)
	for (int i = 0; i < rays.size(); i++)
		raysToCheck.insert(i); //.. except at the start, where we have to check every ray


	while (!faceStack.empty())
	{
		Face_handle curFace = faceStack.back();
		faceStack.pop_back();
		int curFaceIndex = curFace->data().faceIndex;

		if (conflictLists[curFaceIndex].conflictingRayIndicesDownwards.left > conflictLists[curFaceIndex].conflictingRayIndicesDownwards.right) // the lower face
			continue;

		// compute for each relevant ray whether it intersects this face
		for (int i = conflictLists[curFaceIndex].conflictingRayIndicesUpwards.left; i <= conflictLists[curFaceIndex].conflictingRayIndicesUpwards.right; i += 2)
			raysToCheck.insert(i);
		for (int i = conflictLists[curFaceIndex].conflictingRayIndicesDownwards.left; i <= conflictLists[curFaceIndex].conflictingRayIndicesDownwards.right; i += 2)
			raysToCheck.insert(i);
		for (int rayIndex : raysToCheck)
		{
			int twinRayIndex = rayIndex + (1 - 2 * (rayIndex % 2)); // the other ray that originates from the same point

			// iterate through face-vertices
			Halfedge_handle start = curFace->outer_ccb();
			Halfedge_handle curr = start;
			bool rayBelowFace = true;
			do
			{
				// for each vertex, compute if it lies below the line

				// if the vertex is at infinity; just intersect the line and the ray then
				if (curr->source()->is_at_open_boundary())
				{
					if (!curr->is_fictitious() && CGAL::intersection(rays[rayIndex], curr->curve().ray()))
					{ // if they intersect, the ray is not below the face
						rayBelowFace = false;
						break; // no need to check the other vertices
					}
					// else the edge is fictitious, we can safely ignore it
					curr = curr->next();
					continue;
				}

				// the vertex is real; check on which side of the ray it lies
				Point_2 vertex = curr->source()->point();
				Point_2 p1 = rays[rayIndex].source(), p2 = rays[rayIndex].second_point();
				if (p1.x() > p2.x())
				{ // make sure p1 is left of p2, so the oriented side check works
					Point_2 temp = p1;
					p1 = p2;
					p2 = temp;
				}

				if (!Line_2(p1, p2).has_on_positive_side(vertex)) // this vertex is below (or on) the ray, so the ray is not below the face (positive side is left of oriented line, so above)
				{
					rayBelowFace = false;
					break; // no need to check the other vertices
				}

				curr = curr->next();
			} while (curr != start);

			// update the color counts
			if (rayBelowFace)
			{
				if (!currentlyUnder[rayIndex] && currentlyUnder[twinRayIndex]) // if this ray just got below this face, and the other half already was
					underColorCounts[colors[rayIndex / 2]] += 1;
				currentlyUnder[rayIndex] = true;
			}
			else
			{
				if (currentlyUnder[rayIndex] && currentlyUnder[twinRayIndex])
				{ // if this ray used to be under but not anymore, and the other half still is
					underColorCounts[colors[rayIndex / 2]] -= 1;

					// remove the entry if it's 0 (not necessary but a bit cleaner)
					if (underColorCounts[colors[rayIndex / 2]] == 0)
						underColorCounts.erase(colors[rayIndex / 2]);
				}
				currentlyUnder[rayIndex] = false;
			}
		}

		// the first time we pop a face off the stack, we want to try to visit all its neighbors
		if (!visited[curFaceIndex])
		{
			visited[curFaceIndex] = true;

			// add each un-added neighbor to the stack
			Halfedge_handle start = curFace->outer_ccb();
			Halfedge_handle curr = start;
			do
			{
				Face_handle neighbor = curr->twin()->face(); // outer_ccb consists of all inwards-pointing edges, so the twin's face is the neighbor
				if (!neighbor->is_fictitious() && !added[neighbor->data().faceIndex])
				{ // unbounded faces have the fictitious face as neigbor, so need to check for that
					faceStack.push_back(curFace);
					faceStack.push_back(neighbor);
					added[neighbor->data().faceIndex] = true;
				}
				curr = curr->next();
			} while (curr != start);

			//checkUnderRays(underColorCounts, currentlyUnder, curFace); //DEBUG check

			// store the current mode color in this face
			FaceData fd = {
				curFaceIndex,
				maxCount(underColorCounts),
				conflictLists[curFaceIndex] };
			curFace->set_data(fd);
		}

		raysToCheck.clear();
		for (int i = conflictLists[curFaceIndex].conflictingRayIndicesUpwards.left; i <= conflictLists[curFaceIndex].conflictingRayIndicesUpwards.right; i += 2)
			raysToCheck.insert(i);
		for (int i = conflictLists[curFaceIndex].conflictingRayIndicesDownwards.left; i <= conflictLists[curFaceIndex].conflictingRayIndicesDownwards.right; i += 2)
			raysToCheck.insert(i);
	}
}

void AAS1DModeArrangementDS::checkUnderRays(std::map<Color_, int>& underColorCounts, std::vector<bool>& currentlyUnder, Face_handle curFace)
{
	// check if the given underColorCounts and currentlyUnder are correct for curFace (for debug purposes only)
	std::vector<bool> checkUnder;
	for (int rayIndex = 0; rayIndex < rays.size(); rayIndex++)
	{
		int twinRayIndex = rayIndex + (1 - 2 * (rayIndex % 2));
		// iterate through face-vertices
		Halfedge_handle start = curFace->outer_ccb();
		Halfedge_handle curr = start;
		curr = start;
		bool rayBelowFace = true;
		do
		{
			// for each vertex, compute if it lies below the line
			if (curr->source()->is_at_open_boundary())
			{ // vertex at infinity; just intersect the line and the ray then
				if (!curr->is_fictitious() && CGAL::intersection(rays[rayIndex], curr->curve().ray()))
				{ // if they intersect, the ray is not below the face
					rayBelowFace = false;
					break; // no need to check the other vertices
				}
				// else the edge is fictitious, we can safely ignore it
				curr = curr->next();
				continue;
			}

			Point_2 vertex = curr->source()->point();
			Point_2 p1 = rays[rayIndex].source(), p2 = rays[rayIndex].second_point();
			if (p1.x() > p2.x())
			{ // make sure p1 is left of p2, so the oriented side check works
				Point_2 temp = p1;
				p1 = p2;
				p2 = temp;
			}

			if (!Line_2(p1, p2).has_on_positive_side(vertex)) // this vertex is below (or on) the ray, so the ray is not below the face (positive side is left of oriented line, so above)
			{
				rayBelowFace = false;
				break; // no need to check the other vertices
			}

			curr = curr->next();
		} while (curr != start);
		checkUnder.push_back(rayBelowFace);
	}
	std::map<Color_, int> checkUnderColorCounts = std::map<Color_, int>();
	for (int pointIndex = 0; pointIndex < rays.size() / 2; pointIndex++)
	{
		if (checkUnder[2 * pointIndex] && checkUnder[2 * pointIndex + 1])
			checkUnderColorCounts[colors[pointIndex]] += 1;
	}

	for (int i = 0; i < checkUnder.size(); i++)
		if (checkUnder[i] != currentlyUnder[i])
			//throw std::runtime_error("ew");
			std::cout << "ew";
	if (!(checkUnderColorCounts.size() == underColorCounts.size() && std::equal(checkUnderColorCounts.begin(), checkUnderColorCounts.end(), underColorCounts.begin())))
		//throw std::runtime_error("jakkie");
		std::cout << "jakkie";

	//std::cout << "yup";
}

// check if the current cutting adheres to the size bounds
bool AAS1DModeArrangementDS::isValidCutting(int s, double leniance_factor, int leniance_constant)
{
	int max = 2 * ((double)colors.size() / s * leniance_factor + leniance_constant); // times 2 since we count each ray separately

	// Check if any face has more than n/s intersecting lines
	for (Face_handle face = arrangement.faces_begin(); face != arrangement.faces_end(); face++)
	{
		int size = (face->data().conflictList.conflictingRayIndicesUpwards.right - face->data().conflictList.conflictingRayIndicesUpwards.left) / 2 +
			(face->data().conflictList.conflictingRayIndicesDownwards.right - face->data().conflictList.conflictingRayIndicesDownwards.left) / 2;


		if (size != 2 * colors.size())
			maxConflictListSize = std::max(maxConflictListSize, size);
		if (size > max)
			return false;
	}
	return true;
}

// compute the mode color of a given range query
ColorCount AAS1DModeArrangementDS::queryMode(const Range& query)
{
	// map the interval to a 2D point q and find the face containing it
	Point_2 point(((double)query.left + query.right) / 2 - 0.5, ((double)query.right - query.left) / 2); //-0.5 in the x to adjust for inclusivity; recall that coordinates are integer
	auto result = pointLocation->locate(point);

	Face_const_handle face, empty;

	// if we land on a vertex/edge we can choose an arbitrary adjacent face
	if (const Face_const_handle* face_ptr = std::get_if<Face_const_handle>(&result))
	{
		face = *face_ptr;
	}
	else if (const Halfedge_const_handle* edge_ptr = std::get_if<Halfedge_const_handle>(&result))
	{
		face = (*edge_ptr)->face();
	}
	else if (const Vertex_const_handle* vertex_ptr = std::get_if<Vertex_const_handle>(&result))
	{
		face = (*vertex_ptr)->incident_halfedges()->face();
	}

	if (face == empty)
		throw std::exception("didn't find face");

	// candidate colors are those in the conflict list and the mode color of all lines below
	FaceData faceData = face->data();
	std::set<int> candidateColors;
	for (int i = faceData.conflictList.conflictingRayIndicesUpwards.left; i <= faceData.conflictList.conflictingRayIndicesUpwards.right; i += 2)
		candidateColors.insert(colors[i / 2]);
	for (int i = faceData.conflictList.conflictingRayIndicesDownwards.left; i <= faceData.conflictList.conflictingRayIndicesDownwards.right; i += 2)
		candidateColors.insert(colors[i / 2]);
	if (faceData.mode.color != -1)
		candidateColors.insert(faceData.mode.color);

	//std::cout << "nr colors: " << candidateColors.size() << std::endl;

	// compute the color with maximum frequency
	ColorCount mode;
	for (int col : candidateColors)
	{
		int count = rangeTrees[col].queryCount(query);
		if (count > mode.count)
		{
			mode = ColorCount(col, count);
		}
	}
	return mode;
}

AAS1DModeArrangementDS::AAS1DModeArrangementDS(std::vector<Color_> colors, int numColors, int s) : colors(colors)
{
	// build a range tree for each color separately
	std::vector<std::vector<int>> indicesWithColor = generateIndicesWithColor(colors, numColors);
	for (int col = 0; col < numColors; col++)
		rangeTrees.push_back(RangeTree1D(indicesWithColor[col]));

	// convert all points to duals
	computeFunctionRays();

	int MAX_RETRY = 10, retry = 0;
	while (true)
	{
		// sample some rays to use as a cutting
		std::vector<Ray_2> sampledRays;
		for (int i = 1; i < s; i++) {
			int index = 2 * (int)(i * ((double)colors.size() / s));
			sampledRays.push_back(rays[index]);
			sampledRays.push_back(rays[index + 1]);
		}

		// create an arrangement from the sampled rays
		CGAL::insert(arrangement, sampledRays.begin(), sampledRays.end());
		CGAL::insert(arrangement, Line_2(Point_2(0, 0), Point_2(1, 0))); // add the line y = 0 because otherwise the lower face has linear complexity

		// compute the conflict list and mode color of each face
		computeFaceData();

		// if the conflict lists are valid, we are done. otherwise, we try again.
		if (isValidCutting(s, 3, 3)) // parameters are kinda magic (TODO?)
			break;

		// reset and try again
		arrangement.clear();
		maxConflictListSize = 0;
		retry++;
		if (retry > MAX_RETRY)
			throw std::runtime_error("could not find a valid cutting");
		std::cout << "didn't succeed to find a cutting in " << retry << " tries, trying again.." << std::endl;
	}
	pointLocation = std::make_unique<PointLocation>(arrangement);
}

long AAS1DModeArrangementDS::getDSMemoryUsage()
{
	long count = 0;
	count += sizeof(Color_) * colors.size();
	count += sizeof(Ray_2) * rays.size();
	// lower-bound on the space usage of the arrangement; I compute the size here as if it is the simplest form of a DCEL
	count += arrangement.number_of_faces() * (sizeof(FaceData) + sizeof(Halfedge_handle));		  // each face stores data, and a pointer to its outer_ccb
	count += arrangement.number_of_edges() * (3 * sizeof(Halfedge_handle) + sizeof(Face_handle)); // each halfedge stores its prev, next, and twin, and its adjacent face
	count += arrangement.number_of_vertices() * (sizeof(Point_2) + sizeof(Halfedge_handle));	  // each vertex stores its coordinates, and a pointer to an arbitrary incident edge
	// count += pointLocation-> not sure what to retrieve here... no public information in pointlocation class
	count += sizeof(maxConflictListSize);
	return count;
}

std::string AAS1DModeArrangementDS::getName()
{
	return GET_NAME(AAS1DModeArrangementDS);
}