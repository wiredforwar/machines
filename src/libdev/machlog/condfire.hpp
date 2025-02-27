/*
 * C O N D F I R E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogFiredAtCondition

    This conditon code checks for one race having fired at another.

*/

#ifndef _MACHLOG_CONDFIRE_HPP
#define _MACHLOG_CONDFIRE_HPP

#include "base/base.hpp"
#include "phys/phys.hpp"
#include "sim/conditio.hpp"
#include "machlog/machlog.hpp"

class UtlLineTokeniser;

class MachLogFiredAtCondition : public SimCondition
// Canonical form revoked
{
public:
    static MachLogFiredAtCondition* newFromParser(UtlLineTokeniser*);

    bool doHasConditionBeenMet() const override;

    ~MachLogFiredAtCondition() override;

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogFiredAtCondition);
    PER_FRIEND_READ_WRITE(MachLogFiredAtCondition);

protected:
    const PhysRelativeTime& recommendedCallBackTimeGap() const override;
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogFiredAtCondition(const std::string& keyName, MachPhys::Race firingRace, MachPhys::Race firedAtRace);

    friend std::ostream& operator<<(std::ostream& o, const MachLogFiredAtCondition& t);

    MachLogFiredAtCondition(const MachLogFiredAtCondition&);
    MachLogFiredAtCondition& operator=(const MachLogFiredAtCondition&);

    MachPhys::Race firingRace_;
    MachPhys::Race firedAtRace_;
};

PER_DECLARE_PERSISTENT(MachLogFiredAtCondition);

#endif

/* End CONDTIME.HPP *************************************************/
