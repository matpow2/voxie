/*
 Copyright (c) 2013 yvt

 Triangulates arbitary polygons represented by cells.
 Algorithm's time complexity is roughly O(W*H).

 Created to fix #213 ( https://github.com/yvt/openspades/issues/213 ).

 See this web page for algorithm demonstration:
 https://dl.dropboxusercontent.com/u/37804131/triangulate-2.html

 BSD license.
 */

/*
changed for the purpose of generating meshes for voxie
*/

#include <memory>
#include <vector>
#include <algorithm>
#include <list>
#include <cassert>

namespace c2t
{
	struct Point
	{
		int x, y;

		Point()
		{}

		Point(int x, int y)
		: x(x), y(y)
		{}
	};


	// structures used internally
	struct SpanRange {
		int x1, x2;
		SpanRange()
		{
		}

		SpanRange(int x1, int x2): x1(x1), x2(x2) {}
		inline static SpanRange CreateInvalid() { return SpanRange(-10, -10); }
		inline bool IsValid() { return x1 != -10; }
	};

	struct Edge {
		std::vector<Point> points;
	};

	struct Span {
		int x1, x2;
		std::vector<int> xs;
		int y;
		std::shared_ptr<Edge> leftEdge;
		std::shared_ptr<Edge> rightEdge;
	};

	template<class T>
	class Trianglulator {
		T & model;
		const int w, h;

		SpanRange SearchSpan(int x, int y) {
			while(x < w && !model(x, y)) x++;
			if(x >= w) return SpanRange::CreateInvalid();
			while(x > 0 && model(x - 1, y)) x--;
			auto x1 = x;
			while(x < w && model(x, y)) x++;
			return SpanRange(x1, x);
		}

		template<bool flipped> static bool TriangleSide(const Point& p1,
											const Point& p2,
											const Point& p3) {
			auto x1 = p2.x - p1.x, y1 = p2.y - p1.y;
			auto x2 = p3.x - p1.x, y2 = p3.y - p1.y;
			auto area = x2 * y1 - x1 * y2;
			return flipped ? area < 0 : area > 0;
		}

		std::vector<std::shared_ptr<Edge>> lastLeftEdges;
		std::vector<std::shared_ptr<Edge>> lastRightEdges;
		std::vector<int> lastLeftEdgesY;
		std::vector<int> lastRightEdgesY;
		std::vector<int> lastProcessedY;

		std::vector<Point> & polys;

		void Init() {
			lastLeftEdges.resize(w + 1);
			lastRightEdges.resize(w + 1);
			lastLeftEdgesY.resize(w + 1);
			lastRightEdgesY.resize(w + 1);
			lastProcessedY.resize(w + 1);
		}

		std::vector<std::size_t> edgeStack;

		template<bool flipped>
		void EmitEdge(Edge *edge) {
			const auto& points = edge->points;
			edgeStack.clear();
			edgeStack.push_back(0);
			edgeStack.push_back(1);

			for(std::size_t i = 2; i < points.size(); i++) {
				while(edgeStack.size() > 1) {
					auto j = edgeStack.back(); edgeStack.pop_back();
					auto k = edgeStack.back();
					if(TriangleSide<flipped>(points[j], points[k], points[i])) {
						if(flipped) {
							polys.push_back(points[k]);
							polys.push_back(points[j]);
						} else {
							polys.push_back(points[j]);
							polys.push_back(points[k]);
						}
						polys.push_back(points[i]);
					} else {
						edgeStack.push_back(j);
						break;
					}
				}

				edgeStack.push_back(i);
			}
		}

	public:
		Trianglulator(T& model, std::vector<Point> & polys):
		model(model), polys(polys),
		w(model.GetWidth()), h(model.GetHeight()) {
			Init();
		}

		void triangulate() {
			std::fill(lastLeftEdgesY.begin(), lastLeftEdgesY.end(), -1);
			std::fill(lastRightEdgesY.begin(), lastRightEdgesY.end(), -1);
			std::fill(lastProcessedY.begin(), lastProcessedY.end(), -1);
			polys.clear();

			std::list<Span> spans;
			std::vector<std::list<Span>::iterator> removedIterators;

			std::vector<int> points;

			for(int y = 0; y <= h; y++) {

				removedIterators.clear();
				for(auto it = spans.begin(); it != spans.end(); it++) {
					auto& span = *it;

					bool removeSpan = false;

					int x = span.x1;
					for(;x < span.x2; x++) {
						if(!model(x, y)) break;
					}

					if(x < span.x2) {
						removeSpan = true;
					}else if(model(span.x1 - 1, y) || model(span.x2, y)) {
						removeSpan = true;
					}

					if(removeSpan) {

						// generate polygons for span
						{
							const auto& startpoints = span.xs;
							auto& endpoints = points;

							endpoints.clear();

							if(model(span.x1 - 1, y) || !model(span.x1, y))
								endpoints.push_back(span.x1);

							bool last = model(span.x1, y);
							for(int x = span.x1 + 1; x < span.x2; x++) {
								bool b = model(x, y);
								if(b != last) {
									endpoints.push_back(x);
									last = b;
								}
							}

							if(model(span.x2, y) || !model(span.x2 - 1, y))
								endpoints.push_back(span.x2);

							int y1 = span.y, y2 = y;

							{
								auto *leftEdge = span.leftEdge.get();
								if(leftEdge) leftEdge->points.push_back(Point(endpoints.front(), y2));
							}
							if(endpoints.front() > span.x1) {
								if(span.leftEdge == nullptr) {
									span.leftEdge = std::make_shared<Edge>();
									span.leftEdge->points.push_back(Point(span.x1, span.y));
									span.leftEdge->points.push_back(Point(endpoints.front(), y2));
								}
								lastLeftEdges[span.x1] = span.leftEdge;
								lastLeftEdgesY[span.x1] = y;
							}else{
								if(span.leftEdge) EmitEdge<false>(span.leftEdge.get());
								span.leftEdge.reset();
							}

							{
								auto *rightEdge = span.rightEdge.get();
								if(rightEdge) rightEdge->points.push_back(Point(endpoints.back(), y2));
							}
							if(endpoints.back() < span.x2) {
								if(span.rightEdge == nullptr) {
									span.rightEdge = std::make_shared<Edge>();
									span.rightEdge->points.push_back(Point(span.x2, span.y));
									span.rightEdge->points.push_back(Point(endpoints.back(), y2));
								}
								lastRightEdges[span.x2] = span.rightEdge;
								lastRightEdgesY[span.x2] = y;
							}else{
								if(span.rightEdge) EmitEdge<true>(span.rightEdge.get());
								span.rightEdge.reset();
							}

							// emit polygons
							for(std::size_t i = 1; i < endpoints.size(); i++) {
								polys.push_back(Point(startpoints.front(), y1));
								polys.push_back(Point(endpoints[i - 1], y2));
								polys.push_back(Point(endpoints[i], y2));
							}
							for(std::size_t i = 1; i < startpoints.size(); i++) {
								polys.push_back(Point(startpoints[i], y1));
								polys.push_back(Point(startpoints[i - 1], y1));
								polys.push_back(Point(endpoints.back(), y2));
							}
						}

						removedIterators.push_back(it);
					}

					// -- one span done
				}

				{
					std::vector<std::list<Span>::iterator>::iterator it;
					for (it = removedIterators.begin();
						 it != removedIterators.end(); ++it) {
						spans.erase((*it));
					}
				}

				{
					// mark processed spans
					std::list<Span>::iterator it;
					for (it = spans.begin(); it != spans.end(); ++it) {
						lastProcessedY[(*it).x1] = y;
					}
				}

				// new span discovery
				for(int x = 0;;) {
					auto sp = SearchSpan(x, y);
					if(!sp.IsValid()) break;

					if(lastProcessedY[sp.x1] < y) {
						// unprocessed span found.

						spans.push_back(Span());
						auto& span = spans.back();
						if(lastLeftEdgesY[sp.x1] >= y)
							span.leftEdge = lastLeftEdges[sp.x1];
						if(lastRightEdgesY[sp.x2] >= y)
							span.rightEdge = lastRightEdges[sp.x2];

						auto& beginpoints = span.xs;

						if(model(sp.x1 - 1, y - 1) || !model(sp.x1, y - 1))
							beginpoints.push_back(sp.x1);

						bool last = model(sp.x1, y - 1);
						for(int x = sp.x1 + 1; x < sp.x2; x++) {
							bool b = model(x, y - 1);
							if(b != last) {
								beginpoints.push_back(x);
								last = b;
							}
						}

						if(model(sp.x2, y - 1) || !model(sp.x2 - 1, y - 1))
							beginpoints.push_back(sp.x2);

						assert(beginpoints.size() > 0);

						span.x1 = sp.x1;
						span.x2 = sp.x2;
						span.y = y;

					}

					x = sp.x2 + 1;

				}

				// -- one Y level done

			}
		}

	};

}
