/*
 * C M D C A P T R  . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "cmdcaptr.hpp"

#include "machgui/ingame.hpp"

#include "machgui/internal/mgsndman.hpp"
#include "machgui/internal/strings.hpp"

#include "ctl/algorith.hpp"

#include "world4d/domain.hpp"
#include "device/butevent.hpp"
#include "machlog/actor.hpp"
#include "machlog/administ.hpp"
#include "machlog/opadsupc.hpp"
#include "machlog/opsupcon.hpp"
#include "machlog/races.hpp"
#include "machlog/squad.hpp"
#include "machlog/machvman.hpp"

MachGuiCaptureCommand::MachGuiCaptureCommand(MachInGameScreen* pInGameScreen)
    : MachGuiCommand(pInGameScreen)
    , hadFinalPick_(false)
{
    TEST_INVARIANT;
}

MachGuiCaptureCommand::~MachGuiCaptureCommand()
{
    TEST_INVARIANT;

    inGameScreen().setCursorFilter(W4dDomain::EXCLUDE_NOT_SOLID);

    while (! constructions_.empty())
    {
        constructions_.back()->detach(this);
        constructions_.pop_back();
    }
}

void MachGuiCaptureCommand::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCaptureCommand& t)
{

    o << "MachGuiCaptureCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCaptureCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiCaptureCommand::pickOnActor(MachActor* pActor, bool, bool shiftPressed, bool)
{
    MachPhys::Race playerRace = MachLogRaces::instance().playerRace();

    // Check for a pick on an enemy construction
    if (pActor->objectIsConstruction() && pActor->race() != playerRace)
    {
        MachLogConstruction* pCandidateConstruction = &pActor->asConstruction();

        if (! constructionIsDuplicate(pCandidateConstruction))
        {
            // Add to list of constructions to capture
            constructions_.push_back(pCandidateConstruction);

            pCandidateConstruction->attach(this);
        }

        if (! shiftPressed)
        {
            hadFinalPick_ = true;
        }
        else
        {
            // Waypoint click (i.e. not final click)
            MachGuiSoundManager::instance().playSound("gui/sounds/waypoint.wav");
        }
    }
}

bool MachGuiCaptureCommand::constructionIsDuplicate(const MachLogConstruction* pCandidateConstruction) const
{
    ctl_pvector<MachLogConstruction>::const_iterator i
        = find(constructions_.begin(), constructions_.end(), pCandidateConstruction);

    return i != constructions_.end();
}

// virtual
bool MachGuiCaptureCommand::canActorEverExecute(const MachActor& actor) const
{
    // Constructions can be deconstructed
    MachLog::ObjectType objectType = actor.objectType();
    return objectType == MachLog::CONSTRUCTOR;
}

// virtual
bool MachGuiCaptureCommand::isInteractionComplete() const
{
    return hadFinalPick_;
}

// virtual
MachGui::Cursor2dType
MachGuiCaptureCommand::cursorOnTerrain(const MexPoint3d& /*location*/, bool /*ctrlPressed*/, bool, bool)
{
    return MachGui::MENU_CURSOR;
}

// virtual
MachGui::Cursor2dType MachGuiCaptureCommand::cursorOnActor(MachActor* pActor, bool, bool, bool)
{
    MachGui::Cursor2dType cursor = MachGui::INVALID_CURSOR;

    MachPhys::Race playerRace = MachLogRaces::instance().playerRace();

    if (pActor->objectIsConstruction() && pActor->asConstruction().isComplete() && pActor->race() != playerRace)
    {
        cursor = MachGui::CAPTURE_CURSOR;
    }

    return cursor;
}

// virtal
void MachGuiCaptureCommand::typeData(MachLog::ObjectType /*objectType*/, int /*subType*/, uint /*level*/)
{
}

// virtual
bool MachGuiCaptureCommand::doApply(MachActor* pActor, std::string*)
{
    MachLogSuperConstructOperation* pOp = new 
        MachLogSuperConstructOperation(&pActor->asConstructor(), constructions_, MachLogOperation::CAPTURE_OPERATION);
    pActor->newOperation(pOp);

    ASSERT(pActor->objectIsMachine(), "Hey! That actor should have been a machine!");
    pActor->asMachine().manualCommandIssued();

    if (! hasPlayedVoiceMail())
    {
        MachLogMachineVoiceMailManager::instance().postNewMail(*pActor, MachineVoiceMailEventID::MOVE_TO_SITE);
        hasPlayedVoiceMail(true);
    }

    return true;
}

// virtual
std::unique_ptr<MachGuiCommand> MachGuiCaptureCommand::clone() const
{
    return std::make_unique<MachGuiCaptureCommand>(&inGameScreen());
}

// virtual
const std::pair<std::string, std::string>& MachGuiCaptureCommand::iconNames() const
{
    static std::pair<std::string, std::string> names("gui/commands/capture.bmp", "gui/commands/capture.bmp");
    return names;
}

// virtual
uint MachGuiCaptureCommand::cursorPromptStringId() const
{
    return IDS_CAPTURE_COMMAND;
}

// virtual
uint MachGuiCaptureCommand::commandPromptStringid() const
{
    return IDS_CAPTURE_START;
}

// virtual
bool MachGuiCaptureCommand::canAdminApply() const
{
    return true;
}

// virtual
bool MachGuiCaptureCommand::doAdminApply(MachLogAdministrator* pAdministrator, std::string*)
{
    PRE(canAdminApply());

    // Create an admin superconstruct(capture) operation for the administrator
    MachLogAdminSuperConstructOperation* pOp = new 
        MachLogAdminSuperConstructOperation(pAdministrator, constructions_, MachLogOperation::CAPTURE_OPERATION);

    pAdministrator->newOperation(pOp);
    ASSERT(pAdministrator->squadron(), "Administrator didn't have a squadron!");
    pAdministrator->squadron()->manualCommandIssuedToSquadron();

    MachActor* pFirstConstructor = nullptr;

    bool found = false;
    for (MachInGameScreen::Actors::const_iterator i = inGameScreen().selectedActors().begin();
         ! found && i != inGameScreen().selectedActors().end();
         ++i)
        if ((*i)->objectType() == MachLog::CONSTRUCTOR)
        {
            found = true;
            pFirstConstructor = (*i);
        }

    ASSERT(found, "No constructor found in corral!");

    // give out voicemail
    MachLogMachineVoiceMailManager::instance().postNewMail(*pFirstConstructor, MachineVoiceMailEventID::MOVE_TO_SITE);

    return true;
}

// virtual
bool MachGuiCaptureCommand::processButtonEvent(const DevButtonEvent& be)
{
    if (isVisible() && be.scanCode() == Device::KeyCode::KEY_U && be.action() == DevButtonEvent::PRESS && be.previous() == 0)
    {
        inGameScreen().activeCommand(*this);
        return true;
    }

    return false;
}

// virtual
bool MachGuiCaptureCommand::beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int /*clientData*/)
{
    bool stayAttached = true;

    switch (event)
    {
        case W4dSubject::DELETED:
            {
                ctl_pvector<MachLogConstruction>::iterator i
                    = find(constructions_.begin(), constructions_.end(), pSubject);
                if (i != constructions_.end())
                {
                    // one of our constructions has been destroyed
                    stayAttached = false;
                    constructions_.erase(i);
                }
            }
            break;

        default:;
    }

    return stayAttached;
}

// virtual
void MachGuiCaptureCommand::domainDeleted(W4dDomain*)
{
    // inentionally empty...override as necessary
}

/* End CMDCAPTR.CPP **************************************************/
