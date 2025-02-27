/*
 * C M D M O V E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "cmdmove.hpp"

#include "ctl/pvector.hpp"
#include "ctl/list.hpp"

#include "device/butevent.hpp"

#include "mathex/point2d.hpp"

#include "phys/cspace2.hpp"

#include "machphys/constron.hpp"
#include "machphys/consdata.hpp"
#include "machphys/stations.hpp"
#include "machphys/station.hpp"
#include "machphys/machdata.hpp"

#include "machlog/administ.hpp"
#include "machlog/actor.hpp"
#include "machlog/apc.hpp"
#include "machlog/camera.hpp"
#include "machlog/constron.hpp"
#include "machlog/follow.hpp"
#include "machlog/grpmove.hpp"
#include "machlog/inout.hpp"
#include "machlog/machine.hpp"
#include "machlog/machvman.hpp"
#include "machlog/move.hpp"
#include "machlog/opadmove.hpp"
#include "machlog/opeapc.hpp"
#include "machlog/planet.hpp"
#include "machlog/squad.hpp"
#include "machlog/vmman.hpp"
#include "machlog/vmdata.hpp"

#include "machgui/ingame.hpp"
#include "machgui/cameras.hpp"

#include "machgui/internal/strings.hpp"
#include "machgui/internal/mgsndman.hpp"

MachGuiMoveCommand::MachGuiMoveCommand(MachInGameScreen* pInGameScreen)
    : MachGuiCommand(pInGameScreen)
    , action_(SIMPLE_MOVE)
    , hadFinalPick_(false)
    , pConstruction_(nullptr)
{
    // Set sensible vector size
    points_.reserve(8);

    TEST_INVARIANT;
}

MachGuiMoveCommand::~MachGuiMoveCommand()
{
    TEST_INVARIANT;
}

void MachGuiMoveCommand::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiMoveCommand& t)
{

    o << "MachGuiMoveCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiMoveCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiMoveCommand::pickOnTerrain(const MexPoint3d& location, bool ctrlPressed, bool shiftPressed, bool altPressed)
{
    // Check a legal move position while updating cursor
    if (cursorOnTerrain(location, ctrlPressed, shiftPressed, altPressed) == MachGui::MOVETO_CURSOR)
    {
        // Push onto the list of points
        points_.push_back(location);

        // If the shift key was not pressed, this is the final point
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

// virtual
void MachGuiMoveCommand::pickOnActor(MachActor* pActor, bool ctrlPressed, bool shiftPressed, bool altPressed)
{
    // Update cursor, and treat as final pick if not invalid
    MachGui::Cursor2dType cursor = cursorOnActor(pActor, ctrlPressed, shiftPressed, altPressed);
    if (cursor == MachGui::INVALID_CURSOR || cursor == MachGui::MENU_CURSOR)
    {
        hadFinalPick_ = false;
    }
    else if (cursor == MachGui::MOVETO_CURSOR) // Simple move command, if shift is pressed set up way points.
    {
        // Push onto the list of points
        points_.push_back(pActor->globalTransform().position());
        hadFinalPick_ = ! shiftPressed;

        if (! hadFinalPick_)
        {
            // Waypoint click (i.e. not final click)
            MachGuiSoundManager::instance().playSound("gui/sounds/waypoint.wav");
        }
    }
    else // All other move type command finish when an actor is picked on.
    {
        hadFinalPick_ = true;
    }
}

// virtual
bool MachGuiMoveCommand::canActorEverExecute(const MachActor& actor) const
{
    // All machines can move
    return actor.objectIsMachine();
}

// virtual
bool MachGuiMoveCommand::isInteractionComplete() const
{
    return hadFinalPick_;
}

// virtual
bool MachGuiMoveCommand::doApply(MachActor* pActor, std::string* pReason)
{
    PRE(pActor->objectIsMachine());

    // Take appropriate action
    bool canDo = false;
    switch (action_)
    {
        case SIMPLE_MOVE:
            canDo = applySimpleMove(pActor, pReason);
            break;

        case ENTER_BUILDING:
            canDo = applyEnterBuilding(pActor, pReason);
            break;

        case ENTER_APC:
            canDo = applyEnterAPC(pActor, pReason);
            break;

        case FOLLOW_MACHINE:
            canDo = applyFollowMachine(pActor, pReason);
            break;

        default:
            ASSERT_BAD_CASE;
    }

    return canDo;
}

bool MachGuiMoveCommand::applySimpleMove(MachActor* pActor, std::string*)
{
    PRE(points_.size() != 0);
    PRE(pActor->objectIsMachine());

    // Construct a path in the correct format
    MachLogMoveToOperation::Path path;

    if (convertPointsToValidPoints(IGNORE_SELECTED_ACTOR_OBSTACLES, &pActor->asMachine(), points_, &path))
    {
        // Construct a move operation
        MachLogMoveToOperation* pOp
            = new MachLogMoveToOperation(&pActor->asMachine(), path, commandId(), pathFindingPriority());

        // Give it to the actor
        //  The operation is now given to the actor by the group move code
        add(&pActor->asMachine(), pOp);

        ASSERT(pActor->objectIsMachine(), "Hey! That actor should have been a machine!");
        pActor->asMachine().manualCommandIssued();

        if (! hasPlayedVoiceMail())
        {
            MachLogMachineVoiceMailManager::instance().postNewMail(*pActor, MachineVoiceMailEventID::MOVING);
            hasPlayedVoiceMail(true);
        }
    }

    return true;
}

bool MachGuiMoveCommand::applyEnterBuilding(MachActor* pActor, std::string* pReason)
{
    // ! this method has multiple exit points !

    PRE(pConstruction_ != nullptr);

    ASSERT(pActor->objectIsMachine(), "Only machines should be trying to enter a building.");

    bool result = false;

    MachLogMachine& machine = pActor->asMachine();

    // machine cannot enter if it's too fat or there's no free bays for it.

    if (! pActor->asMachine().canFitInsideConstructionNow(*pConstruction_))
        return false;

    // The internal station is a PARKING BAY except for a research lab
    MachLog::ObjectType constructionType = pConstruction_->objectType();
    MachPhysStation::Type stationType
        = ((constructionType == MachLog::HARDWARE_LAB || constructionType == MachLog::SOFTWARE_LAB)
               ? MachPhysStation::RESEARCH_BAY
               : MachPhysStation::PARKING_BAY);

    MachPhysConstructionData& constructionData
        = _CONST_CAST(MachPhysConstructionData&, pConstruction_->constructionData());

    MachPhysStations& stations = constructionData.stations();
    MachPhysStation* pStation = nullptr;
    if (pConstruction_->race() == pActor->race())
        result = stations.freeStation(stationType, &pStation);
    else
        result = pConstruction_->hasInterior();

    if (result)
    {
        // Yes, we can enter the building
        MachLogEnterBuildingOperation* pEnterOp
            = new MachLogEnterBuildingOperation(&machine, pConstruction_, pStation, pathFindingPriority());

        // Give it to the actor
        pActor->newOperation(pEnterOp);

        machine.manualCommandIssued();

        if (pActor == inGameScreen().selectedActors().front())
        {
            // in cases of this being a spy enterign an enemy building,
            // trigger "I'll attempt to download enemy's research" voicemail instead of normal move mail

            if (pActor->objectType() == MachLog::SPY_LOCATOR && pConstruction_->objectType() == MachLog::HARDWARE_LAB
                && (pActor->race() != pConstruction_->race()))
            {
                if (! hasPlayedVoiceMail())
                {
                    MachLogVoiceMailManager::instance().postNewMail(
                        VID_SPY_TASKED_INFLITRATE,
                        pActor->id(),
                        pActor->race());
                    hasPlayedVoiceMail(true);
                }
            }
            else
            {
                if (! hasPlayedVoiceMail())
                {
                    MachLogMachineVoiceMailManager::instance().postNewMail(*pActor, MachineVoiceMailEventID::MOVING);
                    hasPlayedVoiceMail(true);
                }
            }
        }
    }
    else
        *pReason = "Building has no free space";

    return result;
}

bool MachGuiMoveCommand::applyEnterAPC(MachActor* pActor, std::string*)
{
    PRE(pAPC_ != nullptr);
    PRE(pActor->objectIsMachine());

    MachLogMachine& machine = pActor->asMachine();

    bool canEnter = false;

    if (couldEnterAPC(*pActor, *pAPC_))
    {
        // Construct an enter APC operation
        MachLogEnterAPCOperation* pOp = new MachLogEnterAPCOperation(&machine, pAPC_, pathFindingPriority());

        // Give it to the actor
        pActor->newOperation(pOp);

        machine.manualCommandIssued();

        if (! hasPlayedVoiceMail())
        {
            MachLogMachineVoiceMailManager::instance().postNewMail(*pActor, MachineVoiceMailEventID::MOVING);
            hasPlayedVoiceMail(true);
        }

        canEnter = true;
    }

    return canEnter;
}

bool MachGuiMoveCommand::applyFollowMachine(MachActor* pActor, std::string*)
{
    bool result = false;

    PRE(pMachine_ != nullptr);

    // Check not trying to follow self
    if (pActor != _STATIC_CAST(MachActor*, pMachine_))
    {
        // Construct a follow operation
        MachLogFollowOperation* pOp
            = new MachLogFollowOperation(&pActor->asMachine(), pMachine_, MexPoint2d(0, 0), pathFindingPriority());

        // Give it to the actor

        // The operation is now given to the actor by the group move code
        add(&pActor->asMachine(), pOp);

        ASSERT(pActor->objectIsMachine(), "Hey! That actor should have been a machine!");
        pActor->asMachine().manualCommandIssued();

        if (! hasPlayedVoiceMail())
        {
            MachLogMachineVoiceMailManager::instance().postNewMail(*pActor, MachineVoiceMailEventID::MOVING);
            hasPlayedVoiceMail(true);
        }

        result = true;
    }

    return result;
}

// virtual
MachGui::Cursor2dType MachGuiMoveCommand::cursorOnTerrain(const MexPoint3d& location, bool, bool, bool)
{
    MachGui::Cursor2dType cursorType = MachGui::MENU_CURSOR;

    // Check the location is on the ground - not up a hill

    if (! inGameScreen().cameras()->currentCamera()->insideConstruction())
    {
        if (cursorInFogOfWar() || isPointValidOnTerrain(location, IGNORE_SELECTED_ACTOR_OBSTACLES))
        {
            cursorType = MachGui::MOVETO_CURSOR;
        }
    }
    else
    {
        cursorType = MachGui::MOVETO_CURSOR;
    }

    return cursorType;
}

// virtual
MachGui::Cursor2dType MachGuiMoveCommand::cursorOnActor(MachActor* pActor, bool, bool, bool altPressed)
{
    MachGui::Cursor2dType cursorType = MachGui::INVALID_CURSOR;

    bool sameRace = (inGameScreen().selectedActors().front()->race() == pActor->race());

    // Check for a building
    if (pActor->objectIsConstruction())
    {
        MachLogConstruction* pConstruction = &pActor->asConstruction();

        if (atLeastOneCorralActorCanEnterConstruction(inGameScreen(), *pConstruction))
        {
            // Set enter building action and store the building pointer
            action_ = ENTER_BUILDING;
            cursorType = MachGui::ENTER_BUILDING_CURSOR;
            pConstruction_ = pConstruction;
        }
    }
    else if (pActor->objectType() == MachLog::APC && sameRace && ! altPressed) // If ALT is pressed then follow the
                                                                                   // APC instead
    {
        pAPC_ = &pActor->asAPC();

        // Check to see if at least one actor could enter this here APC
        // an APC
        bool noneCanEnter = true;
        for (MachInGameScreen::Actors::const_iterator iter = inGameScreen().selectedActors().begin();
             iter != inGameScreen().selectedActors().end() && noneCanEnter;
             ++iter)
        {
            if (couldEnterAPC((**iter), *pAPC_))
            {
                noneCanEnter = false;
            }
        }

        if (! noneCanEnter)
        {
            // Set enter APC action and store the pointer
            action_ = ENTER_APC;
            cursorType = MachGui::ENTER_APC_CURSOR;
        }
    }
    else if (pActor->objectIsMachine())
    {
        // Make sure we've not got the cursor over the same machine and
        // trying to tell it to follow...
        if (inGameScreen().selectedActors().size() == 1 && inGameScreen().selectedActors().front() == pActor)
        {
            cursorType = MachGui::MENU_CURSOR;
        }
        else
        {
            // Follow it
            action_ = FOLLOW_MACHINE;
            cursorType = MachGui::FOLLOW_CURSOR;
            pMachine_ = &pActor->asMachine();
        }
    }
    else if (pActor->objectIsDebris() || pActor->objectIsOreHolograph())
    {
        // Regular move
        action_ = SIMPLE_MOVE;
        cursorType = MachGui::MOVETO_CURSOR;
    }

    return cursorType;
}

// virtual
void MachGuiMoveCommand::typeData(MachLog::ObjectType, int, uint)
{
    // Do nothing
}

// virtual
std::unique_ptr<MachGuiCommand> MachGuiMoveCommand::clone() const
{
    return std::make_unique<MachGuiMoveCommand>(&inGameScreen());
}

// virtual
const std::pair<std::string, std::string>& MachGuiMoveCommand::iconNames() const
{
    static std::pair<std::string, std::string> names("gui/commands/move.bmp", "gui/commands/move.bmp");
    return names;
}

// virtual
uint MachGuiMoveCommand::cursorPromptStringId() const
{
    return IDS_MOVE_COMMAND;
}

// virtual
uint MachGuiMoveCommand::commandPromptStringid() const
{
    return IDS_MOVE_START;
}

// virtual
bool MachGuiMoveCommand::canAdminApply() const
{
    return true;
}

// virtual
bool MachGuiMoveCommand::actorsCanExecute() const
{
    // update minimum access size as we go

    int tempGroupLowest = 100;
    int tempCandidateLowest = 100;

    const MachInGameScreen::Actors& selectionSet = inGameScreen().selectedActors();

    for (MachInGameScreen::Actors::const_iterator i = selectionSet.begin(); i != selectionSet.end(); ++i)
    {
        if ((*i)->objectIsMachine())

        {
            tempCandidateLowest = (*i)->asMachine().machineData().minAccessSizeIndex();

            if (tempCandidateLowest < tempGroupLowest)
                tempGroupLowest = tempCandidateLowest;
        }
    }

    lowestMinimumAccessSize() = tempGroupLowest;

    return MachGuiCommand::actorsCanExecute();
}

// static
int& MachGuiMoveCommand::lowestMinimumAccessSize()
{
    static int lowestMinimumAccessSize = 100;
    return lowestMinimumAccessSize;
}

// virtual
bool MachGuiMoveCommand::doAdminApply(MachLogAdministrator* pAdministrator, std::string*)
{
    PRE(canAdminApply());
    ;
    PRE(points_.size() != 0);

    // Construct a path in the correct format
    MachLogMoveToOperation::Path path;

    if (convertPointsToValidPoints(IGNORE_SELECTED_ACTOR_OBSTACLES, &pAdministrator->asMachine(), points_, &path))
    {
        // Create an admin Move operation for the administrator
        MachLogAdminMoveToOperation* pOp
            = new MachLogAdminMoveToOperation(pAdministrator, path, pathFindingPriority());

        // give voicemail
        MachLogMachineVoiceMailManager::instance().postNewMail(*pAdministrator, MachineVoiceMailEventID::MOVING);

        // The operation is now given to the actor by the group move code
        add(pAdministrator, pOp);

        ASSERT(pAdministrator->squadron(), "Administrator didn't have a squadron!");
        pAdministrator->squadron()->manualCommandIssuedToSquadron();
    }

    return true;
}

// virtual
bool MachGuiMoveCommand::canApplyToGroup() const
{
    return action_ == SIMPLE_MOVE;
}

// virtual
bool MachGuiMoveCommand::doGroupApply(const Actors& actors, std::string* pReason)
{
    PRE(canApplyToGroup());
    PRE(points_.size() != 0);

    bool canApply = true;

    MachLogMachine* pMachine = nullptr;

    for (Actors::const_reverse_iterator i = actors.rbegin(); i != actors.rend(); ++i)
    {
        if ((*i)->objectIsMachine())
        {
            pMachine = &(*i)->asMachine();
            pMachine->manualCommandIssued();
        }
    }
    // note that if pMachine is now non-null, it will point to the FIRST machine in the group.

    ctl_vector<MexPoint3d> validPoints;
    validPoints.reserve(8);
    size_t nPoints = points_.size();

    // Iterate through way points and adjust to valid way points.
    for (size_t i = 0; i != nPoints; ++i)
    {
        MexPoint2d validPoint;

        bool valid = findClosestPointValidOnTerrain(points_[i], IGNORE_SELECTED_ACTOR_OBSTACLES, pMachine, &validPoint);

        if (valid)
        {
            validPoints.push_back(validPoint);
        }
    }

    if (validPoints.size() > 0)
    {
        MachLogGroupSimpleMove groupMove(actors, validPoints, commandId(), pReason, pathFindingPriority());
        canApply = groupMove.moveOK();

        if (canApply && pMachine)
        {
            // give voicemail
            MachLogMachineVoiceMailManager::instance().postNewMail(*pMachine, MachineVoiceMailEventID::MOVING);
        }
    }

    return canApply;
}

// virtual
bool MachGuiMoveCommand::processButtonEvent(const DevButtonEvent& be)
{
    if (isVisible() && be.scanCode() == Device::KeyCode::KEY_M && be.action() == DevButtonEvent::PRESS
        && be.wasAltPressed() == false && be.wasCtrlPressed() == false && be.wasShiftPressed() == false
        && be.previous() == 0)
    {
        inGameScreen().activeCommand(*this);
        return true;
    }

    return false;
}

// static
bool MachGuiMoveCommand::couldEnterAPC(const MachActor& actor, const MachLogAPC& apc)
{
    return (
        actor.objectIsMachine() && actor.objectType() != MachLog::APC && !(actor.asMachine().machineIsGlider())
        && apc.roomForMachine(actor.asMachine()));
}

// static
bool MachGuiMoveCommand::atLeastOneCorralActorCanEnterConstruction(
    const MachInGameScreen& inGameScreen,
    const MachLogConstruction& construction)
{

    bool noneCanEnter = true;

    // Check first to see if the corral has any machines in it, terminate early if not.
    if (inGameScreen.corralState() & MachInGameScreen::CORRAL_SOMEMACHINES)
    {
        for (MachInGameScreen::Actors::const_iterator iter = inGameScreen.selectedActors().begin();
             iter != inGameScreen.selectedActors().end() && noneCanEnter;
             ++iter)
        {
            if ((*iter)->objectIsMachine() && (*iter)->asMachine().canFitInsideConstructionNow(construction))
            {
                noneCanEnter = false;
            }
        }
    }

    return !(noneCanEnter);
}

/* End CMDMOVE.CPP **************************************************/
