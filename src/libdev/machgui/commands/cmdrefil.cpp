/*
 * C M D R E F I L L  . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "cmdrefil.hpp"

#include "mathex/point3d.hpp"
#include "machgui/ingame.hpp"
#include "machgui/internal/strings.hpp"
#include "device/butevent.hpp"
#include "machlog/actor.hpp"
#include "machlog/machvman.hpp"
#include "machlog/oprefill.hpp"
#include "machlog/administ.hpp"

MachGuiRefillLandMineCommand::MachGuiRefillLandMineCommand(MachInGameScreen* pInGameScreen)
    : MachGuiCommand(pInGameScreen)
    , hadFinalPick_(true)
{
    TEST_INVARIANT;
}

MachGuiRefillLandMineCommand::~MachGuiRefillLandMineCommand()
{
    TEST_INVARIANT;
}

void MachGuiRefillLandMineCommand::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiRefillLandMineCommand& t)
{

    o << "MachGuiRefillLandMineCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiRefillLandMineCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachGuiRefillLandMineCommand::canActorEverExecute(const MachActor& actor) const
{
    // Locators can locate
    MachLog::ObjectType objectType = actor.objectType();
    return objectType == MachLog::SPY_LOCATOR;
}

// virtual
bool MachGuiRefillLandMineCommand::isInteractionComplete() const
{
    return hadFinalPick_;
}

// virtual
bool MachGuiRefillLandMineCommand::doApply(MachActor* pActor, std::string* /*pReason*/)
{
    PRE(pActor->objectIsMachine());

    // Check locator type
    bool canDo = false;
    if (pActor->objectType() == MachLog::SPY_LOCATOR)
    {
        MachLogRefillLandMinesOperation* pOp = new MachLogRefillLandMinesOperation(&pActor->asSpyLocator());

        // Give to actor
        pActor->newOperation(pOp);
        canDo = true;

        if (! hasPlayedVoiceMail())
        {
            MachLogMachineVoiceMailManager::instance().postNewMail(*pActor, MachineVoiceMailEventID::TASKED);
            hasPlayedVoiceMail(true);
        }
    }

    return canDo;
}

// virtual
MachGui::Cursor2dType MachGuiRefillLandMineCommand::cursorOnTerrain(const MexPoint3d&, bool, bool, bool)
{
    MachGui::Cursor2dType cursor = MachGui::INVALID_CURSOR;

    return cursor;
}

// virtual
MachGui::Cursor2dType MachGuiRefillLandMineCommand::cursorOnActor(MachActor*, bool, bool, bool)
{
    MachGui::Cursor2dType cursor = MachGui::INVALID_CURSOR;

    return cursor;
}

// virtual
void MachGuiRefillLandMineCommand::typeData(MachLog::ObjectType, int, uint)
{
    // Do nothing
}

// virtual
std::unique_ptr<MachGuiCommand> MachGuiRefillLandMineCommand::clone() const
{
    return std::make_unique<MachGuiRefillLandMineCommand>(&inGameScreen());
}

// virtual
const std::pair<std::string, std::string>& MachGuiRefillLandMineCommand::iconNames() const
{
    static std::pair<std::string, std::string> names("gui/commands/refill.bmp", "gui/commands/refill.bmp");
    return names;
}

// virtual
uint MachGuiRefillLandMineCommand::cursorPromptStringId() const
{
    return IDS_REFILLLANDMINE_COMMAND;
}

// virtual
uint MachGuiRefillLandMineCommand::commandPromptStringid() const
{
    return IDS_REFILLLANDMINE_START;
}

// virtual
bool MachGuiRefillLandMineCommand::canAdminApply() const
{
    return false;
}

// virtual
bool MachGuiRefillLandMineCommand::doAdminApply(MachLogAdministrator* /*pAdministrator*/, std::string*)
{
    PRE(canAdminApply());
    ;
    return false;
}

// virtual
bool MachGuiRefillLandMineCommand::processButtonEvent(const DevButtonEvent& be)
{
    if (isVisible() && be.scanCode() == Device::KeyCode::KEY_G && be.action() == DevButtonEvent::PRESS && be.previous() == 0)
    {
        inGameScreen().activeCommand(*this);
        return true;
    }

    return false;
}

/* End CMDLOCTO.CPP **************************************************/
