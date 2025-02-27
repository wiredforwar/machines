/*
 * MISSILEM . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogMissileEmplacement

    Handle logical operations for a MissileEmplacement construction
*/

#ifndef _MACHLOG_MISSILEM_HPP
#define _MACHLOG_MISSILEM_HPP

#include "base/base.hpp"

#include "machlog/constron.hpp"
#include "machlog/canattac.hpp"
#include "machlog/canmove.hpp"

// Forward declarations
class MachPhysMissileEmplacement;
class MachLogRace;
class MexPoint3d;
class MachPhysMissileEmplacementData;
class MexTransform3d;
class MexRadians;
// Orthodox canonical (revoked)
class MachLogMissileEmplacement
    : public MachLogConstruction
    , public MachLogCanAttack
    , public MachLogCanTurn
{
public:
    // Construct smelter of designated race and level at location, rotated thru angle about z axis.
    MachLogMissileEmplacement(
        const MachPhys::MissileEmplacementSubType&,
        MachLogRace* pRace,
        uint level,
        const MexPoint3d& location,
        const MexRadians& angle,
        MachPhys::WeaponCombo,
        std::optional<UtlId> withId = std::nullopt);

    ~MachLogMissileEmplacement() override;

    // Inherited from SimActor
    PhysRelativeTime update(const PhysRelativeTime& maxCPUTime, MATHEX_SCALAR clearanceFromDisplayedVolume) override;

    bool canAttack(const MachActor& other) const;
    bool canTurnToAttack(const MachActor& other) const;
    bool canDetect(const MachActor& other) const;

    const MachPhysConstructionData& constructionData() const override;
    const MachPhysMissileEmplacementData& data() const;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachLogMissileEmplacement& t);

    const MachPhys::MissileEmplacementSubType& subType() const;

    //  bool findTargetClosestTo( const MachLogMissileEmplacement& actor, MachActor * & pResult ) const;

    PhysRelativeTime attack(MachActor* pTarget) override;
    virtual PhysRelativeTime attack(const MexPoint3d& position);

    // Turn through angle radians about the z axis.
    // Motion is relative, not absolute.
    // Returns time it will take to complete.
    PhysRelativeTime turn(const MexRadians& angle) override;
    PhysRelativeTime turnWithoutEcho(const MexRadians& angle);

    const MachPhysMissileEmplacement* pPhysMissileEmplacement() const;

    // Special method to determine whether location on map would be in weapon range.
    bool inWeaponRange(const MexPoint3d& targetPosition) const;

    bool inAngleRange(const MexPoint3d& targetPosition) const;
    MexRadians angleToTurnToFace(const MexPoint3d& pos);

    bool domeOpen() const;

    const MexTransform3d& globalAspectTransform() const;

    // inherited from MachActor
    int militaryStrength() const override;
    // inherited from MachActor
    int localStrength() const override;

    bool isNukeSilo() const;
    bool nukeReady() const;
    // PRE( isNukeSilo() );

    MATHEX_SCALAR scannerRange() const;

    // Amount of damage is passed in beHit()
    void beHit(
        const int&,
        MachPhys::WeaponType byType = MachPhys::N_WEAPON_TYPES,
        MachActor* pByActor = nullptr,
        MexLine3d* pByDirection = nullptr,
        MachActor::EchoBeHit = MachActor::ECHO) override;

    void beHitWithoutAnimation(
        int damage,
        PhysRelativeTime physicalTimeDelay = 0,
        MachActor* pByActor = nullptr,
        EchoBeHit = ECHO) override;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogMissileEmplacement);
    PER_FRIEND_READ_WRITE(MachLogMissileEmplacement);

private:
    // Operations deliberately revoked
    MachLogMissileEmplacement(const MachLogMissileEmplacement&);
    MachLogMissileEmplacement& operator=(const MachLogMissileEmplacement&);
    bool operator==(const MachLogMissileEmplacement&);

    // Construct a new physical smelter. Used in ctor initializer list
    static MachPhysMissileEmplacement* pNewPhysMissileEmplacement(
        const MachPhys::MissileEmplacementSubType&,
        MachLogRace* pRace,
        uint level,
        const MexPoint3d& location,
        const MexRadians& angle,
        MachPhys::WeaponCombo);

    // The physical missile emplacement
    MachPhysMissileEmplacement* pPhysMissileEmplacement();

    bool inAngleRange(const W4dEntity& entity) const;
    bool inScanningRange(const MachActor& other) const;
    static MexRadians angleRange();
    static MATHEX_SCALAR sinAngleRange();

    // Data members
    MachPhys::MissileEmplacementSubType subType_;

    bool domeOpen_{}; // only used by some types of emplacement
    bool inTransition_{}; // only used by some types of emplacement
                        // means dome is neither open nor closed.
    // used in lazy evaluation of local strength
    PhysAbsoluteTime lastStrengthEstimateTime_;
};

PER_DECLARE_PERSISTENT(MachLogMissileEmplacement);

#ifdef _INLINE
#include "machlog/missilem.ipp"
#endif

#endif

/* End MISSILEM.HPP **************************************************/
