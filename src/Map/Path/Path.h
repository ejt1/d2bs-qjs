#pragma once

#if defined(__WINE__) || defined(__unix__)
#include "../Map.h"
#include "Reduction/PathReducer.h"
#else
#include "..\Map.h"
#include "Reduction\PathReducer.h"
#endif

namespace Mapping {
namespace Pathing {

typedef int(__fastcall* Estimator)(const Map*, const Point&, const Point&);
typedef int(__fastcall* Distance)(const Point&, const Point&);

class MapPath {
 public:
  virtual void GetPath(const Point& start, const Point& end, PointList& list, bool abs) = 0;

  virtual void SetMap(Map* map) = 0;
  virtual void SetPathReducer(Reducing::PathReducer* reducer) = 0;
};

}  // namespace Pathing
}  // namespace Mapping
