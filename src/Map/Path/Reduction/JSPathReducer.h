#pragma once

#include "PathReducer.h"
#include "../Path.h"
#include "../../Diablo_II/ActMap.h"
#include "js32.h"

namespace Mapping {
namespace Pathing {
namespace Reducing {

#pragma warning(disable : 4512)

// TODO(ejt): since mozjs => quickjs migration this class is probably broken af :)
class JSPathReducer : public PathReducer {
 private:
  JSContext* cx;
  JSValue obj;
  jsval reject, reduce, mutate;

 public:
  JSPathReducer(const JSPathReducer&);
  JSPathReducer& operator=(const JSPathReducer&);
  JSPathReducer(ActMap* /*m*/, JSContext* cx, JSValue obj, jsval _reject, jsval _reduce, jsval _mutate) : obj(JS_DupValue(cx, obj)), reject(_reject), reduce(_reduce), mutate(_mutate) {
    JS_DupValue(cx, reject);
    JS_DupValue(cx, reduce);
    JS_DupValue(cx, mutate);
  }
  ~JSPathReducer(void) {
    JS_FreeValue(cx, obj);
    JS_FreeValue(cx, reject);
    JS_FreeValue(cx, reduce);
    JS_FreeValue(cx, mutate);
  }

  void Reduce(const PointList& in, PointList& out, bool /*abs*/) {
    // create the initial array to pass to the js function
    int count = in.size();

    //	JS_EnterLocalRootScope(cx);

    jsval* vec = new jsval[count];
    for (int i = 0; i < count; i++) {
      JSValue pt = JS_NewObject(cx);
      JS_SetPropertyStr(cx, pt, "x", JS_NewInt32(cx, in[i].first));
      JS_SetPropertyStr(cx, pt, "y", JS_NewInt32(cx, in[i].second));
      vec[i] = pt;
    }
    JSValue arr = JS_NewArray(cx);

    jsval argv[] = {
        JS_NULL,
        JS_NewInt32(cx, 0),
        arr,
    };
    for (int i = 0; i < count; i++) {
      jsval rval = JS_FALSE;
      argv[0] = vec[i];
      argv[1] = JS_NewInt32(cx, i);
      rval = JS_Call(cx, obj, reduce, 3, argv);
      if (!!JS_ToBool(cx, rval))
        out.push_back(in[i]);
    }

    //		JS_LeaveLocalRootScope(cx);
    delete[] vec;
  }
  bool Reject(const Point& pt, bool /*abs*/) {
    jsval rval = JS_FALSE;
    jsval argv[] = {
        JS_NewInt32(cx, pt.first),
        JS_NewInt32(cx, pt.second),
    };
    rval = JS_Call(cx, obj, reject, 2, argv);
    return !!JS_ToBool(cx, rval);
  }
  void GetOpenNodes(const Point& center, PointList& out, const Point& /*endpoint*/) {
    for (int i = 1; i >= -1; i--) {
      for (int j = 1; j >= -1; j--) {
        if (i == 0 && j == 0)
          continue;
        out.push_back(Point(center.first + i, center.second + j));
      }
    }
  }
  int GetPenalty(const Point& /*pt*/, bool /*abs*/) {
    return 0;
  }
  void MutatePoint(Point& pt, bool /*abs*/) {
    jsval rval = JS_FALSE;
    jsval argv[] = {
        JS_NewInt32(cx, pt.first),
        JS_NewInt32(cx, pt.second),
    };
    rval = JS_Call(cx, obj, mutate, 2, argv);
    if (JS_IsObject(rval)) {
      JSValue x = JS_GetPropertyUint32(cx, rval, 0);
      JSValue y = JS_GetPropertyUint32(cx, rval, 1);
      JS_ToInt32(cx, &pt.first, x);
      JS_ToInt32(cx, &pt.second, y);
    }
  }
};

#pragma warning(default : 4512)

}  // namespace Reducing
}  // namespace Pathing
}  // namespace Mapping
