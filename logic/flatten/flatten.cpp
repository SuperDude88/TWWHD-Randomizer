#include <logic/flatten/flatten.hpp>
#include <logic/World.hpp>



FlattenSearch::FlattenSearch(World* world_) {
    world = world_;

    for (auto& [name, area] : world->areaTable)
    {
        for (auto& exit : area->exits)
        {
            auto visit = visitor(&exit, this);
            visitReq(exit.getRequirement(), visit);
        }

        for (auto& event : area->events)
        {
            auto visit = visitor(&event, this);
            visitReq(event.requirement, visit);
        }
    }

    auto root = world->getArea("Root");
    areaExprs[root] = DNF::True();
    newlyUpdatedAreas.insert(root);
    newThingsFound = true;

    for (auto& exit : root->exits)
    {
        if (exit.getConnectedArea() != nullptr)
        {
            exitsToTry.insert(&exit);
        }
    }
}

void FlattenSearch::doSearch()
{
    // This algorithm works in three stages:
    // 1. Compute area and event requirements -> DNFs
    // 2. Compute location requirement -> DNF
    // 3. Simplify location requirement -> Requirement

    // This is step 1. This computes everything that requirements
    // can depend on in a fixpoint algorithm - namely, area access and events.
    newThingsFound = true;
    while (newThingsFound)
    {
        recentlyUpdatedAreas = newlyUpdatedAreas;
        recentlyUpdatedEvents = newlyUpdatedEvents;
        newlyUpdatedAreas = {};
        newlyUpdatedEvents = {};
        newThingsFound = false;
        tryExits();
        tryEvents();
    }

    std::unordered_map<std::string, std::list<LocationAccess*>> itemLocations = {};
    for (auto& [name, area] : world->areaTable)
    {
        for (auto& locAccess : area->locations)
        {
            auto locationName = locAccess.location->getName();
            if (!itemLocations.contains(locationName))
            {
                itemLocations[locationName] = {};
            }
            itemLocations[locationName].push_back(&locAccess);
        }
    }
    // TODO this immediately combines the "local" requirements with the implicit
    // area requirement. It has been hypothesized that converting them
    // separately may produce better tooltips, but at that point you need the
    // TWWR-Tracker boolean-expression multi-level simplification code

    // Step 2: for every location, OR all the ways to access it
    for (auto& [locName, accessList] : itemLocations)
    {
        auto expr = DNF::False();
        for (auto& locAcc : accessList)
        {
            expr = expr.or_(areaExprs[locAcc->area].and_(evaluatePartialRequirement(bitIndex, locAcc->requirement, this)));
        }

        // Step 3: simplify
        world->locationTable[locName]->computedRequirement = DNFToExpr(bitIndex, expr.dedup());
    }
}

// Check for a thing in area whether its logical dependencies
// have recently been updated.
bool FlattenSearch::wasUpdated(Area* area, void* thing)
{
    if (recentlyUpdatedAreas.contains(area))
    {
        return true;
    }
    auto& remoteEventReqs = remoteEventRequirements[thing];
    for (auto& event : remoteEventReqs)
    {
        if (recentlyUpdatedEvents.contains(event))
        {
            return true;
        }
    }
    auto& remoteAreaReqs = remoteAreaRequirements[thing];
    for (auto& area : remoteAreaReqs)
    {
        if (recentlyUpdatedAreas.contains(world->getArea(area)))
        {
            return true;
        }
    }

    return false;
}

void FlattenSearch::tryExits()
{
    auto exits = exitsToTry;
    for (auto& exit : exits)
    {
        if (!wasUpdated(exit->getParentArea(), (void*) exit))
        {
            continue;
        }

        auto connectedArea = exit->getConnectedArea();
        auto& oldExpr = areaExprs[connectedArea];
        auto newPartial = areaExprs[exit->getParentArea()].and_(evaluatePartialRequirement(bitIndex, exit->getRequirement(), this));
        auto [useful, newExpr] = oldExpr.or_useful(newPartial);
        if (useful)
        {
            newlyUpdatedAreas.insert(connectedArea);
            newThingsFound = true;
            areaExprs[connectedArea] = newExpr.dedup();
            for (auto& event : connectedArea->events)
            {
                eventsToTry.insert(&event);
            }
            for (auto& areaExit : connectedArea->exits)
            {
                if (areaExit.getConnectedArea() != nullptr)
                {
                    exitsToTry.insert(&areaExit);
                }
            }
            areasToTry.insert(connectedArea);
        }
    }
}

void FlattenSearch::tryEvents()
{
    for (auto& event : eventsToTry)
    {
        if (!wasUpdated(event->area, (void*) event))
        {
            continue;
        }

        auto& oldExpr = eventExprs[event->event];
        auto newPartial = areaExprs[event->area].and_(evaluatePartialRequirement(bitIndex, event->requirement, this));
        auto [useful, newExpr] = oldExpr.or_useful(newPartial);
        if (useful)
        {
            newlyUpdatedEvents.insert(event->event);
            newThingsFound = true;
            eventExprs[event->event] = newExpr.dedup();
        }
    }
}

DNF evaluatePartialRequirement(BitIndex& bitIndex, const Requirement& req, FlattenSearch* search)
{
    uint32_t expectedCount = 0;
    uint32_t expectedHearts = 0;
    uint32_t totalHearts = 0;
    std::bitset<512> bits = 0;
    Item item;
    EventId event;
    DNF d = DNF();
    Area* area;
    Requirement macro;
    
    switch(req.type)
    {
    case RequirementType::NOTHING:
        return DNF::True();

    case RequirementType::IMPOSSIBLE:
        return DNF::False();

    case RequirementType::OR:
        d = DNF::False();
        for (auto& arg : req.args)
        {
            d = d.or_(evaluatePartialRequirement(bitIndex, std::get<Requirement>(arg), search));
        }
        return d;

    case RequirementType::AND:
        d = DNF::True();
        for (auto& arg : req.args)
        {
            d = d.and_(evaluatePartialRequirement(bitIndex, std::get<Requirement>(arg), search));
        }
        return d;

    case RequirementType::HAS_ITEM:
        [[fallthrough]];
    case RequirementType::HEALTH:
        bits[bitIndex.reqBit(req)] = 1;
        return DNF({bits});

    case RequirementType::EVENT:
        event = std::get<EventId>(req.args[0]);
        return search->eventExprs[event];

    // count requirements frequently have to unify with weaker terms,
    // so a count requirement always requires all lesser item counts too.
    // this ensures redundant terms can be eliminated
    case RequirementType::COUNT:
        expectedCount = std::get<int>(req.args[0]);
        item = std::get<Item>(req.args[1]);
        for (auto i = 1; i <= expectedCount; i++)
        {
            Requirement newReq;
            if (i == 1)
            {
                newReq = Requirement{RequirementType::HAS_ITEM, {item}};
            }
            else
            {
                newReq = Requirement{RequirementType::COUNT, {i, item}};
            }
            bits[bitIndex.reqBit(newReq)] = 1;
        }
        return DNF({bits});

    case RequirementType::CAN_ACCESS:
        area = search->world->getArea(std::get<std::string>(req.args[0]));
        return search->areaExprs[area];

    case RequirementType::NONE:
    default:
        // actually needs to be some error state?
        return DNF::False();
    }
    return DNF::False();
}