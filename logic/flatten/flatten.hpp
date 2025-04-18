#pragma once

#include <logic/Entrance.hpp>
#include <logic/Area.hpp>
#include <logic/World.hpp>
#include <logic/flatten/simplify_algebraic.hpp>

#include <functional>

class FlattenSearch
{
public:

    FlattenSearch() = default;
    FlattenSearch(World* world_);

    World* world = nullptr;
    BitIndex bitIndex = BitIndex();

    // partially computed requirements for areas at a
    // given time of day and for events
    std::unordered_map<EventId, DNF> eventExprs = {};
    std::map<Area*, DNF> areaExprs = {};

    // nodes we haven't looked at we don't even need to bother with
    std::set<Entrance*> exitsToTry = {};
    std::set<EventAccess*> eventsToTry = {};
    std::set<Area*> areasToTry = {};

    // we only re-check an exit or an event if its dependencies changed.
    // dependencies can be the implicit parent area (for events and exits),
    // the opposite-TOD area (for sleeping), and "remote" requirements arising
    // from the expression itself mentioning an event or an area via can_access
    std::set<Area*> recentlyUpdatedAreas = {};
    std::set<EventId> recentlyUpdatedEvents = {};

    std::set<Area*> newlyUpdatedAreas = {};
    std::set<EventId> newlyUpdatedEvents = {};

    std::unordered_map<void*, std::set<EventId>> remoteEventRequirements = {};
    std::unordered_map<void*, std::set<std::string>> remoteAreaRequirements = {};
    bool newThingsFound = false;

    void doSearch();
    bool wasUpdated(Area* area, void* thing);
    void tryExits();
    void tryEvents();
};

template<typename T>
std::function<void(const Requirement& req)> visitor(T* thing, FlattenSearch* search)
{
    auto thingPtr = (void*) thing;
    std::function<void(const Requirement&)> handler = [=](const Requirement& req){
        if (req.type == RequirementType::EVENT)
        {
            if (!search->remoteEventRequirements.contains(thingPtr))
            {
                search->remoteEventRequirements[thingPtr] = {};
            }
            search->remoteEventRequirements[thingPtr].insert(std::get<EventId>(req.args[0]));
        }
        else if (req.type == RequirementType::CAN_ACCESS)
        {
            if (search->remoteAreaRequirements.contains(thingPtr))
            {
                search->remoteAreaRequirements[thingPtr] = {};
            }
            search->remoteAreaRequirements[thingPtr].insert(std::get<std::string>(req.args[0]));
        }
    };

    return handler;
}

template<typename Func>
void visitReq(const Requirement& req, Func f, World* world)
{
    f(req);
    if (req.type == RequirementType::AND or req.type == RequirementType::OR)
    {
        for (auto& arg : req.args)
        {
            visitReq(std::get<Requirement>(arg), f, world);
        }
    }
    else if (req.type == RequirementType::MACRO)
    {
        visitReq(world->macros[std::get<MacroIndex>(req.args[0])], f, world);
    }
}

DNF evaluatePartialRequirement(BitIndex& bitIndex, const Requirement& req, FlattenSearch* search);