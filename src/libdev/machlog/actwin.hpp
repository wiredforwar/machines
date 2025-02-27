/*
 * A C T W I N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogWinAction

    This action class can play a verbal email
*/

#ifndef _MACHLOG_ACTWIN_HPP
#define _MACHLOG_ACTWIN_HPP

#include "base/base.hpp"
#include "sim/action.hpp"
#include "machphys/machphys.hpp"

#include <memory>

class UtlLineTokeniser;
class SimCondition;

class MachLogWinAction : public SimAction
// Canonical form revoked
{
public:
    ~MachLogWinAction() override;
    static std::unique_ptr<MachLogWinAction> newFromParser(SimCondition*, bool enabled, UtlLineTokeniser*);
    static std::unique_ptr<MachLogWinAction> newDynamic(SimCondition*, bool enabled, MachPhys::Race race);

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachLogWinAction);
    PER_FRIEND_READ_WRITE(MachLogWinAction);

protected:
    void doAction() override;
    void doOutputOperator(std::ostream&) const override;

private:
    MachLogWinAction(SimCondition*, bool enabled);
    friend std::ostream& operator<<(std::ostream& o, const MachLogWinAction& t);

    MachLogWinAction(const MachLogWinAction&);
    MachLogWinAction& operator=(const MachLogWinAction&);

    MachPhys::Race race_;
};

PER_DECLARE_PERSISTENT(MachLogWinAction);

#endif

/* End ACTWIn.HPP *************************************************/
