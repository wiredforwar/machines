/*
 * C O N D V O R T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogVortexHasBeenFiredCondition

    This conditon code checks for the BMUs being at a certain level it can cope with greater than or less than
    checks.

*/

#ifndef _MACHLOG_CONDVORT_HPP
#define _MACHLOG_CONDVORT_HPP

#include "base/base.hpp"
#include "phys/phys.hpp"
#include "sim/conditio.hpp"
#include "machlog/machlog.hpp"

class UtlLineTokeniser;

class MachLogVortexHasBeenFiredCondition : public SimCondition
// Canonical form revoked
{
public:
    static MachLogVortexHasBeenFiredCondition* newFromParser(UtlLineTokeniser*);

    bool doHasConditionBeenMet() const override;

    ~MachLogVortexHasBeenFiredCondition() override;

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogVortexHasBeenFiredCondition);
    PER_FRIEND_READ_WRITE(MachLogVortexHasBeenFiredCondition);

protected:
    const PhysRelativeTime& recommendedCallBackTimeGap() const override;
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogVortexHasBeenFiredCondition(const std::string& keyName, MachPhys::Race);

    friend std::ostream& operator<<(std::ostream& o, const MachLogVortexHasBeenFiredCondition& t);

    MachLogVortexHasBeenFiredCondition(const MachLogVortexHasBeenFiredCondition&);
    MachLogVortexHasBeenFiredCondition& operator=(const MachLogVortexHasBeenFiredCondition&);

    MachPhys::Race race_;
};

PER_DECLARE_PERSISTENT(MachLogVortexHasBeenFiredCondition);

#endif

/* End CONDTIME.HPP *************************************************/
