#pragma once

#include "TransportMgr.h"

#include "TravelMgr.h"
#include "TravelNode.h"
#include "PlayerbotAI.h"

#include "ObjectMgr.h"
#include <numeric>
#include <iomanip>

#include "PathFinder.h"
#include "PlayerbotAI.h"
#include "VMapFactory.h"
#include "MoveMap.h"
#include "Transports.h"
#include <playerbot/strategy/StrategyContext.h>

#include "strategy/values/SharedValueContext.h"

#include "Grids/CellImpl.h"

using namespace ai;
using namespace MaNGOS;

WorldPosition::WorldPosition(GuidPosition guid)
{
    if (guid.mapid !=0 || guid.coord_x != 0 || guid.coord_y != 0 || guid.coord_z !=0) {
        set(WorldPosition(guid.mapid, guid.coord_x, guid.coord_y, guid.coord_z, guid.orientation));
        return;
    }

    set(ObjectGuid(guid));
 }

void WorldPosition::set(ObjectGuid guid)
{
    switch (guid.GetHigh())
    {
    case HIGHGUID_PLAYER:
    {
        Player* player = sObjectAccessor.FindPlayer(guid);
        if (player)
            set(WorldLocation(player->GetMapId(), player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation()));
        break;
    }
    case HIGHGUID_GAMEOBJECT:
    {
        GameObjectDataPair const* gpair = sObjectMgr.GetGODataPair(guid.GetCounter());
        if (gpair)
            set(WorldLocation(gpair->second.mapid, gpair->second.posX, gpair->second.posY, gpair->second.posZ, gpair->second.orientation));
        break;
    }
    case HIGHGUID_UNIT:
    {
        CreatureDataPair const* cpair = sObjectMgr.GetCreatureDataPair(guid.GetCounter());
        if (cpair)
            set(WorldLocation(cpair->second.mapid, cpair->second.posX, cpair->second.posY, cpair->second.posZ, cpair->second.orientation));
        break;
    }
    case HIGHGUID_TRANSPORT:
    case HIGHGUID_MO_TRANSPORT:
    case HIGHGUID_ITEM:
    case HIGHGUID_PET:
    case HIGHGUID_DYNAMICOBJECT:
    case HIGHGUID_CORPSE:
        return;
    }

}

WorldPosition::WorldPosition(vector<WorldPosition*> list, WorldPositionConst conType)
{
    uint32 size = list.size();
    if (size == 0)
        return;
    else if (size == 1)
        set(*list.front());
    else if (conType == WP_RANDOM)
        set(*list[urand(0, size - 1)]);
    else if (conType == WP_CENTROID)
        set(std::accumulate(list.begin(), list.end(), WorldLocation(list[0]->getMapId(), 0, 0, 0, 0), [size](WorldLocation i, WorldPosition* j) {i.coord_x += j->getX() / size; i.coord_y += j->getY() / size; i.coord_z += j->getZ() / size; i.orientation += j->getO() / size; return i; }));
    else if (conType == WP_MEAN_CENTROID)
    {
        WorldPosition pos = WorldPosition(list, WP_CENTROID);
        set(*pos.closestSq(list));
    }
}

WorldPosition::WorldPosition(vector<WorldPosition> list, WorldPositionConst conType)
{    
    uint32 size = list.size();
    if (size == 0)
        return;
    else if (size == 1)
        set(list.front());
    else if (conType == WP_RANDOM)
        set(list[urand(0, size - 1)]);
    else if (conType == WP_CENTROID)
        set(std::accumulate(list.begin(), list.end(), WorldLocation(list[0].getMapId(), 0, 0, 0, 0), [size](WorldLocation i, WorldPosition j) {i.coord_x += j.getX() / size; i.coord_y += j.getY() / size; i.coord_z += j.getZ() / size; i.orientation += j.getO() / size; return i; }));
    else if (conType == WP_MEAN_CENTROID)
    {
        WorldPosition pos = WorldPosition(list, WP_CENTROID);
        set(pos.closestSq(list));
    }    
}

float WorldPosition::distance(WorldPosition* center)
{
    if(mapid == center->getMapId())
        return relPoint(center).size(); 

    //this -> mapTransfer | mapTransfer -> center
    return sTravelMgr.mapTransDistance(*this, *center);
};

float WorldPosition::fDist(WorldPosition* center)
{
    if (mapid == center->getMapId())
        return sqrt(sqDistance2d(center));

    //this -> mapTransfer | mapTransfer -> center
    return sTravelMgr.fastMapTransDistance(*this, *center);
};

//When moving from this along list return last point that falls within range.
//Distance is move distance along path.
WorldPosition WorldPosition::lastInRange(vector<WorldPosition> list, float minDist, float maxDist)
{
    WorldPosition rPoint;

    float startDist = 0.0f;

    //Enter the path at the closest point.
    for (auto& p : list)
    {
        float curDist = distance(p);
        if (startDist < curDist || p == list.front())
            startDist = curDist + 0.1f;
    }

    float totalDist = 0.0f;

    //Follow the path from the last nearest point
    //Return last point in range.
    for (auto& p : list)
    {
        float curDist = distance(p);

        if (totalDist > 0) //We have started the path. Keep counting.
            totalDist += p.distance(std::prev(&p, 1));

        if (curDist == startDist) //Start the path here.
            totalDist = startDist;

        if (minDist > 0 && totalDist < minDist)
            continue;

        if (maxDist > 0 && totalDist > maxDist)
            continue; //We do not break here because the path may loop back and have a second startDist point.

        rPoint = p;
    }

    return rPoint;
};

//Todo: remove or adjust to above standard.
WorldPosition WorldPosition::firstOutRange(vector<WorldPosition> list, float minDist, float maxDist)
{
    WorldPosition rPoint;

    for (auto& p : list)
    {
        if (minDist > 0 && distance(p) < minDist)
            return p;

        if (maxDist > 0 && distance(p) > maxDist)
            return p;

        rPoint = p;
    }

    return rPoint;
}

//Returns true if (on the x-y plane) the position is inside the three points.
bool WorldPosition::isInside(WorldPosition* p1, WorldPosition* p2, WorldPosition* p3)
{
    if (getMapId() != p1->getMapId() != p2->getMapId() != p3->getMapId())
        return false;

    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = mSign(p1, p2);
    d2 = mSign(p2, p3);
    d3 = mSign(p3, p1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

G3D::Vector3 WorldPosition::getVector3() 
{
    return G3D::Vector3(coord_x, coord_y, coord_z); 
}

string WorldPosition::print()
{
    ostringstream out;
    out << mapid << std::fixed << std::setprecision(2);
    out << ';'<< coord_x;
    out << ';' << coord_y;
    out << ';' << coord_z;
    out << ';' << orientation;

    return out.str();
}

void WorldPosition::printWKT(vector<WorldPosition> points, ostringstream& out, uint32 dim, bool loop)
{
    switch (dim) {
    case 0:
        if(points.size() == 1)
            out << "\"POINT(";
        else
            out << "\"MULTIPOINT(";
        break;
    case 1:
        out << "\"LINESTRING(";
        break;
    case 2:
        out << "\"POLYGON((";
    }

    for (auto& p : points)
        out << p.getDisplayX() << " " << p.getDisplayY() << (!loop && &p == &points.back() ? "" : ",");

    if (loop)
        out << points.front().getDisplayX() << " " << points.front().getDisplayY();

    out << (dim == 2 ? "))\"," : ")\",");
}

WorldPosition WorldPosition::getDisplayLocation() 
{ 
    return offset(&sTravelNodeMap.getMapOffset(getMapId())); 
};

AreaTableEntry const* WorldPosition::getArea()
{
    uint16 areaFlag = getAreaFlag();

    if(!areaFlag)
        return NULL;

    return GetAreaEntryByAreaFlagAndMap(areaFlag, getMapId());
}

string WorldPosition::getAreaName(bool fullName, bool zoneName)
{    
    if (!isOverworld())
    {
        MapEntry const* map = sMapStore.LookupEntry(getMapId());
        if (map)
            return map->name[0];
    }

    AreaTableEntry const* area = getArea();

    if (!area)
        return "";

    string areaName = area->area_name[0];

    if (fullName)
    {
        uint16 zoneId = area->zone;

        while (zoneId > 0)
        {
            AreaTableEntry const* parentArea = GetAreaEntryByAreaID(zoneId);

            if (!parentArea)
                break;

            string subAreaName = parentArea->area_name[0];

            if (zoneName)
                areaName = subAreaName;
            else
                areaName = subAreaName + " " + areaName;

            zoneId = parentArea->zone;
        }
    }

    return areaName;
}

std::set<Transport*> WorldPosition::getTransports(uint32 entry)
{
    /*
    if(!entry)
        return getMap()->m_transports;
    else
    {
    */
        std::set<Transport*> transports;
        /*
        for (auto transport : getMap()->m_transports)
            if(transport->GetEntry() == entry)
                transports.insert(transport);

        return transports;
    }
    */
    return transports;
}

std::vector<GridPair> WorldPosition::getGridPairs(WorldPosition secondPos)
{
    std::vector<GridPair> retVec;

    int lx = std::min(getGridPair().x_coord, secondPos.getGridPair().x_coord);
    int ly = std::min(getGridPair().y_coord, secondPos.getGridPair().y_coord);
    int ux = std::max(getGridPair().x_coord, secondPos.getGridPair().x_coord);
    int uy = std::max(getGridPair().y_coord, secondPos.getGridPair().y_coord);
    int border = 1;

    lx = std::min(std::max(border, lx), MAX_NUMBER_OF_GRIDS - border);
    ly = std::min(std::max(border, ly), MAX_NUMBER_OF_GRIDS - border);
    ux = std::min(std::max(border, ux), MAX_NUMBER_OF_GRIDS - border);
    uy = std::min(std::max(border, uy), MAX_NUMBER_OF_GRIDS - border);

    for (int x = lx - border; x <= ux + border; x++)
    {
        for (int y = ly - border; y <= uy + border; y++)
        {
            retVec.push_back(GridPair(x, y));
        }
    }

    return retVec;
}

vector<WorldPosition> WorldPosition::fromGridPair(GridPair gridPair)
{
    vector<WorldPosition> retVec;
    GridPair g;

    for (uint32 d = 0; d < 4; d++)
    {
        g = gridPair;

        if (d == 1 || d == 2)
            g >> 1;
        if (d == 2 || d == 3)
            g += 1;

        retVec.push_back(WorldPosition(getMapId(), g));
    }
   
    return retVec;
}

vector<WorldPosition> WorldPosition::fromCellPair(CellPair cellPair)
{
    vector<WorldPosition> retVec;
    CellPair p;

    for (uint32 d = 0; d < 4; d++)
    {
        p = cellPair;

        if (d == 1 || d == 2)
            p >> 1;
        if (d == 2 || d == 3)
            p += 1;

        retVec.push_back(WorldPosition(getMapId(), p));
    }
    return retVec;
}

vector<WorldPosition> WorldPosition::gridFromCellPair(CellPair cellPair)
{    
    Cell c(cellPair);

    return fromGridPair(GridPair(c.GridX(), c.GridY()));
}

vector<pair<int,int>> WorldPosition::getmGridPairs(WorldPosition secondPos)
{
    std::vector<mGridPair> retVec;

    int lx = std::min(getmGridPair().first, secondPos.getmGridPair().first);
    int ly = std::min(getmGridPair().second, secondPos.getmGridPair().second);
    int ux = std::max(getmGridPair().first, secondPos.getmGridPair().first);
    int uy = std::max(getmGridPair().second, secondPos.getmGridPair().second);
    int border = 1;

    //lx = std::min(std::max(border, lx), MAX_NUMBER_OF_GRIDS - border);
    //ly = std::min(std::max(border, ly), MAX_NUMBER_OF_GRIDS - border);
    //ux = std::min(std::max(border, ux), MAX_NUMBER_OF_GRIDS - border);
    //uy = std::min(std::max(border, uy), MAX_NUMBER_OF_GRIDS - border);

    for (int x = lx - border; x <= ux + border; x++)
    {
        for (int y = ly - border; y <= uy + border; y++)
        {
            retVec.push_back(make_pair(x, y));
        }
    }

    return retVec;
}

vector<WorldPosition> WorldPosition::frommGridPair(mGridPair gridPair)
{
    vector<WorldPosition> retVec;
    mGridPair g;

    for (uint32 d = 0; d < 4; d++)
    {
        g = gridPair;

        if (d == 1 || d == 2)
            g.second++;
        if (d == 2 || d == 3)
            g.first++;

        retVec.push_back(WorldPosition(getMapId(), g));
    }

    return retVec;
}

void WorldPosition::loadMapAndVMap(uint32 mapId, int x, int y)
{
    string fileName = "load_map_grid.csv";

    if (isOverworld() && false || false)
    {
#ifdef MANGOSBOT_TWO
        if (!MMAP::MMapFactory::createOrGetMMapManager()->IsMMapTileLoaded(mapId, 0, x, y))
#else
        if (!MMAP::MMapFactory::createOrGetMMapManager()->IsMMapIsLoaded(mapId, x, y))
#endif
            if (sPlayerbotAIConfig.hasLog(fileName))
            {
                ostringstream out;
                out << sPlayerbotAIConfig.GetTimestampStr();
                out << "+00,\"mmap\", " << x << "," << y << "," << (sTravelMgr.isBadMmap(mapId, x, y) ? "0" : "1") << ",";
                printWKT(fromGridPair(GridPair(x, y)), out, 1, true);
                sPlayerbotAIConfig.log(fileName, out.str().c_str());
            }

        int px = (float)(32 - x) * SIZE_OF_GRIDS;
        int py = (float)(32 - y) * SIZE_OF_GRIDS;

#ifdef MANGOSBOT_TWO
        if (!MMAP::MMapFactory::createOrGetMMapManager()->IsMMapTileLoaded(mapId, 0, x, y))
#else
        if (!MMAP::MMapFactory::createOrGetMMapManager()->IsMMapIsLoaded(mapId, x, y))
#endif
            if(getTerrain())
                getTerrain()->GetTerrainType(px, py);

    }
    else
    {

        //This needs to be disabled or maps will not load.
        //Needs more testing to check for impact on movement.
        if (false)
            if (!VMAP::VMapFactory::createOrGetVMapManager()->IsTileLoaded(mapId, x, y) && !sTravelMgr.isBadVmap(mapId, x, y))
            {
                // load VMAPs for current map/grid...
                const MapEntry* i_mapEntry = sMapStore.LookupEntry(mapId);
                const char* mapName = i_mapEntry ? i_mapEntry->name[sWorld.GetDefaultDbcLocale()] : "UNNAMEDMAP\x0";

                int vmapLoadResult = VMAP::VMapFactory::createOrGetVMapManager()->loadMap((sWorld.GetDataPath() + "vmaps").c_str(), mapId, x, y);
                switch (vmapLoadResult)
                {
                case VMAP::VMAP_LOAD_RESULT_OK:
                    //sLog.outError("VMAP loaded name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", mapName, mapId, x, y, x, y);
                    break;
                case VMAP::VMAP_LOAD_RESULT_ERROR:
                    //sLog.outError("Could not load VMAP name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", mapName, mapId, x, y, x, y);
                    sTravelMgr.addBadVmap(mapId, x, y);
                    break;
                case VMAP::VMAP_LOAD_RESULT_IGNORED:
                    sTravelMgr.addBadVmap(mapId, x, y);
                    //sLog.outError("Ignored VMAP name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", mapName, mapId, x, y, x, y);
                    break;
                }

                if (sPlayerbotAIConfig.hasLog(fileName))
                {
                    ostringstream out;
                    out << sPlayerbotAIConfig.GetTimestampStr();
                    out << "+00,\"vmap\", " << x << "," << y << ", " << (sTravelMgr.isBadVmap(mapId, x, y) ? "0" : "1") << ",";
                    printWKT(fromGridPair(GridPair(x, y)), out, 1, true);
                    sPlayerbotAIConfig.log(fileName, out.str().c_str());
                }
            }

#ifdef MANGOSBOT_TWO
        if (!MMAP::MMapFactory::createOrGetMMapManager()->IsMMapTileLoaded(mapId, 0, x, y) && !sTravelMgr.isBadMmap(mapId, x, y))
#else
        if (!MMAP::MMapFactory::createOrGetMMapManager()->IsMMapIsLoaded(mapId, x, y) && !sTravelMgr.isBadMmap(mapId, x, y))
#endif
        {
            // load navmesh
#ifdef MANGOSBOT_TWO
            if (!MMAP::MMapFactory::createOrGetMMapManager()->loadMap(mapId, 0, x, y, 0))
#else
            if (!MMAP::MMapFactory::createOrGetMMapManager()->loadMap(mapId, x, y))
#endif
                sTravelMgr.addBadMmap(mapId, x, y);

            if (sPlayerbotAIConfig.hasLog(fileName))
            {
                ostringstream out;
                out << sPlayerbotAIConfig.GetTimestampStr();
                out << "+00,\"mmap\", " << x << "," << y << "," << (sTravelMgr.isBadMmap(mapId, x, y) ? "0" : "1") << ",";
                printWKT(frommGridPair(mGridPair(x, y)), out, 1, true);
                sPlayerbotAIConfig.log(fileName, out.str().c_str());
            }
        }
    }
}

void WorldPosition::loadMapAndVMaps(WorldPosition secondPos)
{
    for (auto& grid : getmGridPairs(secondPos))
    {
        loadMapAndVMap(getMapId(), grid.first, grid.second);
    }
}

vector<WorldPosition> WorldPosition::fromPointsArray(std::vector<G3D::Vector3> path)
{
    vector<WorldPosition> retVec;
    for (auto p : path)
        retVec.push_back(WorldPosition(getMapId(), p.x, p.y, p.z, getO()));

    return retVec;
}

//A single pathfinding attempt from one position to another. Returns pathfinding status and path.
vector<WorldPosition> WorldPosition::getPathStepFrom(WorldPosition startPos, Unit* bot)
{
    //Load mmaps and vmaps between the two points.
    loadMapAndVMaps(startPos);

    PointsArray points;
    PathType type;

    if (bot)
    {
        PathFinder path(bot);

#ifdef IKE_PATHFINDER
        path.setAreaCost(9, 10.0f);  //Water
        path.setAreaCost(12, 5.0f);  //Mob proximity
        path.setAreaCost(13, 20.0f); //Mob agro
#endif
        path.calculate(startPos.getVector3(), getVector3(), false);

        points = path.getPath();
        type = path.getPathType();

    }
    else
#ifdef IKE_PATHFINDER
    {
        std::hash<std::thread::id> hasher;
        PathFinder path(getMapId(), hasher(std::this_thread::get_id()));

        path.setAreaCost(9, 10.0f);
        path.setAreaCost(12, 5.0f);
        path.setAreaCost(13, 20.0f);

        path.calculate(startPos.getVector3(), getVector3(), false);

        points = path.getPath();
        type = path.getPathType();
    }
#else
        return PATHFIND_NOPATH;
#endif

    if (sPlayerbotAIConfig.hasLog("pathfind_attempt_point.csv"))
    {
        ostringstream out;
        out << std::fixed << std::setprecision(1);
        printWKT({ startPos, *this }, out);
        sPlayerbotAIConfig.log("pathfind_attempt_point.csv", out.str().c_str());
    }

    if (sPlayerbotAIConfig.hasLog("pathfind_attempt.csv") && (type == PATHFIND_INCOMPLETE || type == PATHFIND_NORMAL))
    {
        ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
        out << std::fixed << std::setprecision(1) << type << ",";
        printWKT(fromPointsArray(points), out, 1);
        sPlayerbotAIConfig.log("pathfind_attempt.csv", out.str().c_str());
    }    

    if (type == PATHFIND_INCOMPLETE || type == PATHFIND_NORMAL)
        return fromPointsArray(points);
    else
        return {};

}

bool WorldPosition::cropPathTo(vector<WorldPosition>& path, float maxDistance)
{
    if (path.empty())
        return false;

   auto bestPos = std::min_element(path.begin(), path.end(), [this](WorldPosition i, WorldPosition j) {return this->sqDistance(i) < this->sqDistance(j); });

   bool insRange = this->sqDistance(*bestPos) <= maxDistance * maxDistance;

   if (bestPos == path.end())
       return insRange;

   path.erase(std::next(bestPos), path.end());

   return insRange;
}

//A sequential series of pathfinding attempts. Returns the complete path and if the patfinder eventually found a way to the destination.
vector<WorldPosition> WorldPosition::getPathFromPath(vector<WorldPosition> startPath, Unit* bot, uint8 maxAttempt)
{
    //We start at the end of the last path.
    WorldPosition currentPos = startPath.back();

    //No pathfinding across maps.
    if (getMapId() != currentPos.getMapId())
        return { };

    vector<WorldPosition> subPath, fullPath = startPath;

    //Limit the pathfinding attempts
    for (uint32 i = 0; i < maxAttempt; i++)
    {
        //Try to pathfind to this position.
        subPath = getPathStepFrom(currentPos, bot);

        //If we could not find a path return what we have now.
        if (subPath.empty() || currentPos.distance(&subPath.back()) < sPlayerbotAIConfig.targetPosRecalcDistance)
            break;
        
        //Append the path excluding the start (this should be the same as the end of the startPath)
        fullPath.insert(fullPath.end(), std::next(subPath.begin(),1), subPath.end());

        //Are we there yet?
        if (isPathTo(subPath))
            break;

        //Continue pathfinding.
        currentPos = subPath.back();
    }

    return fullPath;
}

uint32 WorldPosition::getUnitsNear(list<ObjectGuid>& units, float radius)
{
    units.remove_if([this, radius](ObjectGuid guid) {return this->sqDistance(WorldPosition(guid)) > radius * radius; });

    return units.size();
};

uint32 WorldPosition::getUnitsAggro(list<ObjectGuid>& units, Player* bot)
{
    units.remove_if([this, bot](ObjectGuid guid) {Unit* unit = GuidPosition(guid).GetUnit(); if (!unit) return true; return this->sqDistance(WorldPosition(guid)) > unit->GetAttackDistance(bot) * unit->GetAttackDistance(bot); });

    return units.size();
};



bool FindPointCreatureData::operator()(CreatureDataPair const& dataPair)
{
    if (!entry || dataPair.second.id == entry)
        if ((!point || dataPair.second.mapid == point.getMapId()) && (!radius || point.sqDistance(WorldPosition(dataPair.second.mapid, dataPair.second.posX, dataPair.second.posY, dataPair.second.posZ)) < radius * radius))
        {
            data.push_back(&dataPair);
        }

    return false;
}

bool FindPointGameObjectData::operator()(GameObjectDataPair const& dataPair)
{
    if (!entry || dataPair.second.id == entry)
        if ((!point || dataPair.second.mapid == point.getMapId()) && (!radius || point.sqDistance(WorldPosition(dataPair.second.mapid, dataPair.second.posX, dataPair.second.posY, dataPair.second.posZ)) < radius * radius))
        {
            data.push_back(&dataPair);
        }

    return false;
}

vector<CreatureDataPair const*> WorldPosition::getCreaturesNear(float radius, uint32 entry)
{
    FindPointCreatureData worker(*this, radius, entry);
    sObjectMgr.DoCreatureData(worker);
    return worker.GetResult();
}

vector<GameObjectDataPair const*> WorldPosition::getGameObjectsNear(float radius, uint32 entry)
{
    FindPointGameObjectData worker(*this, radius, entry);
    sObjectMgr.DoGOData(worker);
    return worker.GetResult();
}

Creature* GuidPosition::GetCreature()
{
    if (!*this)
        return nullptr;

    return getMap()->GetAnyTypeCreature(*this);
}

Unit* GuidPosition::GetUnit()
{
    if (!*this)
        return nullptr;

    if (IsPlayer())
        return sObjectAccessor.FindPlayer(*this);

    return GetCreature();
}

GameObject* GuidPosition::GetGameObject()
{
    if (!*this)
        return nullptr;

    return getMap()->GetGameObject(*this);
}

Player* GuidPosition::GetPlayer()
{
    if (!*this)
        return nullptr;

    if (IsPlayer())
        return sObjectAccessor.FindPlayer(*this);

    return nullptr;
}

bool GuidPosition::isDead()
{
    if (!getMap())
        return false;

    if (!getMap()->IsLoaded(getX(), getY()))
        return false;

    if (IsUnit() && GetUnit() && GetUnit()->IsInWorld() && GetUnit()->IsAlive())
        return false;

    if (IsGameObject() && GetGameObject() && GetGameObject()->IsInWorld())
        return false;

    return true;
}

vector<WorldPosition*> TravelDestination::getPoints(bool ignoreFull) {
    if (ignoreFull)
        return points;

    uint32 max = maxVisitorsPerPoint;

    if (max == 0)
        return points;

    vector<WorldPosition*> retVec;
    std::copy_if(points.begin(), points.end(), std::back_inserter(retVec), [max](WorldPosition* p) { return p->getVisitors() < max; });
    return retVec;
}

WorldPosition* TravelDestination::nearestPoint(WorldPosition* pos) {
    return *std::min_element(points.begin(), points.end(), [pos](WorldPosition* i, WorldPosition* j) {return i->distance(pos) < j->distance(pos); });
}

vector<WorldPosition*> TravelDestination::touchingPoints(WorldPosition* pos) {
    vector<WorldPosition*> ret_points;
    for (auto& point : points)
    {
        float dist = pos->distance(point);
        if (dist == 0)
            continue;

        if (dist > radiusMax * 2)
            continue;

        ret_points.push_back(point);
    }

    return ret_points;
};

vector<WorldPosition*> TravelDestination::sortedPoints(WorldPosition* pos) {
    vector<WorldPosition*> ret_points = points;

    std::sort(ret_points.begin(), ret_points.end(), [pos](WorldPosition* i, WorldPosition* j) {return i->distance(pos) < j->distance(pos); });

    return ret_points;
};

vector <WorldPosition*> TravelDestination::nextPoint(WorldPosition* pos, bool ignoreFull) {
    return sTravelMgr.getNextPoint(pos, ignoreFull ? points : getPoints());
}

bool TravelDestination::isFull(bool ignoreFull) {
    if (!ignoreFull && maxVisitors > 0 && visitors >= maxVisitors)
        return true;

    if (maxVisitorsPerPoint > 0)
        if (getPoints(ignoreFull).empty())
            return true;

    return false;
}

string QuestTravelDestination::getTitle() {
    return ChatHelper::formatQuest(questTemplate);
}

bool QuestRelationTravelDestination::isActive(Player* bot) {
    if (relation == 0)
    {
        if (questTemplate->GetQuestLevel() >= bot->GetLevel() + (uint32)5)
            return false;
        //if (questTemplate->XPValue(bot) == 0)
        //    return false;
        if (!bot->GetMap()->IsContinent() || !bot->CanTakeQuest(questTemplate, false))
            return false;

        PlayerbotAI* ai = bot->GetPlayerbotAI();
        AiObjectContext* context = ai->GetAiObjectContext();

        uint32 dialogStatus = sTravelMgr.getDialogStatus(bot, entry, questTemplate);

        if (AI_VALUE(bool, "can fight equal"))
        {
            if (dialogStatus != DIALOG_STATUS_AVAILABLE)
                return false;
        }
        else
        {
#ifndef MANGOSBOT_TWO
            if (dialogStatus != DIALOG_STATUS_CHAT)
#else
            if (dialogStatus != DIALOG_STATUS_LOW_LEVEL_AVAILABLE)
#endif
                return false;
        }

        //Do not try to pick up dungeon/elite quests in instances without a group.
        if ((questTemplate->GetType() == QUEST_TYPE_ELITE || questTemplate->GetType() == QUEST_TYPE_DUNGEON) && !AI_VALUE(bool, "can fight boss"))
            return false;
    }
    else
    {
        if (!bot->IsActiveQuest(questId))
            return false;

        if (!bot->CanRewardQuest(questTemplate, false))
            return false;

        uint32 dialogStatus = sTravelMgr.getDialogStatus(bot, entry, questTemplate);
        
#ifdef MANGOSBOT_ZERO
        if (dialogStatus != DIALOG_STATUS_REWARD2 && dialogStatus != DIALOG_STATUS_REWARD_REP)
#else
        if (dialogStatus != DIALOG_STATUS_REWARD2 && dialogStatus != DIALOG_STATUS_REWARD && dialogStatus != DIALOG_STATUS_REWARD_REP)
#endif
            return false;

        PlayerbotAI* ai = bot->GetPlayerbotAI();
        AiObjectContext* context = ai->GetAiObjectContext();

        //Do not try to hand-in dungeon/elite quests in instances without a group.
        if ((questTemplate->GetType() == QUEST_TYPE_ELITE || questTemplate->GetType() == QUEST_TYPE_DUNGEON) && !AI_VALUE(bool, "can fight boss"))
        {
            if (!this->nearestPoint(&WorldPosition(bot))->isOverworld())
                return false;
        }
    }

    return true;
}

string QuestRelationTravelDestination::getTitle() {
    ostringstream out;

    if (relation == 0)
        out << "questgiver";
    else
        out << "questtaker";

    out << " " << ChatHelper::formatWorldEntry(entry);
    return out.str();
}

bool QuestObjectiveTravelDestination::isActive(Player* bot) {
    if (questTemplate->GetQuestLevel() > bot->GetLevel() + (uint32)1)
        return false;

    PlayerbotAI* ai = bot->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();
    if (questTemplate->GetQuestLevel() + 5 > (int)bot->GetLevel() && !AI_VALUE(bool, "can fight equal"))
        return false;

    //Check mob level
    if (getEntry() > 0)
    {
        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(getEntry());

        if (cInfo && (int)cInfo->MaxLevel - (int)bot->GetLevel() > 4)
            return false;

        //Do not try to hand-in dungeon/elite quests in instances without a group.
        if (cInfo->Rank > CREATURE_ELITE_NORMAL)
        {
            if (!this->nearestPoint(&WorldPosition(bot))->isOverworld() && !AI_VALUE(bool, "can fight boss"))
                return false;
            else if (!AI_VALUE(bool, "can fight elite"))
                return false;
        }
    }

    if (questTemplate->GetType() == QUEST_TYPE_ELITE && !AI_VALUE(bool, "can fight elite"))
        return false;

    if (!sTravelMgr.getObjectiveStatus(bot, questTemplate, objective))
        return false;

    WorldPosition botPos(bot);

    if (getEntry() > 0 && !isOut(&botPos))
    {
        TravelTarget* target = context->GetValue<TravelTarget*>("travel target")->Get();

        //Only look for the target if it is unique or if we are currently working on it.
        if (points.size() == 1 || (target->getStatus() == TRAVEL_STATUS_WORK && target->getEntry() == getEntry()))
        {
            list<ObjectGuid> targets = AI_VALUE(list<ObjectGuid>, "possible targets");

            for (auto& target : targets)
                if (target.GetEntry() == getEntry() && target.IsCreature() && ai->GetCreature(target) && ai->GetCreature(target)->IsAlive())
                    return true;

            return false;
        }
    }

    return true;
}

string QuestObjectiveTravelDestination::getTitle() {
    ostringstream out;

    out << "objective " << objective;

    if (itemId)
        out << " loot " << ChatHelper::formatItem(sObjectMgr.GetItemPrototype(itemId), 0, 0) << " from";
    else
        out << " to kill";

    out << " " << ChatHelper::formatWorldEntry(entry);
    return out.str();
}

bool RpgTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();

    CreatureInfo const* cInfo = this->getCreatureInfo();

    if (!cInfo)
        return false;

    bool isUsefull = false;

    if (cInfo->NpcFlags & UNIT_NPC_FLAG_VENDOR)
        if (AI_VALUE2_LAZY(bool, "group or", "should sell,can sell,following party,near leader"))
            isUsefull = true;

    if (cInfo->NpcFlags & UNIT_NPC_FLAG_REPAIR)
        if (AI_VALUE2_LAZY(bool, "group or", "should repair,can repair,following party,near leader"))
            isUsefull = true;

    if (!isUsefull)
        return false;

    //Once the target rpged with it is added to the ignore list. We can now move on.
    set<ObjectGuid>& ignoreList = bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<set<ObjectGuid>&>("ignore rpg target")->Get();

    for (auto& i : ignoreList)
    {
        if (i.GetEntry() == getEntry())
        {
            return false;
        }
    }

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->Faction);
    ReputationRank reaction = ai->getReaction(factionEntry);

    return reaction > REP_NEUTRAL;
}

string RpgTravelDestination::getTitle() {
    ostringstream out;

    out << "rpg npc ";

    out << " " << ChatHelper::formatWorldEntry(entry);

    return out.str();
}

bool ExploreTravelDestination::isActive(Player* bot)
{
    AreaTableEntry const* area = GetAreaEntryByAreaID(areaId);

    if (area->area_level && (uint32)area->area_level > bot->GetLevel() && bot->GetLevel() < DEFAULT_MAX_LEVEL)
        return false;

    if (area->exploreFlag == 0xffff)
        return false;
    int offset = area->exploreFlag / 32;

    uint32 val = (uint32)(1 << (area->exploreFlag % 32));
    uint32 currFields = bot->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);

    return !(currFields & val);    
}

//string ExploreTravelDestination::getTitle()
//{
//    return points[0]->getAreaName();
//};

bool GrindTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();
    
    if (!AI_VALUE(bool, "should get money"))
        return false;

    if (AI_VALUE(bool, "should sell"))
        return false;

    CreatureInfo const* cInfo = this->getCreatureInfo();

    int32 botLevel = bot->GetLevel();

    uint8 botPowerLevel = AI_VALUE(uint8, "durability");
    float levelMod = botPowerLevel / 500.0f; //(0-0.2f)
    float levelBoost = botPowerLevel / 50.0f; //(0-2.0f)

    int32 maxLevel = std::max(botLevel * (0.5f + levelMod), botLevel - 5.0f + levelBoost);
 
    if ((int32)cInfo->MaxLevel > maxLevel) //@lvl5 max = 3, @lvl60 max = 57
        return false;

    int32 minLevel = std::max(botLevel * (0.4f + levelMod), botLevel - 12.0f + levelBoost);

    if ((int32)cInfo->MaxLevel < minLevel) //@lvl5 min = 3, @lvl60 max = 50
        return false;

    if (cInfo->MinLootGold == 0)
        return false;

    if (cInfo->Rank > CREATURE_ELITE_NORMAL && !AI_VALUE(bool, "can fight elite"))
        return false;

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->Faction);
    ReputationRank reaction = ai->getReaction(factionEntry);

    return reaction < REP_NEUTRAL;
}

string GrindTravelDestination::getTitle() {
    ostringstream out;

    out << "grind mob ";

    out << " " << ChatHelper::formatWorldEntry(entry);

    return out.str();
}

bool BossTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();

    if (!AI_VALUE(bool, "can fight boss"))
        return false;

    CreatureInfo const* cInfo = getCreatureInfo();

    /*int32 botLevel = bot->GetLevel();

    uint8 botPowerLevel = AI_VALUE(uint8, "durability");
    float levelMod = botPowerLevel / 500.0f; //(0-0.2f)
    float levelBoost = botPowerLevel / 50.0f; //(0-2.0f)

    int32 maxLevel = botLevel + 3.0;

    if ((int32)cInfo->MaxLevel > maxLevel) //@lvl5 max = 3, @lvl60 max = 57
        return false;

    int32 minLevel = botLevel - 10;

    if ((int32)cInfo->MaxLevel < minLevel) //@lvl5 min = 3, @lvl60 max = 50
        return false;
        */

    if ((int32)cInfo->MaxLevel > bot->GetLevel() + 3)
        return false;

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->Faction);
    ReputationRank reaction = ai->getReaction(factionEntry);

    if (reaction >= REP_NEUTRAL)
        return false;

    WorldPosition botPos(bot);

    if (!isOut(&botPos))
    {
        list<ObjectGuid> targets = AI_VALUE(list<ObjectGuid>, "possible targets");

        for (auto& target : targets)
            if (target.GetEntry() == getEntry() && target.IsCreature() && ai->GetCreature(target) && ai->GetCreature(target)->IsAlive())
                return true;

        return false;
    }

    if (!AI_VALUE2(bool, "has upgrade",  getEntry()))
        return false;

    return true;
}

string BossTravelDestination::getTitle() {
    ostringstream out;

    out << "boss mob ";

    out << " " << ChatHelper::formatWorldEntry(entry);

    return out.str();
}

TravelTarget::~TravelTarget() {
    if (!tDestination)
        return;

    releaseVisitors();
    //sTravelMgr.botTargets.erase(std::remove(sTravelMgr.botTargets.begin(), sTravelMgr.botTargets.end(), this), sTravelMgr.botTargets.end());
}

void TravelTarget::setTarget(TravelDestination* tDestination1, WorldPosition* wPosition1, bool groupCopy1) {
    releaseVisitors();

    wPosition = wPosition1;
    tDestination = tDestination1;
    groupCopy = groupCopy1;
    forced = false;
    radius = 0;

    addVisitors();

    setStatus(TRAVEL_STATUS_TRAVEL);
}

void TravelTarget::copyTarget(TravelTarget* target) {
    setTarget(target->tDestination, target->wPosition);
    groupCopy = target->isGroupCopy();
    forced = target->forced;
    extendRetryCount = target->extendRetryCount;
}

void TravelTarget::addVisitors() {
    if (!visitor)
    {
        wPosition->addVisitor();
        tDestination->addVisitor();
    }

    visitor = true;
}

void TravelTarget::releaseVisitors() {
    if (visitor)
    {
        if (tDestination)
            tDestination->remVisitor();
        if (wPosition)
            wPosition->remVisitor();
    }

    visitor = false;
}

void TravelTarget::setStatus(TravelStatus status) {
    m_status = status;
    startTime = WorldTimer::getMSTime();

    switch (m_status) {
    case TRAVEL_STATUS_NONE:
    case TRAVEL_STATUS_PREPARE:
    case TRAVEL_STATUS_EXPIRED:
        statusTime = 1;
        break;
    case TRAVEL_STATUS_TRAVEL:
        statusTime = getMaxTravelTime() * 2 + sPlayerbotAIConfig.maxWaitForMove;
        break;
    case TRAVEL_STATUS_WORK:
        statusTime = tDestination->getExpireDelay();
        break;
    case TRAVEL_STATUS_COOLDOWN:
        statusTime = tDestination->getCooldownDelay();
    }
}

bool TravelTarget::isActive() {
    if (m_status == TRAVEL_STATUS_NONE || m_status == TRAVEL_STATUS_EXPIRED || m_status == TRAVEL_STATUS_PREPARE)
        return false;

    if (forced && isTraveling())
        return true;

    if ((statusTime > 0 && startTime + statusTime < WorldTimer::getMSTime()))
    {
        setStatus(TRAVEL_STATUS_EXPIRED);
        return false;
    }

    if (m_status == TRAVEL_STATUS_COOLDOWN)
        return true;

    if (isTraveling())
        return true;

    if (isWorking())
        return true;   

    if (!tDestination->isActive(bot)) //Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return true;
    }

    return true;
};

bool TravelTarget::isTraveling() {
    if (m_status != TRAVEL_STATUS_TRAVEL)
        return false;

    if (!tDestination->isActive(bot) && !forced) //Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return false;
    }

    WorldPosition pos(bot);

    bool HasArrived = tDestination->isIn(&pos, radius);

    if (HasArrived)
    {
        setStatus(TRAVEL_STATUS_WORK);
        return false;
    }

    if (!ai->HasStrategy("travel", BOT_STATE_NON_COMBAT))
    {
        setTarget(sTravelMgr.nullTravelDestination, sTravelMgr.nullWorldPosition, true);
        return false;
    }

    return true;
}

bool TravelTarget::isWorking() {
    if (m_status != TRAVEL_STATUS_WORK)
        return false;

    if (!tDestination->isActive(bot)) //Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return false;
    }

    WorldPosition pos(bot);

    /*
    bool HasLeft = tDestination->isOut(&pos);

    if (HasLeft)
    {
        setStatus(TRAVEL_STATUS_TRAVEL);
        return false;
    }
    */

    if (!ai->HasStrategy("travel", BOT_STATE_NON_COMBAT))
    {
        setTarget(sTravelMgr.nullTravelDestination, sTravelMgr.nullWorldPosition, true);
        return false;
    }

    return true;
}

bool TravelTarget::isPreparing() {
    if (m_status != TRAVEL_STATUS_PREPARE)
        return false;

    return true;
}

TravelState TravelTarget::getTravelState() {
    if (!tDestination || tDestination->getName() == "NullTravelDestination")
        return TRAVEL_STATE_IDLE;

    if (tDestination->getName() == "QuestRelationTravelDestination")
    {
        if (((QuestRelationTravelDestination*)tDestination)->getRelation() == 0)
        {
            if (isTraveling() || isPreparing())
                return TRAVEL_STATE_TRAVEL_PICK_UP_QUEST;
            if (isWorking())
                return TRAVEL_STATE_WORK_PICK_UP_QUEST;
        }
        else
        {
            if (isTraveling() || isPreparing())
                return TRAVEL_STATE_TRAVEL_HAND_IN_QUEST;
            if (isWorking())
                return TRAVEL_STATE_WORK_HAND_IN_QUEST;
        }
    }
    else if (tDestination->getName() == "QuestObjectiveTravelDestination")
    {
        if (isTraveling() || isPreparing())
            return TRAVEL_STATE_TRAVEL_DO_QUEST;
        if (isWorking())
            return TRAVEL_STATE_WORK_DO_QUEST;
    }
    else if (tDestination->getName() == "RpgTravelDestination")
    {
        return TRAVEL_STATE_TRAVEL_RPG;
    }
    else if (tDestination->getName() == "ExploreTravelDestination")
    {
        return TRAVEL_STATE_TRAVEL_EXPLORE;
    }

    return TRAVEL_STATE_IDLE;
}

void TravelMgr::Clear()
{
#ifdef MANGOS
    sObjectAccessor.DoForAllPlayers([this](Player* plr) { TravelMgr::setNullTravelTarget(plr); });
#endif
#ifdef CMANGOS
#ifndef MANGOSBOT_ZERO
    sObjectAccessor.ExecuteOnAllPlayers([this](Player* plr) { TravelMgr::setNullTravelTarget(plr); });
#else
    HashMapHolder<Player>::ReadGuard g(HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType& m = sObjectAccessor.GetPlayers();
    for (HashMapHolder<Player>::MapType::iterator itr = m.begin(); itr != m.end(); ++itr)
        TravelMgr::setNullTravelTarget(itr->second);
#endif
#endif

    for (auto& quest : quests)
    {
        for (auto& dest : quest.second->questGivers)
        {
            delete dest;
        }

        for (auto& dest : quest.second->questTakers)
        {
            delete dest;
        }

        for (auto& dest : quest.second->questObjectives)
        {
            delete dest;
        }
    }

    questGivers.clear();
    quests.clear();
    pointsMap.clear();
}

void TravelMgr::logEvent(PlayerbotAI* ai, string eventName, string info1, string info2)
{
    if (sPlayerbotAIConfig.hasLog("bot_events.csv"))
    {
        Player* bot = ai->GetBot();

        ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
        out << bot->GetName() << ",";
        out << eventName << ",";
        out << std::fixed << std::setprecision(2);
        WorldPosition(bot).printWKT(out);

        out << to_string(bot->getRace()) << ",";
        out << to_string(bot->getClass()) << ",";
        float subLevel = ((float)bot->GetLevel() + ((float)bot->GetUInt32Value(PLAYER_XP) / (float)bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP)));

        out << subLevel << ",";

        out << "\""<<info1 << "\",";
        out << "\""<<info2 << "\"";

        sPlayerbotAIConfig.log("bot_events.csv", out.str().c_str());
    }
};


void TravelMgr::logEvent(PlayerbotAI* ai, string eventName, ObjectGuid guid, string info2)
{
    string info1 = "";

    Unit* victim;
    if (guid)
    {
        victim = ai->GetUnit(guid);
        if (victim)
            info1 = victim->GetName();
    }

    logEvent(ai, eventName, info1, info2);
};

void TravelMgr::logQuestError(uint32 errorNr, Quest* quest, uint32 objective, uint32 unitId, uint32 itemId)
{
    bool logQuestErrors = false; //For debugging.

    if (!logQuestErrors)
        return;

    if (errorNr == 1)
    {
        string unitName = "<unknown>";
        CreatureInfo const* cInfo = NULL;
        GameObjectInfo const* gInfo = NULL;

        if (unitId > 0)
            cInfo = ObjectMgr::GetCreatureTemplate(unitId);
        else
            gInfo = ObjectMgr::GetGameObjectInfo(unitId * -1);

        if (cInfo)
            unitName = cInfo->Name;
        else if (gInfo)
            unitName = gInfo->name;

        sLog.outString("Quest %s [%d] has %s %s [%d] but none is found in the world.", quest->GetTitle().c_str(), quest->GetQuestId(), objective == 0 ? "quest giver" : "quest taker", unitName.c_str(), unitId);
    }
    else if (errorNr == 2)
    {
        string unitName = "<unknown>";
        CreatureInfo const* cInfo = NULL;
        GameObjectInfo const* gInfo = NULL;

        if (unitId > 0)
            cInfo = ObjectMgr::GetCreatureTemplate(unitId);
        else
            gInfo = ObjectMgr::GetGameObjectInfo(unitId * -1);

        if (cInfo)
            unitName = cInfo->Name;
        else if (gInfo)
            unitName = gInfo->name;

        sLog.outErrorDb("Quest %s [%d] needs %s [%d] for objective %d but none is found in the world.", quest->GetTitle().c_str(), quest->GetQuestId(), unitName.c_str(), unitId, objective);
    }
    else if (errorNr == 3)
    {
        sLog.outErrorDb("Quest %s [%d] needs itemId %d but no such item exists.", quest->GetTitle().c_str(), quest->GetQuestId(), itemId);
    }
    else if (errorNr == 4)
    {
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);

        string unitName = "<unknown>";
        CreatureInfo const* cInfo = NULL;
        GameObjectInfo const* gInfo = NULL;

        if (unitId > 0)
            cInfo = ObjectMgr::GetCreatureTemplate(unitId);
        else
            gInfo = ObjectMgr::GetGameObjectInfo(unitId * -1);

        if (cInfo)
            unitName = cInfo->Name;
        else if (gInfo)
            unitName = gInfo->name;

        sLog.outString("Quest %s [%d] needs %s [%d] for loot of item %s [%d] for objective %d but none is found in the world.", quest->GetTitle().c_str(), quest->GetQuestId(), unitName.c_str(), unitId, proto->Name1, itemId, objective);
    }
    else if (errorNr == 5)
    {
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);

        sLog.outString("Quest %s [%d] needs item %s [%d] for objective %d but none is found in the world.", quest->GetTitle().c_str(), quest->GetQuestId(), proto->Name1, itemId, objective);
    }
    else if (errorNr == 6)
    {
        sLog.outErrorDb("Quest %s [%d] has no quest giver.", quest->GetTitle().c_str(), quest->GetQuestId());
    }
    else if (errorNr == 7)
    {
        sLog.outErrorDb("Quest %s [%d] has no quest taker.", quest->GetTitle().c_str(), quest->GetQuestId());
    }
    else if (errorNr == 8)
    {
        sLog.outErrorDb("Quest %s [%d] has no quest viable quest objective.", quest->GetTitle().c_str(), quest->GetQuestId());
    }
}

void TravelMgr::SetMobAvoidArea() 
{
    sLog.outString("start mob avoidance maps");
    PathFinder path;
    FactionTemplateEntry const* humanFaction = sFactionTemplateStore.LookupEntry(1);
    FactionTemplateEntry const* orcFaction = sFactionTemplateStore.LookupEntry(2);

    for (auto& creaturePair : WorldPosition().getCreaturesNear())
    {

        CreatureData const cData = creaturePair->second;
        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(cData.id);

        if (!cInfo)
            continue;

        WorldPosition point = WorldPosition(cData.mapid, cData.posX, cData.posY, cData.posZ, cData.orientation);

        if (cInfo->NpcFlags > 0)
            continue;

        FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->Faction);
        ReputationRank reactionHum = PlayerbotAI::GetFactionReaction(humanFaction, factionEntry);
        ReputationRank reactionOrc = PlayerbotAI::GetFactionReaction(orcFaction, factionEntry);

        if (reactionHum >= REP_NEUTRAL || reactionOrc >= REP_NEUTRAL)
            continue;

        if (!point.getTerrain())
            continue;

        point.loadMapAndVMap();

        path.setArea(point.getMapId(), point.getX(), point.getY(), point.getZ(), 12, 50.0f);
        path.setArea(point.getMapId(), point.getX(), point.getY(), point.getZ(), 13, 20.0f);
    }
    sLog.outString("end mob avoidance maps");
}

void TravelMgr::LoadQuestTravelTable()
{
    if (!sTravelMgr.quests.empty())
        return;

    // Clearing store (for reloading case)
    Clear();

    /* remove this
    questGuidMap cQuestMap = GAI_VALUE(questGuidMap,"quest objects");

    for (auto cQuest : cQuestMap)
    {
        sLog.outErrorDb("[Quest id: %d]", cQuest.first);

        for (auto cObj : cQuest.second)
        {
            sLog.outErrorDb(" [Objective type: %d]", cObj.first);

            for (auto cCre : cObj.second)
            {
                sLog.outErrorDb(" %s %d", cCre.GetTypeName(), cCre.GetEntry());
            }
        }
    }
    */

    struct unit { uint64 guid; uint32 type; uint32 entry; uint32 map; float  x; float  y; float  z;  float  o; uint32 c; } t_unit;
    vector<unit> units;

    //struct relation { uint32 type; uint32 role;  uint32 entry; uint32 questId; } t_rel;
    //vector<relation> relations;

    //struct loot { uint32 type; uint32 entry;  uint32 item; } t_loot;
    //vector<loot> loots;

    ObjectMgr::QuestMap const& questMap = sObjectMgr.GetQuestTemplates();
    vector<uint32> questIds;

    unordered_map <uint32, uint32> entryCount;

    for (auto& quest : questMap)
        questIds.push_back(quest.first);

    sort(questIds.begin(), questIds.end());

    sLog.outErrorDb("Loading units locations.");
    for (auto& creaturePair : WorldPosition().getCreaturesNear())
    {
        t_unit.type = 0;
        t_unit.guid = ObjectGuid(HIGHGUID_UNIT, creaturePair->second.id, creaturePair->first).GetRawValue();
        t_unit.entry = creaturePair->second.id;
        t_unit.map = creaturePair->second.mapid;
        t_unit.x = creaturePair->second.posX;
        t_unit.y = creaturePair->second.posY;
        t_unit.z = creaturePair->second.posZ;
        t_unit.o = creaturePair->second.orientation;

        entryCount[creaturePair->second.id]++;

        units.push_back(t_unit);
    }

    for (auto& unit : units)
    {
        unit.c = entryCount[unit.entry];
    }

    sLog.outErrorDb("Loading game object locations.");
    for (auto& goPair : WorldPosition().getGameObjectsNear())
    {
        t_unit.type = 1;
        t_unit.guid = ObjectGuid(HIGHGUID_GAMEOBJECT, goPair->second.id, goPair->first).GetRawValue();
        t_unit.entry = goPair->second.id;
        t_unit.map = goPair->second.mapid;
        t_unit.x = goPair->second.posX;
        t_unit.y = goPair->second.posY;
        t_unit.z = goPair->second.posZ;
        t_unit.o = goPair->second.orientation;
        t_unit.c = 1;

        units.push_back(t_unit);
    }

    /*
    //                     0    1  2   3          4          5          6           7     8
    string query = "SELECT 0,guid,id,map,position_x,position_y,position_z,orientation, (select count(*) from creature k where c.id = k.id) FROM creature c UNION ALL SELECT 1,guid,id,map,position_x,position_y,position_z,orientation, (select count(*) from gameobject h where h.id = g.id)  FROM gameobject g";

    QueryResult* result = WorldDatabase.PQuery(query.c_str());

    if (result)
    {
        BarGoLink bar(result->GetRowCount());
        do
        {
            Field* fields = result->Fetch();
            bar.step();

            t_unit.type = fields[0].GetUInt32();
            t_unit.guid = fields[1].GetUInt32();
            t_unit.entry = fields[2].GetUInt32();
            t_unit.map = fields[3].GetUInt32();
            t_unit.x = fields[4].GetFloat();
            t_unit.y = fields[5].GetFloat();
            t_unit.z = fields[6].GetFloat();
            t_unit.o = fields[7].GetFloat();
            t_unit.c = fields[8].GetUInt32();

            units.push_back(t_unit);

        } while (result->NextRow());

        delete result;

        sLog.outString(">> Loaded " SIZEFMTD " units locations.", units.size());
    }
    else
    {
        sLog.outString();
        sLog.outErrorDb(">> Error loading units locations.");
    }

#ifdef MANGOS
    query = "SELECT actor, role, entry, quest FROM quest_relations qr";
#endif
#ifdef CMANGOS
    query = "SELECT 0, 0, id, quest FROM creature_questrelation UNION ALL SELECT 0, 1, id, quest FROM creature_involvedrelation UNION ALL SELECT 1, 0, id, quest FROM gameobject_questrelation UNION ALL SELECT 1, 1, id, quest FROM gameobject_involvedrelation";
#endif

    result = WorldDatabase.PQuery(query.c_str());

    if (result)
    {
        BarGoLink bar(result->GetRowCount());

        do
        {
            Field* fields = result->Fetch();
            bar.step();

            t_rel.type = fields[0].GetUInt32();
            t_rel.role = fields[1].GetUInt32();
            t_rel.entry = fields[2].GetUInt32();
            t_rel.questId = fields[3].GetUInt32();

            relations.push_back(t_rel);

        } while (result->NextRow());

        delete result;

        sLog.outString(">> Loaded " SIZEFMTD " relations.", relations.size());
    }
    else
    {
        sLog.outString();
        sLog.outErrorDb(">> Error loading relations.");
    }

    query = "SELECT 0, ct.entry, item FROM creature_template ct JOIN creature_loot_template clt ON (ct.lootid = clt.entry) UNION ALL SELECT 0, entry, item FROM npc_vendor UNION ALL SELECT 1, gt.entry, item FROM gameobject_template gt JOIN gameobject_loot_template glt ON (gt.TYPE = 3 AND gt.DATA1 = glt.entry)";

    result = WorldDatabase.PQuery(query.c_str());

    if (result)
    {
        BarGoLink bar(result->GetRowCount());

        do
        {
            Field* fields = result->Fetch();
            bar.step();

            t_loot.type = fields[0].GetUInt32();
            t_loot.entry = fields[1].GetUInt32();
            t_loot.item = fields[2].GetUInt32();

            loots.push_back(t_loot);

        } while (result->NextRow());

        delete result;

        sLog.outString(">> Loaded " SIZEFMTD " loot lists.", loots.size());
    }
    else
    {
        sLog.outString();
        sLog.outErrorDb(">> Error loading loot lists.");
    }
    */

    sLog.outErrorDb("Loading quest data.");

    bool loadQuestData = true;

    if (loadQuestData)
    {
        questGuidpMap questMap = GAI_VALUE(questGuidpMap, "quest guidp map");

        for (auto& q : questMap)
        {
            uint32 questId = q.first;

            QuestContainer* container = new QuestContainer;

            for (auto& r : q.second)
            {
                uint32 flag = r.first;

                for (auto& e : r.second)
                {
                    int32 entry = e.first;

                    QuestTravelDestination* loc;
                    vector<QuestTravelDestination*> locs;

                    if (flag & (uint32)QuestRelationFlag::questGiver)
                    {
                        loc = new QuestRelationTravelDestination(questId, entry, 0, sPlayerbotAIConfig.tooCloseDistance, sPlayerbotAIConfig.sightDistance);
                        loc->setExpireDelay(5 * 60 * 1000);
                        loc->setMaxVisitors(15, 0);
                        container->questGivers.push_back(loc);
                        locs.push_back(loc);
                    }
                    if (flag & (uint32)QuestRelationFlag::questTaker)
                    {
                        loc = new QuestRelationTravelDestination(questId, entry, 1, sPlayerbotAIConfig.tooCloseDistance, sPlayerbotAIConfig.sightDistance);
                        loc->setExpireDelay(5 * 60 * 1000);
                        loc->setMaxVisitors(15, 0);
                        container->questTakers.push_back(loc);
                        locs.push_back(loc);
                    }
                    if(flag & ((uint32)QuestRelationFlag::objective1 | (uint32)QuestRelationFlag::objective2 | (uint32)QuestRelationFlag::objective3 | (uint32)QuestRelationFlag::objective4))
                    {
                        uint32 objective;
                        if (flag & (uint32)QuestRelationFlag::objective1)
                            objective = 0;
                        else if (flag & (uint32)QuestRelationFlag::objective2)
                            objective = 1;
                        else if (flag & (uint32)QuestRelationFlag::objective3)
                            objective = 2;
                        else if (flag & (uint32)QuestRelationFlag::objective4)
                            objective = 3;

                        loc = new QuestObjectiveTravelDestination(questId, entry, objective, sPlayerbotAIConfig.tooCloseDistance, sPlayerbotAIConfig.sightDistance);
                        loc->setExpireDelay(1 * 60 * 1000);
                        loc->setMaxVisitors(100, 1);
                        container->questObjectives.push_back(loc);
                        locs.push_back(loc);
                    }

                    for (auto& guidP : e.second)
                    {
                        WorldPosition point = guidP;
                        pointsMap.insert(make_pair(guidP.GetRawValue(), point));

                        for (auto tLoc : locs)
                        {
                            tLoc->addPoint(&pointsMap.find(guidP.GetRawValue())->second);
                        }
                    }
                }
            }

            if (!container->questTakers.empty())
            {
                quests.insert(make_pair(questId, container));

                for (auto loc : container->questGivers)
                    questGivers.push_back(loc);
            }
        }
    }

    sLog.outErrorDb("Loading Rpg, Grind and Boss locations.");

    WorldPosition point;

    //Rpg locations
    for (auto& u : units)
    {
        RpgTravelDestination* rLoc;
        GrindTravelDestination* gLoc;
        BossTravelDestination* bLoc;

        if (u.type != 0)
            continue;

        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(u.entry);

        if (!cInfo)
            continue;

        if (cInfo->ExtraFlags & CREATURE_EXTRA_FLAG_INVISIBLE)
            continue;

        vector<uint32> allowedNpcFlags;

        allowedNpcFlags.push_back(UNIT_NPC_FLAG_INNKEEPER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_GOSSIP);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_QUESTGIVER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_FLIGHTMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_BANKER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_AUCTIONEER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_STABLEMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_PETITIONER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TABARDDESIGNER);

        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_REPAIR);

        point = WorldPosition(u.map, u.x, u.y, u.z, u.o);

        for (vector<uint32>::iterator i = allowedNpcFlags.begin(); i != allowedNpcFlags.end(); ++i)
        {
            if ((cInfo->NpcFlags & *i) != 0)
            {
                rLoc = new RpgTravelDestination(u.entry, sPlayerbotAIConfig.tooCloseDistance, sPlayerbotAIConfig.sightDistance);
                rLoc->setExpireDelay(5 * 60 * 1000);
                rLoc->setMaxVisitors(15, 0);


                pointsMap.insert_or_assign(u.guid, point);
                rLoc->addPoint(&pointsMap.find(u.guid)->second);
                rpgNpcs.push_back(rLoc);
                break;
            }
        }

        if (cInfo->MinLootGold > 0)
        {
            gLoc = new GrindTravelDestination(u.entry, sPlayerbotAIConfig.tooCloseDistance, sPlayerbotAIConfig.sightDistance);
            gLoc->setExpireDelay(5 * 60 * 1000);
            gLoc->setMaxVisitors(100, 0);

            point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
            pointsMap.insert_or_assign(u.guid, point);
            gLoc->addPoint(&pointsMap.find(u.guid)->second);
            grindMobs.push_back(gLoc);
        }

        if (cInfo->Rank == 3 || (cInfo->Rank == 1 && !point.isOverworld() && u.c == 1))
        {
            string nodeName = cInfo->Name;

            bLoc = new BossTravelDestination(u.entry, sPlayerbotAIConfig.tooCloseDistance, sPlayerbotAIConfig.sightDistance);
            bLoc->setExpireDelay(5 * 60 * 1000);
            bLoc->setMaxVisitors(0, 0);

            pointsMap.insert_or_assign(u.guid, point);
            bLoc->addPoint(&pointsMap.find(u.guid)->second);
            bossMobs.push_back(bLoc);
        }
    }

    sLog.outErrorDb("Loading Explore locations.");

    //Explore points
    for (auto& u : units)
    {
        ExploreTravelDestination* loc;

        WorldPosition point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
        AreaTableEntry const* area = point.getArea();

        if (!area)
            continue;

        if (!area->exploreFlag)
            continue;

        if (u.type == 1) 
            continue;

        auto iloc = exploreLocs.find(area->ID);

        int32 guid = u.type == 0 ? u.guid : u.guid * -1;

        pointsMap.insert_or_assign(guid, point);

        if (iloc == exploreLocs.end())
        {
            loc = new ExploreTravelDestination(area->ID, sPlayerbotAIConfig.tooCloseDistance, sPlayerbotAIConfig.sightDistance);
            loc->setMaxVisitors(1000, 0);
            loc->setCooldownDelay(1000);
            loc->setExpireDelay(1000);
            loc->setTitle(area->area_name[0]);
            exploreLocs.insert_or_assign(area->ID, loc);
        }
        else
        {
            loc = iloc->second;
        }

        loc->addPoint(&pointsMap.find(guid)->second);
    }     


     //Clear these logs files
    sPlayerbotAIConfig.openLog("zones.csv", "w");
    sPlayerbotAIConfig.openLog("creatures.csv", "w");
    sPlayerbotAIConfig.openLog("gos.csv", "w");
    sPlayerbotAIConfig.openLog("bot_movement.csv", "w");
    sPlayerbotAIConfig.openLog("bot_pathfinding.csv", "w");
    sPlayerbotAIConfig.openLog("pathfind_attempt.csv", "w");
    sPlayerbotAIConfig.openLog("pathfind_attempt_point.csv", "w");
    sPlayerbotAIConfig.openLog("pathfind_result.csv", "w");
    sPlayerbotAIConfig.openLog("load_map_grid.csv", "w");
    sPlayerbotAIConfig.openLog("strategy.csv", "w");

    sPlayerbotAIConfig.openLog("unload_grid.csv", "w");
    sPlayerbotAIConfig.openLog("unload_obj.csv", "w");
    sPlayerbotAIConfig.openLog("bot_events.csv", "w");
    sPlayerbotAIConfig.openLog("travel_map.csv", "w");
    sPlayerbotAIConfig.openLog("quest_map.csv", "w");

#ifdef IKE_PATHFINDER
    bool mmapAvoidMobMod = true;

    if (mmapAvoidMobMod)
    {
        //Mob avoidance
        SetMobAvoidArea();
    }
#endif

    sTravelNodeMap.loadNodeStore();

    sTravelNodeMap.generateAll();


    sTravelNodeMap.printMap();
    sTravelNodeMap.printNodeStore();
    sTravelNodeMap.saveNodeStore();

    //Creature/gos/zone export.
    if (sPlayerbotAIConfig.hasLog("creatures.csv"))
    {
        for (auto& creaturePair : WorldPosition().getCreaturesNear())
        {
            CreatureData const cData = creaturePair->second;
            CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(cData.id);

            if (!cInfo)
                continue;

            WorldPosition point = WorldPosition(cData.mapid, cData.posX, cData.posY, cData.posZ, cData.orientation);

            string name = cInfo->Name;
            name.erase(remove(name.begin(), name.end(), ','), name.end());
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());

            ostringstream out;
            out << name << ",";
            point.printWKT(out);
            out << cInfo->MaxLevel << ",";
            out << cInfo->Rank << ",";
            out << cInfo->Faction << ",";
            out << cInfo->NpcFlags << ",";
            out << point.getAreaName() << ",";
            out << std::fixed;

            sPlayerbotAIConfig.log("creatures.csv", out.str().c_str());
        }
    }

    if (sPlayerbotAIConfig.hasLog("vmangoslines.csv"))
    {

        uint32 mapId = 0;
        vector<WorldPosition> pos;

            static float const topNorthSouthLimit[] = {
                2032.048340f, -6927.750000f,
                1634.863403f, -6157.505371f,
                1109.519775f, -5181.036133f,
                1315.204712f, -4096.020508f,
                1073.089233f, -3372.571533f,
                 825.833191f, -3125.778809f,
                 657.343994f, -2314.813232f,
                 424.736145f, -1888.283691f,
                 744.395813f, -1647.935425f,
                1424.160645f,  -654.948181f,
                1447.065308f,  -169.751358f,
                1208.715454f,   189.748703f,
                1596.240356f,   998.616699f,
                1577.923706f,  1293.419922f,
                1458.520264f,  1727.373291f,
                1591.916138f,  3728.139404f
            };

            pos.clear();

# define my_sizeof(type) ((char *)(&type+1)-(char*)(&type))

            int size = my_sizeof(topNorthSouthLimit) / my_sizeof(topNorthSouthLimit[0]);

            for (int32 i = 0; i < size-1; i=i+2)
            {
                if (topNorthSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, topNorthSouthLimit[i], topNorthSouthLimit[i + 1], 0));
            }

            ostringstream out;
            out << "topNorthSouthLimit" << ",";
            WorldPosition().printWKT(pos,out,1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const ironforgeAreaSouthLimit[] = {
                -7491.33f,  3093.74f,
                -7472.04f,  -391.88f,
                -6366.68f,  -730.10f,
                -6063.96f, -1411.76f,
                -6087.62f, -2190.21f,
                -6349.54f, -2533.66f,
                -6308.63f, -3049.32f,
                -6107.82f, -3345.30f,
                -6008.49f, -3590.52f,
                -5989.37f, -4312.29f,
                -5806.26f, -5864.11f
            };

            pos.clear();

            size = my_sizeof(ironforgeAreaSouthLimit) / my_sizeof(ironforgeAreaSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (ironforgeAreaSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, ironforgeAreaSouthLimit[i], ironforgeAreaSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();
            
            out << "ironforgeAreaSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const stormwindAreaNorthLimit[] = {
                 -8004.25f,  3714.11f,
                 -8075.00f, -179.00f,
                 -8638.00f, 169.00f,
                 -9044.00f, 35.00f,
                 -9068.00f, -125.00f,
                 -9094.00f, -147.00f,
                 -9206.00f, -290.00f,
                 -9097.00f, -510.00f,
                 -8739.00f, -501.00f,
                 -8725.50f, -1618.45f,
                 -9810.40f, -1698.41f,
                -10049.60f, -1740.40f,
                -10670.61f, -1692.51f,
                -10908.48f, -1563.87f,
                -13006.40f, -1622.80f,
                -12863.23f, -4798.42f
            };

            pos.clear();

            size = my_sizeof(stormwindAreaNorthLimit) / my_sizeof(stormwindAreaNorthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (stormwindAreaNorthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, stormwindAreaNorthLimit[i], stormwindAreaNorthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "stormwindAreaNorthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const stormwindAreaSouthLimit[] = {
                 -8725.337891f,  3535.624023f,
                 -9525.699219f,   910.132568f,
                 -9796.953125f,   839.069580f,
                 -9946.341797f,   743.102844f,
                -10287.361328f,   760.076477f,
                -10083.828125f,   380.389893f,
                -10148.072266f,    80.056450f,
                -10014.583984f,  -161.638519f,
                 -9978.146484f,  -361.638031f,
                 -9877.489258f,  -563.304871f,
                 -9980.967773f, -1128.510498f,
                 -9991.717773f, -1428.793213f,
                 -9887.579102f, -1618.514038f,
                -10169.600586f, -1801.582031f,
                 -9966.274414f, -2227.197754f,
                 -9861.309570f, -2989.841064f,
                 -9944.026367f, -3205.886963f,
                 -9610.209961f, -3648.369385f,
                 -7949.329590f, -4081.389404f,
                 -7910.859375f, -5855.578125f
            };

            pos.clear();

            size = my_sizeof(stormwindAreaSouthLimit) / my_sizeof(stormwindAreaSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (stormwindAreaSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, stormwindAreaSouthLimit[i], stormwindAreaSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "stormwindAreaSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());
            
            mapId = 1;
       
            static float const northMiddleLimit[] = {
                  -2280.00f,  4054.00f,
                  -2401.00f,  2365.00f,
                  -2432.00f,  1338.00f,
                  -2286.00f,   769.00f,
                  -2137.00f,   662.00f,
                  -2044.54f,   489.86f,
                  -1808.52f,   436.39f,
                  -1754.85f,   504.55f,
                  -1094.55f,   651.75f,
                   -747.46f,   647.73f,
                   -685.55f,   408.43f,
                   -311.38f,   114.43f,
                   -358.40f,  -587.42f,
                   -377.92f,  -748.70f,
                   -512.57f,  -919.49f,
                   -280.65f, -1008.87f,
                    -81.29f,  -930.89f,
                    284.31f, -1105.39f,
                    568.86f,  -892.28f,
                   1211.09f, -1135.55f,
                    879.60f, -2110.18f,
                    788.96f, -2276.02f,
                    899.68f, -2625.56f,
                   1281.54f, -2689.42f,
                   1521.82f, -3047.85f,
                   1424.22f, -3365.69f,
                   1694.11f, -3615.20f,
                   2373.78f, -4019.96f,
                   2388.13f, -5124.35f,
                   2193.79f, -5484.38f,
                   1703.57f, -5510.53f,
                   1497.59f, -6376.56f,
                   1368.00f, -8530.00f
            };

            pos.clear();

            size = my_sizeof(northMiddleLimit) / my_sizeof(northMiddleLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (northMiddleLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, northMiddleLimit[i], northMiddleLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "northMiddleLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const durotarSouthLimit[] = {
                    2755.00f, -3766.00f,
                    2225.00f, -3596.00f,
                    1762.00f, -3746.00f,
                    1564.00f, -3943.00f,
                    1184.00f, -3915.00f,
                     737.00f, -3782.00f,
                     -75.00f, -3742.00f,
                    -263.00f, -3836.00f,
                    -173.00f, -4064.00f,
                     -81.00f, -4091.00f,
                     -49.00f, -4089.00f,
                     -16.00f, -4187.00f,
                      -5.00f, -4192.00f,
                     -14.00f, -4551.00f,
                    -397.00f, -4601.00f,
                    -522.00f, -4583.00f,
                    -668.00f, -4539.00f,
                    -790.00f, -4502.00f,
                   -1176.00f, -4213.00f,
                   -1387.00f, -4674.00f,
                   -2243.00f, -6046.00f
            };

            pos.clear();

            size = my_sizeof(durotarSouthLimit) / my_sizeof(durotarSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (durotarSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, durotarSouthLimit[i], durotarSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "durotarSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const valleyoftrialsSouthLimit[] = {
                    -324.00f, -3869.00f,
                    -774.00f, -3992.00f,
                    -965.00f, -4290.00f,
                    -932.00f, -4349.00f,
                    -828.00f, -4414.00f,
                    -661.00f, -4541.00f,
                    -521.00f, -4582.00f
            };

            pos.clear();

            size = my_sizeof(valleyoftrialsSouthLimit) / my_sizeof(valleyoftrialsSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (valleyoftrialsSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, valleyoftrialsSouthLimit[i], valleyoftrialsSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "valleyoftrialsSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const middleToSouthLimit[] = {
                        -2402.01f,      4255.70f,
                    -2475.933105f,  3199.568359f, // Desolace
                    -2344.124023f,  1756.164307f,
                    -2826.438965f,   403.824738f, // Mulgore
                    -3472.819580f,   182.522476f, // Feralas
                    -4365.006836f, -1602.575439f, // the Barrens
                    -4515.219727f, -1681.356079f,
                    -4543.093750f, -1882.869385f, // Thousand Needles
                        -4824.16f,     -2310.11f,
                    -5102.913574f, -2647.062744f,
                    -5248.286621f, -3034.536377f,
                    -5246.920898f, -3339.139893f,
                    -5459.449707f, -4920.155273f, // Tanaris
                        -5437.00f,     -5863.00f
            };

            pos.clear();

            size = my_sizeof(middleToSouthLimit) / my_sizeof(middleToSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (middleToSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, middleToSouthLimit[i], middleToSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "middleToSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const orgrimmarSouthLimit[] = {
                    2132.5076f, -3912.2478f,
                    1944.4298f, -3855.2583f,
                    1735.6906f, -3834.2417f,
                    1654.3671f, -3380.9902f,
                    1593.9861f, -3975.5413f,
                    1439.2548f, -4249.6923f,
                    1436.3106f, -4007.8950f,
                    1393.3199f, -4196.0625f,
                    1445.2428f, -4373.9052f,
                    1407.2349f, -4429.4145f,
                    1464.7142f, -4545.2875f,
                    1584.1331f, -4596.8764f,
                    1716.8065f, -4601.1323f,
                    1875.8312f, -4788.7187f,
                    1979.7647f, -4883.4585f,
                    2219.1562f, -4854.3330f
            };

            pos.clear();

            size = my_sizeof(orgrimmarSouthLimit) / my_sizeof(orgrimmarSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (orgrimmarSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, orgrimmarSouthLimit[i], orgrimmarSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "orgrimmarSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const feralasThousandNeedlesSouthLimit[] = {
                    -6495.4995f, -4711.981f,
                    -6674.9995f, -4515.0019f,
                    -6769.5717f, -4122.4272f,
                    -6838.2651f, -3874.2792f,
                    -6851.1314f, -3659.1179f,
                    -6624.6845f, -3063.3843f,
                    -6416.9067f, -2570.1301f,
                    -5959.8466f, -2287.2634f,
                    -5947.9135f, -1866.5028f,
                    -5947.9135f,  -820.4881f,
                    -5876.7114f,    -3.5138f,
                    -5876.7114f,   917.6407f,
                    -6099.3603f,  1153.2884f,
                    -6021.8989f,  1638.1809f,
                    -6091.6176f,  2335.8892f,
                    -6744.9946f,  2393.4855f,
                    -6973.8608f,  3077.0281f,
                    -7068.7241f,  4376.2304f,
                    -7142.1211f,  4808.4331f
            };


            pos.clear();

            size = my_sizeof(feralasThousandNeedlesSouthLimit) / my_sizeof(feralasThousandNeedlesSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (feralasThousandNeedlesSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, feralasThousandNeedlesSouthLimit[i], feralasThousandNeedlesSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "feralasThousandNeedlesSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());
    }

    if (sPlayerbotAIConfig.hasLog("gos.csv"))
    {
        for (auto& gameObjectPair : WorldPosition().getGameObjectsNear())
        {
            GameObjectData const gData = gameObjectPair->second;
            auto data = sGOStorage.LookupEntry<GameObjectInfo>(gData.id);

            if (!data)
                continue;

            WorldPosition point = WorldPosition(gData.mapid, gData.posX, gData.posY, gData.posZ, gData.orientation);

            string name = data->name;
            name.erase(remove(name.begin(), name.end(), ','), name.end());
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());

            ostringstream out;
            out << name << ",";
            point.printWKT(out);
            out << data->type << ",";
            out << point.getAreaName() << ",";
            out << std::fixed;

            sPlayerbotAIConfig.log("gos.csv", out.str().c_str());
        }
    }

    if (sPlayerbotAIConfig.hasLog("zones.csv"))
    {
        std::unordered_map<string, vector<WorldPosition>> zoneLocs;

        vector<WorldPosition> Locs = {};
        
        for (auto& u : units)
        {
            WorldPosition point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
            string name = to_string(u.map) + point.getAreaName();

            if (zoneLocs.find(name) == zoneLocs.end())
                zoneLocs.insert_or_assign(name, Locs);

            zoneLocs.find(name)->second.push_back(point);            
        }        

        for (auto& loc : zoneLocs)
        {
            if (loc.second.empty())
                continue;

            if (!sTravelNodeMap.getMapOffset(loc.second.front().getMapId()) && loc.second.front().getMapId() != 0)
                continue;

            vector<WorldPosition> points = loc.second;;
           
            ostringstream out; 

            WorldPosition pos = WorldPosition(points, WP_MEAN_CENTROID);

            out << "\"center\"" << ",";
            out << points.begin()->getMapId() << ",";
            out << points.begin()->getAreaName() << ",";
            out << points.begin()->getAreaName(true, true) << ",";

            pos.printWKT(out);

            out << "\n";
            
            out << "\"area\"" << ",";
            out << points.begin()->getMapId() << ",";
            out << points.begin()->getAreaName() << ",";
            out << points.begin()->getAreaName(true, true) << ",";

            point.printWKT(points, out, 0);

            sPlayerbotAIConfig.log("zones.csv", out.str().c_str());
        }
    }

    if (sPlayerbotAIConfig.hasLog("quest_map.csv"))
    {
        for (auto container : quests)
        {
            vector<pair<uint32, QuestTravelDestination*>> printQuestMap;

            for (auto dest : container.second->questGivers)
                printQuestMap.push_back(make_pair(0, dest));

            for (auto dest : container.second->questObjectives)
                printQuestMap.push_back(make_pair(1, dest));

            for (auto dest : container.second->questTakers)
                printQuestMap.push_back(make_pair(2, dest));

            for (auto dest : printQuestMap)
            {
                ostringstream out;

                out << std::fixed << std::setprecision(2);
                out << to_string(dest.first) << ",";
                out << to_string(dest.second->GetQuestTemplate()->GetQuestId()) << ",";
                out << "\"" << dest.second->GetQuestTemplate()->GetTitle() << "\"" << ",";
                if (dest.second->getName() == "QuestObjectiveTravelDestination")
                    out << to_string(((QuestObjectiveTravelDestination*)dest.second)->getObjective()) << ",";
                else
                    out << to_string(0) << ",";

                out << to_string(dest.second->getEntry()) << ",";

                vector<WorldPosition> points;

                for (auto p : dest.second->getPoints())
                    points.push_back(*p);

                WorldPosition().printWKT(points, out, 0);

                out << to_string(dest.second->GetQuestTemplate()->GetQuestLevel()) << ",";
                out << to_string(dest.second->GetQuestTemplate()->GetMinLevel()) << ",";
                out << to_string(dest.second->GetQuestTemplate()->GetMaxLevel()) << ",";
                out << to_string((uint32(ceilf(dest.second->GetQuestTemplate()->GetRewMoneyMaxLevel() / 0.6))));

                sPlayerbotAIConfig.log("quest_map.csv", out.str().c_str());
            }
        }
    }

    bool printStrategyMap = false;

    if (printStrategyMap && sPlayerbotAIConfig.hasLog("strategy.csv"))
    {
        static map<uint8, string> classes;
        static map<uint8, map<uint8, string> > specs;
        classes[CLASS_DRUID] = "druid";
        specs[CLASS_DRUID][0] = "balance";
        specs[CLASS_DRUID][1] = "feral combat";
        specs[CLASS_DRUID][2] = "restoration";

        classes[CLASS_HUNTER] = "hunter";
        specs[CLASS_HUNTER][0] = "beast mastery";
        specs[CLASS_HUNTER][1] = "marksmanship";
        specs[CLASS_HUNTER][2] = "survival";

        classes[CLASS_MAGE] = "mage";
        specs[CLASS_MAGE][0] = "arcane";
        specs[CLASS_MAGE][1] = "fire";
        specs[CLASS_MAGE][2] = "frost";

        classes[CLASS_PALADIN] = "paladin";
        specs[CLASS_PALADIN][0] = "holy";
        specs[CLASS_PALADIN][1] = "protection";
        specs[CLASS_PALADIN][2] = "retribution";

        classes[CLASS_PRIEST] = "priest";
        specs[CLASS_PRIEST][0] = "discipline";
        specs[CLASS_PRIEST][1] = "holy";
        specs[CLASS_PRIEST][2] = "shadow";

        classes[CLASS_ROGUE] = "rogue";
        specs[CLASS_ROGUE][0] = "assasination";
        specs[CLASS_ROGUE][1] = "combat";
        specs[CLASS_ROGUE][2] = "subtlety";

        classes[CLASS_SHAMAN] = "shaman";
        specs[CLASS_SHAMAN][0] = "elemental";
        specs[CLASS_SHAMAN][1] = "enhancement";
        specs[CLASS_SHAMAN][2] = "restoration";

        classes[CLASS_WARLOCK] = "warlock";
        specs[CLASS_WARLOCK][0] = "affliction";
        specs[CLASS_WARLOCK][1] = "demonology";
        specs[CLASS_WARLOCK][2] = "destruction";

        classes[CLASS_WARRIOR] = "warrior";
        specs[CLASS_WARRIOR][0] = "arms";
        specs[CLASS_WARRIOR][1] = "fury";
        specs[CLASS_WARRIOR][2] = "protection";

#ifdef MANGOSBOT_TWO
        classes[CLASS_DEATH_KNIGHT] = "dk";
        specs[CLASS_DEATH_KNIGHT][0] = "blood";
        specs[CLASS_DEATH_KNIGHT][1] = "frost";
        specs[CLASS_DEATH_KNIGHT][2] = "unholy";
#endif

        //Use randombot 0.
        ostringstream cout; cout << sPlayerbotAIConfig.randomBotAccountPrefix << 0;
        string accountName = cout.str();

        QueryResult* results = LoginDatabase.PQuery("SELECT id FROM account where username = '%s'", accountName.c_str());
        if (results)
        {

            Field* fields = results->Fetch();
            uint32 accountId = fields[0].GetUInt32();

            WorldSession* session = new WorldSession(accountId, NULL, SEC_PLAYER,
#ifndef MANGOSBOT_ZERO
                2, 0, LOCALE_enUS, accountName.c_str(), 0, 0, false);
#else
                0, LOCALE_enUS, accountName.c_str(), 0);
#endif

            vector <pair<pair<uint32, uint32>, uint32>> classSpecLevel;

            std::unordered_map<string, vector<pair<pair<uint32, uint32>, uint32>>> actions;

            ostringstream out;

            for (uint8 race = RACE_HUMAN; race < MAX_RACES; race++)
            {
                for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
                {
#ifdef MANGOSBOT_TWO
                    if (cls != 10)
#else
                    if (cls != 10 && cls != 6)
#endif
                    {
                        Player* player = new Player(session);

                        if (player->Create(sObjectMgr.GeneratePlayerLowGuid(), "dummy",
                            race, //race
                            cls, //class
                            1, //gender
                            1, // skinColor,
                            1,
                            1,
                            1, // hairColor,
                            1, 0))
                        {

                            for (uint8 tab = 0; tab < 3; tab++)
                            {
                                TalentSpec newSpec;
                                if (tab == 0)
                                    newSpec = TalentSpec(player, "1-0-0");
                                else if (tab == 1)
                                    newSpec = TalentSpec(player, "0-1-0");
                                else
                                    newSpec = TalentSpec(player, "0-0-1");

                                for (uint32 lvl = 1; lvl < MAX_LEVEL; lvl++)
                                {
                                    player->SetLevel(lvl);

                                    ostringstream tout;
                                    newSpec.ApplyTalents(player, &tout);

                                    PlayerbotAI* ai = new PlayerbotAI(player);

                                    ai->ResetStrategies(false);

                                    AiObjectContext* con = ai->GetAiObjectContext();

                                    list<string> tstrats;
                                    set<string> strategies, sstrats;

                                    tstrats = ai->GetStrategies(BOT_STATE_COMBAT);
                                    sstrats = con->GetSupportedStrategies();
                                    if (!sstrats.empty())
                                        strategies.insert(tstrats.begin(), tstrats.end());

                                    tstrats = ai->GetStrategies(BOT_STATE_NON_COMBAT);
                                    if (!tstrats.empty())
                                        strategies.insert(tstrats.begin(), tstrats.end());

                                    tstrats = ai->GetStrategies(BOT_STATE_DEAD);
                                    if (!tstrats.empty())
                                        strategies.insert(tstrats.begin(), tstrats.end());

                                    sstrats = con->GetSupportedStrategies();
                                    if(!sstrats.empty())
                                        strategies.insert(sstrats.begin(), sstrats.end());

                                    for (auto& stratName : strategies)
                                    {
                                        Strategy* strat = con->GetStrategy(stratName);

                                        if (strat->getDefaultActions())
                                            for (int32 i = 0; i < NextAction::size(strat->getDefaultActions()); i++)
                                            {
                                                NextAction* nextAction = strat->getDefaultActions()[i];

                                                ostringstream aout;

                                                aout << nextAction->getRelevance() << "," << nextAction->getName() << ",,S:" << stratName;

                                                if (actions.find(aout.str().c_str()) != actions.end())
                                                    classSpecLevel = actions.find(aout.str().c_str())->second;
                                                else
                                                    classSpecLevel.clear();

                                                classSpecLevel.push_back(make_pair(make_pair(cls, tab), lvl));

                                                actions.insert_or_assign(aout.str().c_str(), classSpecLevel);
                                            }

                                        std::list<TriggerNode*> triggers;
                                        strat->InitTriggers(triggers);
                                        for (auto& triggerNode : triggers)
                                        {
                                            //out << " TN:" << triggerNode->getName();

                                            Trigger* trigger = con->GetTrigger(triggerNode->getName());

                                            if (trigger)
                                            {

                                                triggerNode->setTrigger(trigger);

                                                NextAction** nextActions = triggerNode->getHandlers();

                                                for (int32 i = 0; i < NextAction::size(nextActions); i++)
                                                {
                                                    NextAction* nextAction = nextActions[i];
                                                    //out << " A:" << nextAction->getName() << "(" << nextAction->getRelevance() << ")";

                                                    ostringstream aout;

                                                    aout << nextAction->getRelevance() << "," << nextAction->getName() << "," << triggerNode->getName() << "," << stratName;

                                                    if (actions.find(aout.str().c_str()) != actions.end())
                                                        classSpecLevel = actions.find(aout.str().c_str())->second;
                                                    else
                                                        classSpecLevel.clear();

                                                    classSpecLevel.push_back(make_pair(make_pair(cls, tab), lvl));

                                                    actions.insert_or_assign(aout.str().c_str(), classSpecLevel);
                                                }
                                            }
                                        }
                                    }

                                    delete ai;
                                }
                            }                            
                        }
                        delete player;
                    }
                }
            }

            vector< string> actionKeys;

            for (auto& action : actions)
                actionKeys.push_back(action.first);

            std::sort(actionKeys.begin(), actionKeys.end(), [](string i, string j)
                {stringstream is(i); stringstream js(j); float iref, jref; string iact, jact, itrig, jtrig, istrat, jstrat;
            is >> iref >> iact >> itrig >> istrat;
            js >> jref >> jact >> jtrig >> jstrat;
            if (iref > jref)
                return true;
            if (iref == jref && istrat < jstrat)
                return true;
            if (iref == jref && !(istrat > jstrat) && iact < jact)
                return true;
            if (iref == jref && !(istrat > jstrat) && !(iact > jact) && itrig < jtrig)
                return true;
            return false;
             });

            sPlayerbotAIConfig.log("strategy.csv", "relevance, action, trigger, strategy, classes");

            for (auto& actionkey : actionKeys)
            {
                if (actions.find(actionkey)->second.size() != (MAX_LEVEL - 1) * (MAX_CLASSES - 1))
                {
                    classSpecLevel = actions.find(actionkey)->second;
                    
                    vector<pair<pair<uint32, uint32>,pair<uint32, uint32>>> classs;

                    for (auto cl : classSpecLevel)
                    {
                        uint32 minLevel = MAX_LEVEL; uint32 maxLevel = 0;

                        uint32 cls = cl.first.first;
                        uint32 tb = cl.first.second;

                        if (std::find_if(classs.begin(), classs.end(), [cls,tb](pair<pair<uint32, uint32>, pair<uint32, uint32>> i){return i.first.first ==cls && i.first.second == tb;}) == classs.end())
                        {
                            for (auto cll : classSpecLevel)
                            {
                                if (cll.first.first == cl.first.first && cll.first.second == cl.first.second)
                                {
                                    minLevel = std::min(minLevel, cll.second);
                                    maxLevel = std::max(maxLevel, cll.second);
                                }
                            }

                            classs.push_back(make_pair(cl.first, make_pair(minLevel, maxLevel)));
                        }
                    }

                    out << actionkey;

                    if (classs.size() != 9 * 3)
                    {
                        out << ",";

                        for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
                        {
                            bool a[3] = { false,false,false };
                            uint32 min[3] = { 0,0,0 };
                            uint32 max[3] = { 0,0,0 };

                            if (std::find_if(classs.begin(), classs.end(), [cls](pair<pair<uint32, uint32>, pair<uint32, uint32>> i) {return i.first.first == cls; }) == classs.end())
                                continue;

                            for (uint32 tb = 0; tb < 3; tb++)
                            {
                                auto tcl = std::find_if(classs.begin(), classs.end(), [cls, tb](pair<pair<uint32, uint32>, pair<uint32, uint32>> i) {return i.first.first == cls && i.first.second == tb; });
                                if (tcl == classs.end())
                                    continue;

                                a[tb] = true;
                                min[tb] = tcl->second.first;
                                max[tb] = tcl->second.second;
                            }

                            if (a[0] && a[1] && a[2] && min[0] == min[1] == min[2] && max[0] == max[1] == max[2])
                            {
                                if (min[0] != 1 || max[0] != MAX_LEVEL - 1)
                                    out << classes[cls] << "(" << min[0] << "-" << max[0] << ")";
                                else
                                    out << classes[cls];

                                if (cls != classs.back().first.first)
                                    out << ";";
                            }
                            else
                            {
                                for (uint32 tb = 0; tb < 3; tb++)
                                {
                                    if (!a[tb])
                                        continue;

                                    if (min[tb] != 1 || max[tb] != MAX_LEVEL - 1)
                                        out << specs[cls][tb] << " " << classes[cls] << "(" << min[tb] << "-" << max[tb] << ")";
                                    else
                                        out << specs[cls][tb] << " " << classes[cls];

                                    if (cls != classs.back().first.first || tb != classs.back().first.second)
                                        out << ";";
                                }
                            }
                        }                       
                    }
                    else
                        "all";

                    out << "\n";
                }
                else
                    out << actionkey << "\n";
            }

            sPlayerbotAIConfig.log("strategy.csv", out.str().c_str());
        }
    }
    /*

    sPlayerbotAIConfig.openLog(7, "w");

    //Zone area map REMOVE!
    uint32 k = 0;
    for (auto& node : sTravelNodeMap.getNodes())
    {
        WorldPosition* pos = node->getPosition();
        //map area
        for (uint32 x = 0; x < 2000; x++)
        {
            for (uint32 y = 0; y < 2000; y++)
            {
                if (!pos->getMap())
                    continue;

                float nx = pos->getX() + (x*5)-5000.0f;
                float ny = pos->getY() + (y*5)-5000.0f;
                float nz = pos->getZ() + 100.0f;

                //pos->getMap()->GetHitPosition(nx, ny, nz + 200.0f, nx, ny, nz, -0.5f);

                if (!pos->getMap()->GetHeightInRange(nx, ny, nz, 5000.0f)) // GetHeight can fail
                    continue;

                WorldPosition  npos = WorldPosition(pos->getMapId(), nx, ny, nz, 0.0);
                uint32 area = path.getArea(npos.getMapId(), npos.getX(), npos.getY(), npos.getZ());

                ostringstream out;
                out << std::fixed << area << "," << npos.getDisplayX() << "," << npos.getDisplayY();
                sPlayerbotAIConfig.log(7, out.str().c_str());
            }
        }
        k++;

        if (k > 0)
            break;
    }

    //Explore map output (REMOVE!)

    sPlayerbotAIConfig.openLog(5, "w");
    for (auto i : exploreLocs)
    {
        for (auto j : i.second->getPoints())
        {
            ostringstream out;
            string name = i.second->getTitle();
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());
            out << std::fixed << std::setprecision(2) << name.c_str() << "," << i.first << "," << j->getDisplayX() << "," << j->getDisplayY() << "," << j->getX() << "," << j->getY() << "," << j->getZ();
            sPlayerbotAIConfig.log(5, out.str().c_str());
        }
    }

    */    
}

uint32 TravelMgr::getDialogStatus(Player* pPlayer, int32 questgiver, Quest const* pQuest)
{
    uint32 dialogStatus = DIALOG_STATUS_NONE;

    QuestRelationsMapBounds rbounds;                        // QuestRelations (quest-giver)
    QuestRelationsMapBounds irbounds;                       // InvolvedRelations (quest-finisher)

    uint32 questId = pQuest->GetQuestId();

    if (questgiver > 0)
    {
        rbounds = sObjectMgr.GetCreatureQuestRelationsMapBounds(questgiver);
        irbounds = sObjectMgr.GetCreatureQuestInvolvedRelationsMapBounds(questgiver);
    }
    else
    {
        rbounds = sObjectMgr.GetGOQuestRelationsMapBounds(questgiver * -1);
        irbounds = sObjectMgr.GetGOQuestInvolvedRelationsMapBounds(questgiver * -1);
    }

    // Check markings for quest-finisher
    for (QuestRelationsMap::const_iterator itr = irbounds.first; itr != irbounds.second; ++itr)
    {
        if (itr->second != questId)
            continue;

        uint32 dialogStatusNew = DIALOG_STATUS_NONE;

        if (!pQuest || !pQuest->IsActive())
        {
            continue;
        }

        QuestStatus status = pPlayer->GetQuestStatus(questId);

        if ((status == QUEST_STATUS_COMPLETE && !pPlayer->GetQuestRewardStatus(questId)) ||
            (pQuest->IsAutoComplete() && pPlayer->CanTakeQuest(pQuest, false)))
        {
            if (pQuest->IsAutoComplete() && pQuest->IsRepeatable())
            {
                dialogStatusNew = DIALOG_STATUS_REWARD_REP;
            }
            else
            {
                dialogStatusNew = DIALOG_STATUS_REWARD2;
            }
        }
        else if (status == QUEST_STATUS_INCOMPLETE)
        {
            dialogStatusNew = DIALOG_STATUS_INCOMPLETE;
        }

        if (dialogStatusNew > dialogStatus)
        {
            dialogStatus = dialogStatusNew;
        }
    }

    // check markings for quest-giver
    for (QuestRelationsMap::const_iterator itr = rbounds.first; itr != rbounds.second; ++itr)
    {
        if (itr->second != questId)
            continue;

        uint32 dialogStatusNew = DIALOG_STATUS_NONE;

        if (!pQuest || !pQuest->IsActive())
        {
            continue;
        }

        QuestStatus status = pPlayer->GetQuestStatus(questId);

        if (status == QUEST_STATUS_NONE)                    // For all other cases the mark is handled either at some place else, or with involved-relations already
        {
            if (pPlayer->CanSeeStartQuest(pQuest))
            {
                if (pPlayer->SatisfyQuestLevel(pQuest, false))
                {
                    int32 lowLevelDiff = sWorld.getConfig(CONFIG_INT32_QUEST_LOW_LEVEL_HIDE_DIFF);
                    if (pQuest->IsAutoComplete() || (pQuest->IsRepeatable() && pPlayer->getQuestStatusMap()[questId].m_rewarded))
                    {
                        dialogStatusNew = DIALOG_STATUS_REWARD_REP;
                    }
                    else if (lowLevelDiff < 0 || pPlayer->GetLevel() <= pPlayer->GetQuestLevelForPlayer(pQuest) + uint32(lowLevelDiff))
                    {
                        dialogStatusNew = DIALOG_STATUS_AVAILABLE;
                    }
                    else
                    {
#ifndef MANGOSBOT_TWO
                        dialogStatusNew = DIALOG_STATUS_CHAT;
#else
                        dialogStatusNew = DIALOG_STATUS_LOW_LEVEL_AVAILABLE;
#endif
                    }
                }
                else
                {
                    dialogStatusNew = DIALOG_STATUS_UNAVAILABLE;
                }
            }
        }

        if (dialogStatusNew > dialogStatus)
        {
            dialogStatus = dialogStatusNew;
        }
    }

    return dialogStatus;
}

//Selects a random WorldPosition from a list. Use a distance weighted distribution.
vector<WorldPosition*> TravelMgr::getNextPoint(WorldPosition* center, vector<WorldPosition*> points, uint32 amount) {
    vector<WorldPosition*> retVec;

    if (points.size() < 2)
    {
        retVec.push_back(points[0]);
        return retVec;
    }

    retVec = points;

    vector<uint32> weights;

    std::transform(retVec.begin(), retVec.end(), std::back_inserter(weights), [center](WorldPosition* point) { return 200000 / (1 + point->distance(center)); });

    std::mt19937 gen(time(0));

    weighted_shuffle(retVec.begin(), retVec.end(), weights.begin(), weights.end(), gen);

    return retVec;
}

vector<WorldPosition> TravelMgr::getNextPoint(WorldPosition center, vector<WorldPosition> points, uint32 amount) {
    vector<WorldPosition> retVec;

    if (points.size() < 2)
    {
        if (points.size() == 1)
            retVec.push_back(points[0]);
        return retVec;
    }

    retVec = points;

    
    vector<uint32> weights;

    //List of weights based on distance (Gausian curve that starts at 100 and lower to 1 at 1000 distance)
    //std::transform(retVec.begin(), retVec.end(), std::back_inserter(weights), [center](WorldPosition point) { return 1 + 1000 * exp(-1 * pow(point.distance(center) / 400.0, 2)); });

    //List of weights based on distance (Twice the distance = half the weight). Caps out at 200.0000 range.
    std::transform(retVec.begin(), retVec.end(), std::back_inserter(weights), [center](WorldPosition point) { return 200000/(1+point.distance(center)); });

    std::mt19937 gen(time(0));

    weighted_shuffle(retVec.begin(), retVec.end(), weights.begin(), weights.end(), gen);

    return retVec;
}

QuestStatusData* TravelMgr::getQuestStatus(Player* bot, uint32 questId)
{
    return &bot->getQuestStatusMap()[questId];
}

bool TravelMgr::getObjectiveStatus(Player* bot, Quest const* pQuest, uint32 objective)
{
    uint32 questId = pQuest->GetQuestId();
    if (!bot->IsActiveQuest(questId))
        return false;

    if (bot->GetQuestStatus(questId) != QUEST_STATUS_INCOMPLETE)
        return false;

    QuestStatusData* questStatus = sTravelMgr.getQuestStatus(bot, questId);

    uint32  reqCount = pQuest->ReqItemCount[objective];
    uint32  hasCount = questStatus->m_itemcount[objective];

    if (reqCount && hasCount < reqCount)
        return true;

    reqCount = pQuest->ReqCreatureOrGOCount[objective];
    hasCount = questStatus->m_creatureOrGOcount[objective];

    if (reqCount && hasCount < reqCount)
        return true;

    return false;
}

vector<TravelDestination*> TravelMgr::getQuestTravelDestinations(Player* bot, uint32 questId, bool ignoreFull, bool ignoreInactive, float maxDistance, bool ignoreObjectives)
{
    WorldPosition botLocation(bot);

    vector<TravelDestination*> retTravelLocations;

    if (questId == -1)
    {
        for (auto& dest : questGivers)
        {
            if (!ignoreInactive && !dest->isActive(bot))
                continue;

            if (dest->isFull(ignoreFull))
                continue;

            if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                continue;

            // ignore PvP Halls for now
            for (auto p : dest->getPoints(true))
                if (p->getMapId() == 449 || p->getMapId() == 450)
                    continue;

            retTravelLocations.push_back(dest);
        }
    }
    else
    {
        auto i = quests.find(questId);

        if (i != quests.end())
        {
            for (auto& dest : i->second->questTakers)
            {
                if (!ignoreInactive && !dest->isActive(bot))
                    continue;

                if (dest->isFull(ignoreFull))
                    continue;

                if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                    continue;

                // ignore PvP Halls for now
                for (auto p : dest->getPoints(true))
                    if (p->getMapId() == 449 || p->getMapId() == 450)
                        continue;

                retTravelLocations.push_back(dest);
            }

            if (!ignoreObjectives)
                for (auto& dest : i->second->questObjectives)
                {
                    if (!ignoreInactive && !dest->isActive(bot))
                        continue;

                    if (dest->isFull(ignoreFull))
                        continue;

                    if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                        continue;

                    retTravelLocations.push_back(dest);
                }
        }
    }

    return retTravelLocations;
}

vector<TravelDestination*> TravelMgr::getRpgTravelDestinations(Player* bot, bool ignoreFull, bool ignoreInactive, float maxDistance)
{
    WorldPosition botLocation(bot);

    vector<TravelDestination*> retTravelLocations;

    for (auto& dest : rpgNpcs)
    {
        if (!ignoreInactive && !dest->isActive(bot))
            continue;

        if (dest->isFull(ignoreFull))
            continue;

        if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
            continue;

        // ignore PvP Halls for now
        for (auto p : dest->getPoints(true))
            if (p->getMapId() == 449 || p->getMapId() == 450)
                continue;

        retTravelLocations.push_back(dest);
    }

    return retTravelLocations;
}

vector<TravelDestination*> TravelMgr::getExploreTravelDestinations(Player* bot, bool ignoreFull, bool ignoreInactive)
{
    WorldPosition botLocation(bot);

    vector<TravelDestination*> retTravelLocations;

    for (auto& dest : exploreLocs)
    {
        if (!ignoreInactive && !dest.second->isActive(bot))
            continue;

        if (dest.second->isFull(ignoreFull))
            continue;

        // ignore PvP Halls for now
        for (auto p : dest.second->getPoints(true))
            if (p->getMapId() == 449 || p->getMapId() == 450)
                continue;

        retTravelLocations.push_back(dest.second);
    }

    return retTravelLocations;
}

vector<TravelDestination*> TravelMgr::getGrindTravelDestinations(Player* bot, bool ignoreFull, bool ignoreInactive, float maxDistance)
{
    WorldPosition botLocation(bot);

    vector<TravelDestination*> retTravelLocations;

    for (auto& dest : grindMobs)
    {
        if (!ignoreInactive && !dest->isActive(bot))
            continue;

        if (dest->isFull(ignoreFull))
            continue;

        if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
            continue;

        // ignore PvP Halls for now
        for (auto p : dest->getPoints(true))
            if (p->getMapId() == 449 || p->getMapId() == 450)
                continue;

        retTravelLocations.push_back(dest);
    }

    return retTravelLocations;
}

vector<TravelDestination*> TravelMgr::getBossTravelDestinations(Player* bot, bool ignoreFull, bool ignoreInactive, float maxDistance)
{
    WorldPosition botLocation(bot);

    vector<TravelDestination*> retTravelLocations;

    for (auto& dest : bossMobs)
    {
        if (!ignoreInactive && !dest->isActive(bot))
            continue;

        if (dest->isFull(ignoreFull))
            continue;

        if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
            continue;

        retTravelLocations.push_back(dest);
    }

    return retTravelLocations;
}

void TravelMgr::setNullTravelTarget(Player* player)
{
    if (!player)
        return;

    if (!player->GetPlayerbotAI())
        return;

    TravelTarget* target = player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();

    if (target)
        target->setTarget(sTravelMgr.nullTravelDestination, sTravelMgr.nullWorldPosition, true);
}

void TravelMgr::addMapTransfer(WorldPosition start, WorldPosition end, float portalDistance, bool makeShortcuts)
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return;
    
    //Calculate shortcuts.
    if(makeShortcuts)
        for (auto& mapTransfers : mapTransfersMap)
        {
            uint32 sMapt = mapTransfers.first.first;
            uint32 eMapt = mapTransfers.first.second;

            for (auto& mapTransfer : mapTransfers.second)
            {
                if (eMapt == sMap && sMapt != eMap) // [S1 >MT> E1 -> S2] >THIS> E2
                {
                    float newDistToEnd = mapTransDistance(*mapTransfer.getPointFrom(), start) + portalDistance;
                    if (mapTransDistance(*mapTransfer.getPointFrom(), end) > newDistToEnd)
                        addMapTransfer(*mapTransfer.getPointFrom(), end, newDistToEnd, false);
                }

                if (sMapt == eMap && eMapt != sMap) // S1 >THIS> [E1 -> S2 >MT> E2]
                {
                    float newDistToEnd = portalDistance + mapTransDistance(end, *mapTransfer.getPointTo());
                    if (mapTransDistance(start, *mapTransfer.getPointTo()) > newDistToEnd)
                        addMapTransfer(start, *mapTransfer.getPointTo(), newDistToEnd, false);
                }
            }
        }

    //Add actual transfer.
    auto mapTransfers = mapTransfersMap.find(make_pair(start.getMapId(), end.getMapId()));
    
    if (mapTransfers == mapTransfersMap.end())
        mapTransfersMap.insert({ { sMap, eMap }, {mapTransfer(start, end, portalDistance)} });
    else
        mapTransfers->second.push_back(mapTransfer(start, end, portalDistance));        
};

void TravelMgr::loadMapTransfers()
{
    for (auto& node : sTravelNodeMap.getNodes())
    {
        for (auto& link : *node->getLinks())
        {
            addMapTransfer(*node->getPosition(), *link.first->getPosition(), link.second->getDistance());
        }
    }
}

float TravelMgr::mapTransDistance(WorldPosition start, WorldPosition end)
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return start.distance(end);

    float minDist = 200000;

    auto mapTransfers = mapTransfersMap.find({ sMap, eMap });
    
    if (mapTransfers == mapTransfersMap.end())
        return minDist;

    for (auto& mapTrans : mapTransfers->second)
    {
        float dist = mapTrans.distance(start, end);

        if (dist < minDist)
            minDist = dist;
    }    

    return minDist;
}

float TravelMgr::fastMapTransDistance(WorldPosition start, WorldPosition end)
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return start.fDist(end);

    float minDist = 200000;

    auto mapTransfers = mapTransfersMap.find({ sMap, eMap });

    if (mapTransfers == mapTransfersMap.end())
        return minDist;

    for (auto& mapTrans : mapTransfers->second)
    {
        float dist = mapTrans.fDist(start, end);

        if (dist < minDist)
            minDist = dist;
    }

    return minDist;
}

void TravelMgr::printGrid(uint32 mapId, int x, int y, string type)
{
    string fileName = "unload_grid.csv";

    if (sPlayerbotAIConfig.hasLog(fileName))
    {
        WorldPosition p = WorldPosition(mapId, 0, 0, 0, 0);

        ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr();
        out << "+00, " << 0 << 0 << x << "," << y << ", " << type << ",";
        p.printWKT(p.fromGridPair(GridPair(x, y)), out, 1, true);
        sPlayerbotAIConfig.log(fileName, out.str().c_str());
    }
}

void TravelMgr::printObj(WorldObject* obj, string type)
{
    string fileName = "unload_grid.csv";

    if (sPlayerbotAIConfig.hasLog(fileName))
    {
        WorldPosition p = WorldPosition(obj);

        Cell const& cell = obj->GetCurrentCell();

        vector<WorldPosition> vcell, vgrid;
        vcell = p.fromCellPair(p.getCellPair());
        vgrid = p.gridFromCellPair(p.getCellPair());

        {
            ostringstream out;
            out << sPlayerbotAIConfig.GetTimestampStr();
            out << "+00, " << obj->GetObjectGuid().GetEntry() << "," << obj->GetObjectGuid().GetCounter() << "," << cell.GridX() << "," << cell.GridY() << ", " << type << ",";

            p.printWKT(vcell, out, 1, true);
            sPlayerbotAIConfig.log(fileName, out.str().c_str());
        }

        {
            ostringstream out;
            out << sPlayerbotAIConfig.GetTimestampStr();
            out << "+00, " << obj->GetObjectGuid().GetEntry() << "," << obj->GetObjectGuid().GetCounter() << "," << cell.GridX() << "," << cell.GridY() << ", " << type << ",";

            p.printWKT(vgrid, out, 1, true);
            sPlayerbotAIConfig.log(fileName, out.str().c_str());
        }
    }

    fileName = "unload_obj.csv";

    if (sPlayerbotAIConfig.hasLog(fileName))
    {
        WorldPosition p = WorldPosition(obj);

        Cell const& cell = obj->GetCurrentCell();

        {
            ostringstream out;
            out << sPlayerbotAIConfig.GetTimestampStr();
            out << "+00, " << obj->GetObjectGuid().GetEntry() << "," << obj->GetObjectGuid().GetCounter() << "," << cell.GridX() << "," << cell.GridY() << ", " << type << ",";

            p.printWKT({ p }, out, 0);
            sPlayerbotAIConfig.log(fileName, out.str().c_str());
        }
    }
}