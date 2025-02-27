/*
 * C O N D R E S E . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "stdlib/string.hpp"
#include "mathex/point2d.hpp"
#include "sim/manager.hpp"
#include "utility/linetok.hpp"
#include "machlog/condrese.hpp"
#include "machlog/races.hpp"
#include "machlog/restree.hpp"
#include "machlog/resitem.hpp"
#include "machlog/mlnotif.hpp"
#include "machlog/scenario.hpp"

class MachLogResearchCompleteConditionNotifiable : public MachLogNotifiable
{
private:
    MachLogResearchCompleteCondition* pOwner_;

public:
    MachLogResearchCompleteConditionNotifiable(MachPhys::Race race, MachLogResearchCompleteCondition* pOwner)
        : MachLogNotifiable(race)
        , pOwner_(pOwner)
    {
    }
    ~MachLogResearchCompleteConditionNotifiable() override { }
    void notifiableBeNotified() override
    {
        if (pOwner_->pItem_->isResearched(pOwner_->race_))
            pOwner_->callBackTimeGap_ = 0;
    }
};

PER_DEFINE_PERSISTENT(MachLogResearchCompleteCondition);

MachLogResearchCompleteCondition::MachLogResearchCompleteCondition(
    const std::string& keyName,
    MachPhys::Race race,
    MachLog::ObjectType ot,
    int hwLevel,
    int subType,
    MachPhys::WeaponCombo wc)
    : SimCondition(keyName)
    , objectType_(ot)
    , subType_(subType)
    , hwLevel_(hwLevel)
    , swLevel_(0)
    , weaponCombo_(wc)
    , race_(race)
    , callBackTimeGap_(1)
    , pNotifiable_(nullptr)
{
    pItem_ = &MachLogRaces::instance().researchTree().researchItem(ot, subType, hwLevel, wc);
    if (pItem_->isResearched(race_))
        callBackTimeGap_ = 0;
    else
    {
        pNotifiable_ = new MachLogResearchCompleteConditionNotifiable(race, this);
        MachLogRaces::instance().researchTree().addMe(pNotifiable_);
    }
    TEST_INVARIANT;
}

MachLogResearchCompleteCondition::~MachLogResearchCompleteCondition()
{
    TEST_INVARIANT;
    if (pNotifiable_)
    {
        MachLogRaces::instance().researchTree().removeMe(pNotifiable_);
        delete pNotifiable_;
    }
}

// virtual
bool MachLogResearchCompleteCondition::doHasConditionBeenMet() const
{
    HAL_STREAM(
        "MachLogResearchCompleteCondition::doHasConditionBeenMet checking condition of research item for researched\n");
    HAL_STREAM(" researched? " << pItem_->isResearched(race_) << std::endl);
    doOutputOperator(Diag::instance().halStream());
    return pItem_->isResearched(race_);
}

// static
MachLogResearchCompleteCondition* MachLogResearchCompleteCondition::newFromParser(UtlLineTokeniser* pParser)
{
    // format of a RESEARCH_COMPLETE condition is:
    //  <keyname> RACE <race> <objectType> <hwLevel> [<subType>] [<weaponCombo>]
    int subType = 0;
    MachPhys::WeaponCombo wc = MachPhys::N_WEAPON_COMBOS;
    MachLog::ObjectType ot = MachLogScenario::objectType(pParser->tokens()[4]);
    size_t nTokens = pParser->tokens().size();
    if (nTokens >= 7)
        subType = MachLogScenario::objectSubType(ot, pParser->tokens()[6]);
    if (nTokens >= 8)
        wc = MachLogScenario::weaponCombo(pParser->tokens()[7]);

    return new MachLogResearchCompleteCondition(
        pParser->tokens()[1], // keyName
        MachLogScenario::machPhysRace(pParser->tokens()[3]),
        ot,
        atol(pParser->tokens()[5].c_str()),
        subType,
        wc);
}

void MachLogResearchCompleteCondition::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogResearchCompleteCondition& t)
{

    t.doOutputOperator(o);
    return o;
}

// virtual
const PhysRelativeTime& MachLogResearchCompleteCondition::recommendedCallBackTimeGap() const
{
    return callBackTimeGap_;
}

// virtual
void MachLogResearchCompleteCondition::doOutputOperator(std::ostream& o) const
{
    SimCondition::doOutputOperator(o);
    o << "MachLogResearchCompleteCondition " << static_cast<const void*>(this) << " start" << std::endl;
    o << objectType_ << " sub " << subType_ << " hw " << hwLevel_ << " wc " << weaponCombo_ << " " << race_
      << std::endl;
    o << "callBackTimeGap_ " << callBackTimeGap_ << std::endl;
    o << "researchComplete.pNotifiable_ " << (void*)pNotifiable_ << std::endl;
    o << "pItem_ " << (void*)pItem_ << std::endl;
    o << "direct address: "
      << (void*)&MachLogRaces::instance().researchTree().researchItem(objectType_, subType_, hwLevel_, weaponCombo_)
      << std::endl;
}

void perWrite(PerOstream& ostr, const MachLogResearchCompleteCondition& researchComplete)
{
    const SimCondition& base1 = researchComplete;

    ostr << base1;
    ostr << researchComplete.objectType_;
    ostr << researchComplete.subType_;
    ostr << researchComplete.hwLevel_;
    ostr << researchComplete.swLevel_;
    ostr << researchComplete.weaponCombo_;
    ostr << researchComplete.race_;
    ostr << researchComplete.callBackTimeGap_;
    bool hasNotifiable = (researchComplete.pNotifiable_ != nullptr);
    PER_WRITE_RAW_OBJECT(ostr, hasNotifiable);
}

void perRead(PerIstream& istr, MachLogResearchCompleteCondition& researchComplete)
{
    HAL_STREAM("perRead MachLogResearchCompleteCondition\n");
    HAL_INDENT(2);
    SimCondition& base1 = researchComplete;

    istr >> base1;
    istr >> researchComplete.objectType_;
    istr >> researchComplete.subType_;
    istr >> researchComplete.hwLevel_;
    istr >> researchComplete.swLevel_;
    istr >> researchComplete.weaponCombo_;
    istr >> researchComplete.race_;
    istr >> researchComplete.callBackTimeGap_;
    bool hasNotifiable;
    PER_READ_RAW_OBJECT(istr, hasNotifiable);
    if (hasNotifiable)
    {
        researchComplete.pNotifiable_
            = new MachLogResearchCompleteConditionNotifiable(researchComplete.race_, &researchComplete);
        MachLogRaces::instance().researchTree().addMe(researchComplete.pNotifiable_);
    }
    else
        researchComplete.pNotifiable_ = nullptr;
    researchComplete.pItem_ = &MachLogRaces::instance().researchTree().researchItem(
        researchComplete.objectType_,
        researchComplete.subType_,
        researchComplete.hwLevel_,
        researchComplete.weaponCombo_);
    researchComplete.doOutputOperator(Diag::instance().halStream());
    HAL_INDENT(-2);
    HAL_STREAM("perRead MachLogResearchCompleteCondition DONE\n");
}

MachLogResearchCompleteCondition::MachLogResearchCompleteCondition(PerConstructor con)
    : SimCondition(con)
{
}

/* End CONDTIME.CPP *************************************************/
