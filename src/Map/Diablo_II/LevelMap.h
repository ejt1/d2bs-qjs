#pragma once

#include "..\Map.h"

#include <list>
#include <map>

#include "D2Ptrs.h"
#include "D2Helpers.h"

namespace Mapping {

enum ExitType { Linkage = 1, Tile = 2 };

struct Exit {
 public:
  DWORD Target;
  Point Position;
  ExitType Type;
  DWORD TileId;

  Exit(Point location, DWORD levelId, ExitType type, DWORD tileId) : Target(levelId), Position(location), Type(type), TileId(tileId) {
  }
};

class LevelMap;

typedef std::vector<Exit> ExitArray;
typedef std::map<DWORD, LevelMap*> LevelMapList;
typedef std::list<D2DrlgRoomStrc*> RoomList;
typedef std::list<D2DrlgLevelStrc*> LevelList;
static RoomList RoomsAdded;
static RoomList roomCache;
static LevelList levelCache;
class LevelMap : public Map {
 public:
  enum CollisionFlag {
    None = 0x0000,
    BlockWalk = 0x0001,
    BlockLineOfSight = 0x0002,
    Wall = 0x0004,
    BlockPlayer = 0x0008,
    AlternateTile = 0x0010,
    Blank = 0x0020,
    Missile = 0x0040,
    Player = 0x0080,
    NPCLocation = 0x0100,
    Item = 0x0200,
    Object = 0x0400,
    ClosedDoor = 0x0800,
    NPCCollision = 0x1000,
    FriendlyNPC = 0x2000,
    Unknown = 0x4000,
    DeadBody = 0x8000,  // also portal
    ThickenedWall = 0xfefe,
    Avoid = 0xffff
  };

 private:
  static LevelMapList cache;

  D2DrlgActStrc* act;
  const D2DrlgLevelStrc* level;
  int width, height;
  int posX, posY;
  Matrix<CollisionFlag> mapPoints;
  RoomList addedRooms;

  D2DrlgLevelStrc* cachedLevel;
  CRITICAL_SECTION* lock;

  void Build(void);

  static inline int GetLevelNo(D2DrlgRoomStrc* room, DWORD tile) {
    return GetTileLevelNo(room, tile);
  }

  void AddRoom(D2DrlgRoomStrc* const room, RoomList& rooms, D2UnitStrc* player);
  void AddCollisionMap(D2ActiveRoomStrc* pRoom1);
  void SetCollisionData(int x, int y, int value);
  bool IsGap(int x, int y, bool abs) const;
  void FillGaps(void);
  void ShrinkMap(void);
  void ThickenWalls(void);

  bool RoomSpaceIsWalkable(D2ActiveRoomStrc* pRoom1, const Point& point, bool abs) const;
  bool ValueIsWalkable(const WORD* value) const;
  bool ValueHasFlag(int flag, const WORD* value) const;

  bool EdgeIsWalkable(const Point& edgePoint, const Point& offsetPoint, D2ActiveRoomStrc* pRoom1Adjecent, bool abs) const;

  void FindRoomTileExits(D2DrlgRoomStrc* room, ExitArray& exits) const;
  void FindRoomLinkageExits(ExitArray& exits, RoomList& added) const;
  Point GetEdgeCenterPoint(const Point& currentPoint, const Point& edgeDirection) const;
  bool ExitExists(Point loc, ExitArray& exits) const;
  bool ExitExists(DWORD dwLevelNo, ExitArray& exits) const;
  bool isPointInRoom(D2DrlgRoomStrc* room, const Point& pt) const;
  bool isPointInLevel(D2DrlgLevelStrc* level, const Point& pt) const;
  WORD getCollFromRoom(D2DrlgRoomStrc* room, const Point& pt) const;

  void DrillExits();

  LevelMap(const D2DrlgLevelStrc* level);
  virtual ~LevelMap(void);

 public:
  static LevelMap* GetMap(D2DrlgLevelStrc* level);
  static void ClearCache(void);
  void Dump(const char* file, const PointList& points) const;
  void CleanUp(void) const;
  Point AbsToRelative(const Point& point) const;
  Point RelativeToAbs(const Point& point) const;

  inline int GetWidth(void) const {
    return width;
  }
  inline int GetHeight(void) const {
    return height;
  }
  inline int GetPosX(void) const {
    return posX;
  }
  inline int GetPosY(void) const {
    return posY;
  }
  /* If these are backwards, it's not me. */
  inline int GetMaxX(void) const {
    return posX + height;
  }
  inline int GetMaxY(void) const {
    return posY + width;
  }

  int GetMapData(const Point& point, bool abs = true) const;
  bool IsValidPoint(const Point& point, bool abs = true) const;

  void GetExits(ExitArray&) const;

  bool SpaceHasFlag(int flag, const Point& point, bool abs = true) const;
  bool PathHasFlag(int flag, const PointList& points, bool abs = true) const;

  bool SpaceIsWalkable(const Point& point, bool abs = true) const;
  bool SpaceIsWalkableForExit(const Point& point, bool abs = true) const;
  bool SpaceHasLineOfSight(const Point& point, bool abs = true) const;
  bool SpaceIsInvalid(const Point& point, bool abs = true) const;
  bool SpaceIsThickenedWall(const Point& point, bool abs = true) const;

  bool PathIsWalkable(const PointList& points, bool abs = true) const;
  bool PathHasLineOfSight(const PointList& points, bool abs = true) const;
};

}  // namespace Mapping
