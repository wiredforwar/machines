/*
 * A C T L O S T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogLostAction

    This action class sets the race to having lost
*/

#ifndef _MACHLOG_ACTLost_HPP
#define _MACHLOG_ACTLost_HPP

#include "base/base.hpp"
#include "sim/action.hpp"
#include "machphys/machphys.hpp"

#include <memory>

class UtlLineTokeniser;
class SimCondition;

class MachLogLostAction : public SimAction
// Canonical form revoked
{
public:
    ~MachLogLostAction() override;
    static std::unique_ptr<MachLogLostAction> newFromParser(SimCondition*, bool enabled, UtlLineTokeniser*);
    static std::unique_ptr<MachLogLostAction> newDynamic(SimCondition*, bool enabled, MachPhys::Race);

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogLostAction);
    PER_FRIEND_READ_WRITE(MachLogLostAction);

protected:
    void doAction() override;
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogLostAction(SimCondition*, bool enabled);
    friend std::ostream& operator<<(std::ostream& o, const MachLogLostAction& t);

    MachLogLostAction(const MachLogLostAction&);
    MachLogLostAction& operator=(const MachLogLostAction&);

    MachPhys::Race race_;
};

PER_DECLARE_PERSISTENT(MachLogLostAction);

#endif

/* End ACTLost.HPP *************************************************/
