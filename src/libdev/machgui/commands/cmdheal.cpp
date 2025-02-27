/*
 * C M D H E A L . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "cmdheal.hpp"

#include "machgui/ingame.hpp"
#include "machgui/internal/strings.hpp"
#include "device/butevent.hpp"
#include "machlog/actor.hpp"
#include "machlog/administ.hpp"
#include "machlog/move.hpp"
#include "machlog/machvman.hpp"
#include "machlog/opheal.hpp"
#include "machlog/opadheal.hpp"
#include "ctl/pvector.hpp"

MachGuiHealCommand::MachGuiHealCommand(MachInGameScreen* pInGameScreen)
    : MachGuiCommand(pInGameScreen)
    , action_(HEAL_OBJECT)
    , pDirectObject_(nullptr)
    , hadFinalPick_(false)
{
    TEST_INVARIANT;
}

MachGuiHealCommand::~MachGuiHealCommand()
{
    TEST_INVARIANT;
}

void MachGuiHealCommand::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiHealCommand& t)
{

    o << "MachGuiHealCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiHealCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiHealCommand::pickOnTerrain(const MexPoint3d& location, bool ctrlPressed, bool shiftPressed, bool altPressed)
{
    // Check the location is on the ground - not up a hill
    if (cursorOnTerrain(location, ctrlPressed, shiftPressed, altPressed) != MachGui::INVALID_CURSOR)
    {
        // Store the location and set the action
        action_ = MOVE_TO_LOCATION;
        location_ = location;
        hadFinalPick_ = true;
    }
}

// virtual
void MachGuiHealCommand::pickOnActor(MachActor* pActor, bool ctrlPressed, bool shiftPressed, bool altPressed)
{
    hadFinalPick_ = cursorOnActor(pActor, ctrlPressed, shiftPressed, altPressed) != MachGui::INVALID_CURSOR;
}

// virtual
bool MachGuiHealCommand::canActorEverExecute(const MachActor& actor) const
{
    // Administrators and aggressors can Heal
    bool result = false;
    MachLog::ObjectType objectType = actor.objectType();
    if (objectType == MachLog::ADMINISTRATOR)
    {
        const MachLogAdministrator& mla = actor.asAdministrator();
        if (mla.hasHealingWeapon())
            result = true;
    }
    return result;
}

// virtual
bool MachGuiHealCommand::isInteractionComplete() const
{
    return hadFinalPick_;
}

// virtual
bool MachGuiHealCommand::doApply(MachActor* pActor, std::string* pReason)
{
    PRE(pActor->objectIsMachine());

    // Take appropriate action
    bool canDo = false;
    switch (action_)
    {
        case MOVE_TO_LOCATION:
            canDo = applyMove(pActor, pReason);
            break;

        case HEAL_OBJECT:
            canDo = applyHealObject(pActor, pReason);
            break;

        default:
            ASSERT_BAD_CASE;
    }

    return canDo;
}

bool MachGuiHealCommand::applyMove(MachActor* pActor, std::string*)
{
    PRE(pActor->objectIsMachine());

    MexPoint2d validPoint;

    bool valid
        = findClosestPointValidOnTerrain(location_, IGNORE_SELECTED_ACTOR_OBSTACLES, &pActor->asMachine(), &validPoint);

    if (valid)
    {
        // Construct a move operation
        MachLogMoveToOperation* pOp = new MachLogMoveToOperation(&pActor->asMachine(), validPoint, commandId());

        // Give it to the actor
        pActor->newOperation(pOp);

        if (! hasPlayedVoiceMail())
        {
            MachLogMachineVoiceMailManager::instance().postNewMail(*pActor, MachineVoiceMailEventID::MOVING);
            hasPlayedVoiceMail(true);
        }
    }

    return true;
}

bool MachGuiHealCommand::applyHealObject(MachActor* pActor, std::string*)
{
    // Check not trying to Heal oneself
    bool canDo = pActor != pDirectObject_;
    if (canDo)
    {
        // Construct appropriate type of operation
        MachLogOperation* pOp;

        ASSERT(
            pActor->objectType() == MachLog::ADMINISTRATOR && pActor->asAdministrator().hasHealingWeapon(),
            "Non-administrator or non-heal-capable administrator about to be issued heal op!");

        pOp = new MachLogHealOperation(&pActor->asAdministrator(), pDirectObject_);

        // Give it to the actor
        pActor->newOperation(pOp);

        if (! hasPlayedVoiceMail())
        {
            MachLogMachineVoiceMailManager::instance().postNewMail(*pActor, MachineVoiceMailEventID::HEAL_TARGET);
            hasPlayedVoiceMail(true);
        }
    }

    return canDo;
}

// virtual
MachGui::Cursor2dType MachGuiHealCommand::cursorOnTerrain(const MexPoint3d& location, bool, bool, bool)
{
    MachGui::Cursor2dType cursor = MachGui::INVALID_CURSOR;

    if (cursorInFogOfWar() || isPointValidOnTerrain(location, IGNORE_SELECTED_ACTOR_OBSTACLES))
        cursor = MachGui::MOVETO_CURSOR;

    return cursor;
}

// virtual
MachGui::Cursor2dType MachGuiHealCommand::cursorOnActor(MachActor* pActor, bool, bool, bool)
{
    MachGui::Cursor2dType cursorType = MachGui::INVALID_CURSOR;

    // Check for a machine at less than 100% hps.
    if (pActor->objectIsMachine() && pActor->hpRatio() != 1.0 && atLeastOneCanHeal(&pActor->asMachine()))
    {
        // Set the Heal object action
        action_ = HEAL_OBJECT;
        cursorType = MachGui::HEAL_CURSOR;
        pDirectObject_ = pActor;
    }

    return cursorType;
}

// virtual
void MachGuiHealCommand::typeData(MachLog::ObjectType, int, uint)
{
    // Do nothing
}

// virtual
std::unique_ptr<MachGuiCommand> MachGuiHealCommand::clone() const
{
    return std::make_unique<MachGuiHealCommand>(&inGameScreen());
}

// virtual
const std::pair<std::string, std::string>& MachGuiHealCommand::iconNames() const
{
    static std::pair<std::string, std::string> names("gui/commands/heal.bmp", "gui/commands/heal.bmp");
    return names;
}

// virtual
uint MachGuiHealCommand::cursorPromptStringId() const
{
    return IDS_HEAL_COMMAND;
}

// virtual
uint MachGuiHealCommand::commandPromptStringid() const
{
    return IDS_HEAL_START;
}

// virtual
bool MachGuiHealCommand::canAdminApply() const
{
    return false;
}

// virtual
bool MachGuiHealCommand::doAdminApply(MachLogAdministrator* pAdministrator, std::string*)
{
    PRE(canAdminApply());

    // Check not trying to Heal oneself
    bool canDo = pAdministrator != pDirectObject_;
    if (canDo)
    {
        // Create an admin Heal operation for the administrator
        MachLogAdminHealOperation* pOp = new MachLogAdminHealOperation(pAdministrator, pDirectObject_);
        pAdministrator->newOperation(pOp);

        MachActor* pFirstHealingMachine = nullptr;

        bool found = false;
        for (MachInGameScreen::Actors::const_iterator i = inGameScreen().selectedActors().begin();
             ! found && i != inGameScreen().selectedActors().end();
             ++i)
            if ((*i)->objectIsMachine() && (*i)->objectIsCanAttack() && (*i)->asCanAttack().hasHealingWeapon())
            {
                found = true;
                pFirstHealingMachine = (*i);
            }

        ASSERT(found, "No heal-capable machine found in corral!");

        // give out voicemail
        MachLogMachineVoiceMailManager::instance().postNewMail(
            *pFirstHealingMachine,
            MachineVoiceMailEventID::HEAL_TARGET);
    }

    return canDo;
}

// virtual
bool MachGuiHealCommand::processButtonEvent(const DevButtonEvent& be)
{
    if (isVisible() && be.scanCode() == Device::KeyCode::KEY_H && be.action() == DevButtonEvent::PRESS && be.previous() == 0)
    {
        inGameScreen().activeCommand(*this);
        return true;
    }

    return false;
}

bool MachGuiHealCommand::atLeastOneCanHeal(const MachLogMachine* pTargetMachine) const
{
    bool noneCanHealThis = true;

    for (MachInGameScreen::Actors::const_iterator iter = inGameScreen().selectedActors().begin();
         iter != inGameScreen().selectedActors().end() && noneCanHealThis;
         ++iter)
    {
        MachActor* pActor = (*iter);

        if (pActor->objectType() == MachLog::ADMINISTRATOR && pActor->asAdministrator().hasHealingWeapon()
            && pActor != pTargetMachine) // can't heal oneself.......
        {
            noneCanHealThis = false;
        }
    }

    return !(noneCanHealThis);
}

/* End CMDATTAC.CPP **************************************************/
