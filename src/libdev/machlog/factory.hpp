/*
 * FACTORY . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogFactory

    Handle logical operations for a Factory construction
*/

#ifndef _MACHLOG_FACTORY_HPP
#define _MACHLOG_FACTORY_HPP

#include "base/base.hpp"
#include "machlog/constron.hpp"
#include "machlog/restree.hpp"

// Forward declarations
class MachPhysFactory;
class MachLogRace;
class MexPoint2d;
class MexPoint3d;
class MexRadians;
class MachLogProductionUnit;
class MachLogFactoryImpl;
class MachLogMachine;
class MachPhysConstructionData;
class MachPhysObjectData;
class MachPhysFactoryData;

template <class X> class ctl_list;

// Orthodox canonical (revoked)
class MachLogFactory : public MachLogConstruction
{
public:
    enum BuiltOrBlocking
    {
        NEWLY_BUILT_MACHINE,
        BLOCKING_MACHINE
    };

    using ProductionLine = ctl_list<MachLogProductionUnit*>;
    // Construct factory of designated race and level at location, rotated thru angle about z axis.
    MachLogFactory(
        MachPhys::FactorySubType subType,
        MachLogRace* pRace,
        uint level,
        const MexPoint3d& location,
        const MexRadians& angle,
        std::optional<UtlId> withId = std::nullopt);

    ~MachLogFactory() override;

    // Inherited from SimActor
    PhysRelativeTime update(const PhysRelativeTime& maxCPUTime, MATHEX_SCALAR clearanceFromDisplayedVolume) override;

    const MachPhysConstructionData& constructionData() const override;
    const MachPhysFactoryData& data() const;

    // export production line for viewing contents.
    const ProductionLine& productionLine() const;
    // Add a machine to the production line
    // returns whether that factory is allowed to build that machine
    bool buildMachine(
        const MachLog::ObjectType type,
        const int subType,
        const MachLogMachine::Level hwLevel,
        const MachLogMachine::Level swLevel,
        int priority,
        MachPhys::WeaponCombo wc = MachPhys::N_WEAPON_COMBOS);
    // Add a machine to the production line. If that type of machine is already there then it will not be built.
    // returns whether that factory is allowed to build that machine. Mainly for use with AI races
    bool buildMachineUnique(
        const MachLog::ObjectType type,
        const int subType,
        const MachLogMachine::Level hwLevel,
        const MachLogMachine::Level swLevel,
        int priority,
        MachPhys::WeaponCombo wc = MachPhys::N_WEAPON_COMBOS);
    // Add a machine to the production line. If that type of machine is already there then it will not be built.
    // returns whether that factory is allowed to build that machine. Mainly for use with AI races
    bool buildMachineUniqueWithSquadIndex(
        const MachLog::ObjectType type,
        const int subType,
        const MachLogMachine::Level hwLevel,
        const MachLogMachine::Level swLevel,
        int priority,
        int squadIndex,
        MachPhys::WeaponCombo wc = MachPhys::N_WEAPON_COMBOS);
    // Am I currently building anything. The answer is returned via the pointer to productionUnit
    bool currentlyBuilding(MachLogProductionUnit* pReturn = nullptr) const;

    // manipulation functions
    // destroy everything remaining on the production line
    void cancelProductionUnit(const MachLogProductionUnit* pUnit);
    enum MoveDirection
    {
        TOWARDS_FRONT,
        TOWARDS_BACK
    };
    void moveProductionUnit(MachLogProductionUnit*, MoveDirection);
    // use this method to remove the currently being produced machine.
    void cancelAllProduction();

    // how much has been built so far.
    MachPhys::BuildingMaterialUnits amountBuilt() const;
    MachPhys::BuildingMaterialUnits amountBuilt();

    bool isAllowedToBuild(MachLog::ObjectType, int subType, int hwLevel, MachPhys::WeaponCombo wc) const;

    const MachLogResearchTree::ResearchItems& buildItems() const;

    MATHEX_SCALAR percentComplete() const;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachLogFactory& t);

    const MachPhys::FactorySubType& subType() const;

    static void loadGame();
    static void unloadGame();

    void specifiedDeployPoint(const MexPoint2d& dest);
    const MexPoint2d& specifiedDeployPoint();

    void assignToDifferentRace(MachLogRace& newRace) override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogFactory);
    PER_FRIEND_READ_WRITE(MachLogFactory);

protected:
    void doOutputOperator(std::ostream&) const override;

    typedef MachLogResearchTree::ResearchItems ResearchItemsArray[5][5];

    static ResearchItemsArray& researchItems();

private:
    void insertIntoProductionLine(MachLogProductionUnit* newUnit);

    void clearProductionLine();

    // Operations deliberately revoked
    MachLogFactory(const MachLogFactory&);
    MachLogFactory& operator=(const MachLogFactory&);
    bool operator==(const MachLogFactory&);

    // Construct a new physical factory. Used in ctor initializer list
    static MachPhysFactory* pNewPhysFactory(
        MachPhys::FactorySubType subType,
        MachLogRace* pRace,
        uint level,
        const MexPoint3d& location,
        const MexRadians& angle);

    void createNewMachine(
        const MachLogProductionUnit& newMachine,
        const MexPoint2d& pos1,
        const MexPoint2d& pos2,
        MachLogRace* pRace);
    void moveToDeployPoint(
        MachLogMachine* pMach,
        const MexPoint2d& entranceInternalPoint,
        const MexPoint2d& entranceExternalPoint,
        BuiltOrBlocking builtOrBlocking = NEWLY_BUILT_MACHINE);

    MachPhys::BuildingMaterialUnits currentTotalCost() const;

    void amountBuilt(const MachPhys::BuildingMaterialUnits&);

    static const MachLogResearchTree::ResearchItems& buildItems(MachPhys::FactorySubType, int level);
    // reverse order search of the production line. If the machine is currently being produced
    // Then it will not be stopped
    void removeMachine(const MachLog::ObjectType);
    // The physical factory
    MachPhysFactory* pPhysFactory();
    const MachPhysFactory* pPhysFactory() const;

    MachLogFactoryImpl* pImpl_;
};

PER_DECLARE_PERSISTENT(MachLogFactory);

#ifdef _INLINE
#include "machlog/factory.ipp"
#endif

PER_ENUM_PERSISTENT(MachLogFactory::MoveDirection);
#endif

/* End FACTORY.HPP **************************************************/
