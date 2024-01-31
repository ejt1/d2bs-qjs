#pragma once

#include "..\..\Map.h"

namespace Mapping {
namespace Pathing {
namespace Reducing {

class PathReducer {
 public:
  PathReducer() {
  }
  PathReducer(const PathReducer&);
  virtual ~PathReducer() {
  }
  PathReducer& operator=(const PathReducer&);

  virtual void Reduce(const PointList& in, PointList& out, bool abs) = 0;
  virtual void GetOpenNodes(const Point& center, PointList& out, const Point& endpoint) = 0;
  virtual bool Reject(const Point& pt, bool abs) = 0;
  virtual int GetPenalty(const Point& pt, bool abs) = 0;
  virtual void MutatePoint(Point& pt, bool abs) = 0;
};

}  // namespace Reducing
}  // namespace Pathing
}  // namespace Mapping