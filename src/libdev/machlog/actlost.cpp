/*
 * A C T L O S T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "stdlib/string.hpp"
#include "utility/linetok.hpp"
#include "mathex/point2d.hpp"
#include "sim/manager.hpp"
#include "machlog/actlost.hpp"
#include "machlog/scenario.hpp"
#include "machlog/races.hpp"

PER_DEFINE_PERSISTENT(MachLogLostAction);

MachLogLostAction::MachLogLostAction(SimCondition* pCondition, bool enabled)
    : SimAction(pCondition, enabled)
{
    TEST_INVARIANT;
}

// virtual
MachLogLostAction::~MachLogLostAction()
{
    TEST_INVARIANT;
}

void MachLogLostAction::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogLostAction& t)
{

    o << "MachLogLostAction " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogLostAction " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachLogLostAction::doAction()
{
    MachLogRaces::instance().hasLost(race_, true);
}

// static
std::unique_ptr<MachLogLostAction>
MachLogLostAction::newFromParser(SimCondition* pCondition, bool enabled, UtlLineTokeniser* pParser)
{
    std::unique_ptr<MachLogLostAction> pResult(new MachLogLostAction(pCondition, enabled));
    for (std::size_t i = 0; i < pParser->tokens().size(); ++i)
    {
        const std::string& token = pParser->tokens()[i];
        if (token == "RACE")
            pResult->race_ = MachLogScenario::machPhysRace(pParser->tokens()[i + 1]);
    }
    return pResult;
}

// virtual
void MachLogLostAction::doOutputOperator(std::ostream& o) const
{
    SimAction::doOutputOperator(o);
    o << "Race " << race_ << std::endl;
}

void perWrite(PerOstream& ostr, const MachLogLostAction& action)
{
    const SimAction& base1 = action;

    ostr << base1;
    ostr << action.race_;
}

void perRead(PerIstream& istr, MachLogLostAction& action)
{
    SimAction& base1 = action;

    istr >> base1;
    istr >> action.race_;
}

MachLogLostAction::MachLogLostAction(PerConstructor con)
    : SimAction(con)
{
}

std::unique_ptr<MachLogLostAction> MachLogLostAction::newDynamic(SimCondition* pCondition, bool enabled, MachPhys::Race race)
{
    std::unique_ptr<MachLogLostAction> pResult(new MachLogLostAction(pCondition, enabled));
    pResult->race_ = race;
    return pResult;
}

/* End ACTREINF.CPP *************************************************/
