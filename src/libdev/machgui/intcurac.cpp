/*
 * I N T C U R A C . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/intcurac.hpp"

#include "ctl/pvector.hpp"

#include "machphys/objdata.hpp"
#include "machphys/constron.hpp"

#include "machlog/constron.hpp"
#include "machlog/administ.hpp"
#include "machlog/aggressr.hpp"
#include "machlog/construc.hpp"
#include "machlog/geolocat.hpp"
#include "machlog/mine.hpp"
#include "machlog/missilem.hpp"
#include "machlog/pod.hpp"
#include "machlog/spy.hpp"
#include "machlog/technici.hpp"
#include "machlog/apc.hpp"
#include "machlog/rescarr.hpp"
#include "machlog/races.hpp"

#include "machgui/commands/cmdmove.hpp"
#include "machgui/ingame.hpp"

MachGuiIntelligentCursorOnActor::MachGuiIntelligentCursorOnActor()
    : pCursorActor_(nullptr)
    , useCursor_(MachGui::SELECT_CURSOR)
{

    TEST_INVARIANT;
}

MachGuiIntelligentCursorOnActor::~MachGuiIntelligentCursorOnActor()
{
    TEST_INVARIANT;
}

void MachGuiIntelligentCursorOnActor::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiIntelligentCursorOnActor& t)
{

    o << "MachGuiIntelligentCursorOnActor " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiIntelligentCursorOnActor " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

MachGui::Cursor2dType MachGuiIntelligentCursorOnActor::cursorType(
    MachActor* pSelectedActor,
    MachActor* pActorUnderCursor,
    MachInGameScreen* pInGameScreen,
    bool ctrlPressed,
    bool shiftPressed,
    bool altPressed)
{
    ctrlPressed_ = ctrlPressed;
    shiftPressed_ = shiftPressed;
    altPressed_ = altPressed;
    pInGameScreen_ = pInGameScreen;
    pCursorActor_ = pActorUnderCursor;

    MachPhys::Race playerRace = MachLogRaces::instance().playerRace();

    bool isActorPlayersRace
        = (MachLogRaces::instance().dispositionToRace(playerRace, pSelectedActor->race()) == MachLogRaces::OUR_RACE);

    // Don't waste time doing complex processing if the actor does not belong to the
    // player.
    if (! isActorPlayersRace)
    {
        // Only one non-player race actor can be selected at anyone time, therefore, if CTRL pressed, show
        // invalid cursor.
        if (! ctrlPressed && pActorUnderCursor->selectableType() == MachLog::FULLY_SELECTABLE)
        {
            useCursor_ = MachGui::SELECT_CURSOR;
        }
        else if (
            pActorUnderCursor->objectIsDebris() || pActorUnderCursor->objectIsOreHolograph()
            || pActorUnderCursor->objectIsArtefact())
        {
            useCursor_ = MachGui::MENU_CURSOR;
        }
        else
        {
            useCursor_ = MachGui::INVALID_CURSOR;
        }
    }
    else
    {
        // Use the restorer
        process(pSelectedActor);
    }

    // Return the result
    return useCursor_;
}

// virtual
MachLogTypeRestorer::ProcessAsSubtype
MachGuiIntelligentCursorOnActor::doProcessConstruction(MachLogConstruction* pConstruction)
{
    ASSERT(pCursorActor_, "pCursorActor_ should not be NULL in MachGuiIntelligentCursorOnActor::doProcessConstruction");

    bool typeSpecificHandling = false;

    // Can construction attack...
    if (pConstruction->objectType() == MachLog::MISSILE_EMPLACEMENT // Must be missile emplacement
        && pConstruction->asMissileEmplacement().subType() != MachPhys::ICBM // Must not be Nuke
        && ! pCursorActor_->objectIsDebris() // Can't attack debris
        && ! pCursorActor_->objectIsOreHolograph() // Can't attack ore
        && pConstruction->asMissileEmplacement().canFireAt(*pCursorActor_))
    {
        // Check for an enemy unit, and if so, attack it
        MachLogRaces::DispositionToRace disposition
            = MachLogRaces::instance().dispositionToRace(pConstruction->race(), pCursorActor_->race());
        bool enemyTarget = (disposition == MachLogRaces::ENEMY || disposition == MachLogRaces::NEUTRAL);

        if (enemyTarget)
        {
            typeSpecificHandling = true;
            useCursor_ = MachGui::ATTACK_CURSOR;
        }
    }

    // If nothing special, use standard machine logic
    if (! typeSpecificHandling)
        doDefaultProcess(pConstruction);

    return DO_NOT_PROCESS_AS_SUBTYPE;
}

// virtual
MachLogTypeRestorer::ProcessAsSubtype MachGuiIntelligentCursorOnActor::doDefaultProcess(MachActor* pActor)
{
    ASSERT(pCursorActor_, "pCursorActor_ should not be NULL in MachGuiIntelligentCursorOnActor::doDefaultProcess");

    MachLogRaces& races = MachLogRaces::instance();

    MachPhys::Race playerRace = races.playerRace();
    bool isPlayersRace = playerRace == pCursorActor_->race();

    // Get the operand type
    MachLog::ObjectType operandType = pCursorActor_->objectType();

    if ((! isPlayersRace) && ctrlPressed_)
        useCursor_ = MachGui::INVALID_CURSOR; // Only allow one enemy actor to be selected.
    else
    {
        if (pCursorActor_->selectableType() == MachLog::FULLY_SELECTABLE)
        {
            // If we're wounded, default on a friendly garrison is enter cursor
            if (! ctrlPressed_ && // Ctrl overrides all intelligent cursors with select cursor
                operandType == MachLog::GARRISON && // Cursor on garrison
                pActor->objectIsMachine() && // I am a machine
                isPlayersRace) // Garrison is my race
            // not( pActor->hpRatio() >= 1.0 ) )  // Wounded ( removed due to Acclaim bug report, NA 9/1/99 )
            {
                // must check to see that at least one actor in the corral is capable of entering the building
                if (MachGuiMoveCommand::atLeastOneCorralActorCanEnterConstruction(
                        *pInGameScreen_,
                        pCursorActor_->asConstruction()))
                    useCursor_ = MachGui::ENTER_BUILDING_CURSOR; // Try to enter the construction
                else
                    useCursor_ = MachGui::SELECT_CURSOR;
            }
            // If over smelter we may want to recycle the machine
            else if (
                altPressed_ && // We want an alternative cursor ( alt pressed )
                ! ctrlPressed_ && // Ctrl overrides all intelligent cursors with select cursor
                (operandType == MachLog::SMELTER || operandType == MachLog::POD) && // Cursor on smelt-capable building
                isPlayersRace && // Building is my race
                pActor->objectIsMachine() && // I am a machine
                pCursorActor_->asConstruction().isComplete()) // Building is finished and operational
            {
                useCursor_ = MachGui::RECYCLE_CURSOR;
            }
            // If over machine then we may want to follow it
            else if (
                altPressed_ && // We want an alternative cursor ( alt pressed )
                ! ctrlPressed_ && // Ctrl overrides all intelligent cursors with select cursor
                pCursorActor_->objectIsMachine() && // Cursor is on machine
                pActor->objectIsMachine()) // I am a machine
            {
                useCursor_ = MachGui::FOLLOW_CURSOR;
            }
            // If over construction we may want to enter it
            else if (
                altPressed_ && // We want an alternative cursor ( alt pressed )
                ! ctrlPressed_ && // Ctrl overrides all intelligent cursors with select cursor
                pCursorActor_->objectIsConstruction() && // Cursor is on building
                pActor->objectIsMachine() && // I am a machine
                pCursorActor_->asConstruction().physConstruction().hasInterior()) // Building can be entered
            {
                // must check to see that at least one actor in the corral is capable of entering the building
                if (MachGuiMoveCommand::atLeastOneCorralActorCanEnterConstruction(
                        *pInGameScreen_,
                        pCursorActor_->asConstruction()))
                    useCursor_ = MachGui::ENTER_BUILDING_CURSOR; // Try to enter the construction
                else
                    useCursor_ = MachGui::SELECT_CURSOR;
            }
            else
            {
                useCursor_
                    = MachGui::SELECT_CURSOR; // Either selecting one enemy actor or another of the players actors
            }
        }
        else
        {
            // If the non-fully-selectable item is debris then allow machines to move on top
            // of it but don't allow it to be selected into the corral.
            if (pCursorActor_->objectIsDebris() && pActor->objectIsMachine())
            {
                if (pActor->objectType() == MachLog::GEO_LOCATOR)
                {
                    useCursor_ = MachGui::LOCATETO_CURSOR;
                }
                else
                {
                    useCursor_ = MachGui::MOVETO_CURSOR;
                }
            }
            else
            {
                useCursor_ = MachGui::MENU_CURSOR; // Don't allow non selectable items to be selected
            }
        }
    }

    return DO_NOT_PROCESS_AS_SUBTYPE;
}

// virtual
MachLogTypeRestorer::ProcessAsSubtype MachGuiIntelligentCursorOnActor::doProcessMachine(MachLogMachine*)
{
    return PROCESS_AS_SUBTYPE;
}

// virtual
MachLogTypeRestorer::ProcessAsSubtype
MachGuiIntelligentCursorOnActor::doProcessAdministrator(MachLogAdministrator* pActor)
{
    ASSERT(
        pCursorActor_,
        "pCursorActor_ should not be NULL in MachGuiIntelligentCursorOnActor::doProcessAdministrator");

    bool typeSpecificHandling = false;

    // Don't attack debris or ore...
    if (! pCursorActor_->objectIsDebris() && ! pCursorActor_->objectIsOreHolograph()
        && pActor->canFireAt(*pCursorActor_))
    {
        // Check for an enemy unit, and if so, attack it
        MachLogRaces::DispositionToRace disposition
            = MachLogRaces::instance().dispositionToRace(pActor->race(), pCursorActor_->race());
        bool enemyTarget = (disposition == MachLogRaces::ENEMY || disposition == MachLogRaces::NEUTRAL);

        if (enemyTarget)
        {
            if (pActor->asCanAttack().hasTreacheryWeapon()
                && altPressed_ // We want an alternative cursor ( alt pressed )
                && ! ctrlPressed_ // Ctrl overrides all intelligent cursors with select cursor
                && pCursorActor_->objectIsMachine())
            {
                typeSpecificHandling = true;
                useCursor_ = MachGui::TREACHERY_CURSOR;
            }
            else
            {
                typeSpecificHandling = true;
                useCursor_ = MachGui::ATTACK_CURSOR;
            }
        }
        else
        {
            if (! enemyTarget && pActor->objectIsCanAttack())
            {
                // Has the administrator got the potential to heal? ( make sure we don't try and heal ourself ).
                if (pActor->asCanAttack().hasHealingWeapon() && (pActor != pCursorActor_)
                    && (pCursorActor_->objectIsMachine())
                    && ! ctrlPressed_) // Ctrl overrides all intelligent cursors with select cursor
                {
                    // Find out if machine being pointed at needs healing...
                    if (pCursorActor_->hpRatio() != 1.0)
                    {
                        useCursor_ = MachGui::HEAL_CURSOR;
                        typeSpecificHandling = true;
                    }
                }
            }
        }
    }

    // If nothing special, use standard machine logic
    if (! typeSpecificHandling)
        processStandardMachine(pActor);

    return DO_NOT_PROCESS_AS_SUBTYPE;
}

// virtual
MachLogTypeRestorer::ProcessAsSubtype MachGuiIntelligentCursorOnActor::doProcessAggressor(MachLogAggressor* pActor)
{
    ASSERT(pCursorActor_, "pCursorActor_ should not be NULL in MachGuiIntelligentCursorOnActor::doProcessAggressor");

    bool typeSpecificHandling = false;

    if (! pCursorActor_->objectIsDebris() && ! pCursorActor_->objectIsOreHolograph()
        && pActor->canFireAt(*pCursorActor_))
    {
        // Check for an enemy unit, and if so, attack it
        MachLogRaces::DispositionToRace disposition
            = MachLogRaces::instance().dispositionToRace(pActor->race(), pCursorActor_->race());
        bool enemyTarget = (disposition == MachLogRaces::ENEMY || disposition == MachLogRaces::NEUTRAL);

        if (enemyTarget)
        {
            typeSpecificHandling = true;
            useCursor_ = MachGui::ATTACK_CURSOR;
        }
    }

    // If nothing special, use standard machine logic
    if (! typeSpecificHandling)
        processStandardMachine(pActor);

    return DO_NOT_PROCESS_AS_SUBTYPE;
}

// virtual
MachLogTypeRestorer::ProcessAsSubtype MachGuiIntelligentCursorOnActor::doProcessConstructor(MachLogConstructor* pActor)
{
    ASSERT(pCursorActor_, "pCursorActor_ should not be NULL in MachGuiIntelligentCursorOnActor::doProcessConstructor");

    bool typeSpecificHandling = false;

    MachLogRaces::DispositionToRace disposition
        = MachLogRaces::instance().dispositionToRace(pActor->race(), pCursorActor_->race());

    bool friendlyRace = (disposition == MachLogRaces::OUR_RACE || disposition == MachLogRaces::ALLY);

    // Check for an incomplete friendly building, and join in construction or repair.
    if (pCursorActor_->objectIsConstruction())
    {
        if (friendlyRace)
        {
            // If ctrl pressed then we use selection cursor, see default processing.
            if (! altPressed_ && ! ctrlPressed_)
            {
                // Get HP info ( max HP and current HP )
                const MachPhysObjectData& objData = pCursorActor_->objectData();
                MachPhys::HitPointUnits maxHp = objData.hitPoints();

                // If construction is not complete then join in...
                if (! pCursorActor_->asConstruction().isComplete())
                {
                    typeSpecificHandling = true;
                    useCursor_ = MachGui::JOINCONSTRUCT_CURSOR;
                }
                // ...else check for damaged construction and repair it.
                else if (pCursorActor_->hp() != maxHp)
                {
                    typeSpecificHandling = true;
                    useCursor_ = MachGui::REPAIR_CURSOR;
                }
            }
            else if (altPressed_ && ! ctrlPressed_)
            {
                typeSpecificHandling = true;
                useCursor_ = MachGui::DECONSTRUCT_CURSOR;
            }
        }
        else // Enemy race
        {
            if (pCursorActor_->asConstruction().isComplete())
            {
                typeSpecificHandling = true;
                if (altPressed_)
                {
                    useCursor_ = MachGui::DECONSTRUCT_CURSOR;
                }
                else
                {
                    useCursor_ = MachGui::CAPTURE_CURSOR;
                }
            }
            else /*if( pCursorActor_->asConstruction().constructedUnits() > 0 )*/
            {
                useCursor_ = MachGui::DECONSTRUCT_CURSOR;
                typeSpecificHandling = true;
            }
        }
    }

    // If nothing special, use standard machine logic
    if (! typeSpecificHandling)
        processStandardMachine(pActor);

    return DO_NOT_PROCESS_AS_SUBTYPE;
}

// virtual
MachLogTypeRestorer::ProcessAsSubtype MachGuiIntelligentCursorOnActor::doProcessGeoLocator(MachLogGeoLocator* pActor)
{
    bool typeSpecificHandling = false;

    // If nothing special, use standard machine logic
    if (! typeSpecificHandling)
        processStandardMachine(pActor);

    return DO_NOT_PROCESS_AS_SUBTYPE;
}

// virtual
MachLogTypeRestorer::ProcessAsSubtype MachGuiIntelligentCursorOnActor::doProcessSpyLocator(MachLogSpyLocator* pActor)
{
    // Get the operand type
    MachLog::ObjectType operandType = pCursorActor_->objectType();

    MachLogRaces::DispositionToRace disposition
        = MachLogRaces::instance().dispositionToRace(pActor->race(), pCursorActor_->race());

    bool ourRace = (disposition == MachLogRaces::OUR_RACE);

    // Check for the operand being a building
    if (!ourRace && operandType == MachLog::HARDWARE_LAB && ! ctrlPressed_)
    {
        // Try to enter the construction
        useCursor_ = MachGui::ENTER_BUILDING_CURSOR;
    }
    else if (ourRace && operandType == MachLog::GARRISON && ! pActor->fullyStockedMines() && ! ctrlPressed_)
    {
        // Try to enter the construction
        useCursor_ = MachGui::ENTER_BUILDING_CURSOR;
    }
    else
    {
        processStandardMachine(pActor);
    }

    return DO_NOT_PROCESS_AS_SUBTYPE;
}

// virtual
MachLogTypeRestorer::ProcessAsSubtype MachGuiIntelligentCursorOnActor::doProcessTechnician(MachLogTechnician* pActor)
{
    // Get the operand type
    MachLog::ObjectType operandType = pCursorActor_->objectType();

    MachLogRaces::DispositionToRace disposition
        = MachLogRaces::instance().dispositionToRace(pActor->race(), pCursorActor_->race());

    bool valid = (disposition == MachLogRaces::OUR_RACE);

    // Check for the operand being a friendly lab. If ctrl is pressed then user probably wants to select
    // the lab instead of have the machine enter it.
    if (valid && pCursorActor_->objectIsConstruction() && operandType == MachLog::HARDWARE_LAB && ! ctrlPressed_
        && pActor->canFitInsideConstructionNow(pCursorActor_->asConstruction()))
    {
        // Try to enter the construction
        useCursor_ = MachGui::ENTER_BUILDING_CURSOR;
    }
    else
    {
        processStandardMachine(pActor);
    }

    return DO_NOT_PROCESS_AS_SUBTYPE;
}

// virtual
MachLogTypeRestorer::ProcessAsSubtype MachGuiIntelligentCursorOnActor::doProcessPersonnelCarrier(MachLogAPC* pActor)
{
    bool typeSpecificHandling = false;

    // If nothing special, use standard machine logic
    if (! typeSpecificHandling)
        processStandardMachine(pActor);

    return DO_NOT_PROCESS_AS_SUBTYPE;
}

// virtual
MachLogTypeRestorer::ProcessAsSubtype
MachGuiIntelligentCursorOnActor::doProcessResourceCarrier(MachLogResourceCarrier* pActor)
{
    bool typeSpecificHandling = false;

    MachLogRaces::DispositionToRace disposition
        = MachLogRaces::instance().dispositionToRace(pActor->race(), pCursorActor_->race());
    bool myBuilding = (disposition == MachLogRaces::OUR_RACE);

    // If operand is a construction..
    if (pCursorActor_->objectIsConstruction() && pActor->isNormalResourceCarrier() && ! ctrlPressed_)
    {
        MachLog::ObjectType operandType = pCursorActor_->objectType();
        if (operandType == MachLog::MINE)
        {
            if (pCursorActor_->asMine().worthVisiting())
            {
                useCursor_ = MachGui::PICKUP_CURSOR;
                typeSpecificHandling = true;
            }
            else
                useCursor_ = MachGui::INVALID_CURSOR;
        }
        else if ((operandType == MachLog::SMELTER || operandType == MachLog::POD) && ! myBuilding)
        {
            useCursor_ = MachGui::PICKUP_CURSOR;
            typeSpecificHandling = true;
        }
        else if ((operandType == MachLog::SMELTER || operandType == MachLog::POD) && myBuilding)
        {
            useCursor_ = MachGui::TRANSPORT_CURSOR;
            typeSpecificHandling = true;
        }
    }
    else if (pCursorActor_->objectIsDebris() && pActor->isScavenger())
    {
        useCursor_ = MachGui::SCAVENGE_CURSOR;
        typeSpecificHandling = true;
    }

    // If nothing special, use standard machine logic
    if (! typeSpecificHandling)
        processStandardMachine(pActor);

    return DO_NOT_PROCESS_AS_SUBTYPE;
}

void MachGuiIntelligentCursorOnActor::processStandardMachine(MachLogMachine* pActor)
{
    // Get the operand type and see if friendly race
    MachLog::ObjectType operandType = pCursorActor_->objectType();

    MachLogRaces::DispositionToRace disposition
        = MachLogRaces::instance().dispositionToRace(pActor->race(), pCursorActor_->race());

    bool valid = (disposition == MachLogRaces::OUR_RACE);

    // Check to see if corral is full of APCs, i.e. contains some actors that can enter
    // an APC
    bool allowAPCEnter = (pInGameScreen_->corralState() & MachInGameScreen::CORRAL_SOMEMACHINES)
        && !(pInGameScreen_->corralState() & MachInGameScreen::CORRAL_ALLAPCS);

    // Check for the operand being a building
    if (pCursorActor_->objectIsConstruction())
    {
        doDefaultProcess(pActor);
    }
    else if (pCursorActor_->objectIsMachine())
    {
        // Operand is a machine.
        // If an APC we could enter it, as long as ctrl is not pressed
        if (valid && operandType == MachLog::APC && // Cursor is over an APC
            ! ctrlPressed_ && // Ctrl overrides intelligent cursors, forces select cursor
            ! altPressed_ && // Alt means we are specifying alternative cursor ( see doDefaultProcess )
            allowAPCEnter) // APCs can't enter APCs. Therefore there must be at least one other type of machine
        {
            // must check to see that at least one actor in the corral is capable of entering this APC
            bool noneCanEnter = true;
            for (MachInGameScreen::Actors::const_iterator iter = pInGameScreen_->selectedActors().begin();
                 iter != pInGameScreen_->selectedActors().end() && noneCanEnter;
                 ++iter)
            {
                if (MachGuiMoveCommand::couldEnterAPC((**iter), pCursorActor_->asAPC()))
                {
                    noneCanEnter = false;
                }
            }

            if (! noneCanEnter)
                useCursor_ = MachGui::ENTER_APC_CURSOR;
            else
                useCursor_ = MachGui::INVALID_CURSOR;
        }
        else
        {
            doDefaultProcess(pActor);
        }
    }
    else if (pCursorActor_->objectIsDebris() || pCursorActor_->objectIsOreHolograph())
    {
        if (pActor->objectType() == MachLog::GEO_LOCATOR)
        {
            useCursor_ = MachGui::LOCATETO_CURSOR;
        }
        else
        {
            useCursor_ = MachGui::MOVETO_CURSOR;
        }
    }
    else
    {
        // Unknown type - invalid action
        useCursor_ = MachGui::INVALID_CURSOR;
    }
}

// virtual
MachLogTypeRestorer::ProcessAsSubtype MachGuiIntelligentCursorOnActor::doProcessLocator(MachLogMachine*)
{
    return PROCESS_AS_SUBTYPE;
}

// virtual
MachLogTypeRestorer::ProcessAsSubtype MachGuiIntelligentCursorOnActor::doProcessTransporter(MachLogMachine*)
{
    return PROCESS_AS_SUBTYPE;
}

// Forced recompile 19/2/99 CPS
/* End INTCURAC.CPP *************************************************/
