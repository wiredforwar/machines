/*
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "cmdresea.hpp"

#include "machgui/ingame.hpp"
#include "machgui/internal/strings.hpp"
#include "device/butevent.hpp"
#include "machlog/actor.hpp"
#include "machlog/machlog.hpp"
#include "ctl/pvector.hpp"

MachGuiResearchCommand::MachGuiResearchCommand(MachInGameScreen* pInGameScreen)
    : MachGuiCommand(pInGameScreen)
{
    TEST_INVARIANT;
}

MachGuiResearchCommand::~MachGuiResearchCommand()
{
    TEST_INVARIANT;
}

void MachGuiResearchCommand::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiResearchCommand& t)
{

    o << "MachGuiResearchCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiResearchCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiResearchCommand::pickOnActor(MachActor* pActor, bool ctrlPressed, bool, bool)
{
    // Usual selection of the actor
    if (! ctrlPressed)
        inGameScreen().deselectAll();

    inGameScreen().select(pActor);
}

// virtual
bool MachGuiResearchCommand::canActorEverExecute(const MachActor& actor) const
{
    // Factories can build
    MachLog::ObjectType objectType = actor.objectType();
    return objectType == MachLog::HARDWARE_LAB;
}

// virtual
bool MachGuiResearchCommand::isInteractionComplete() const
{
    // Is complete if the selection set no longer consists of single hwlab
    return ! actorsCanExecute();
}

// virtual
bool MachGuiResearchCommand::doApply(MachActor*, std::string*)
{
    // This command interaction is all via the menu icons, so this function is not called
    return true;
}

// virtual
MachGui::Cursor2dType MachGuiResearchCommand::cursorOnTerrain(const MexPoint3d&, bool, bool, bool)
{
    MachGui::Cursor2dType cursor = MachGui::INVALID_CURSOR;

    return cursor;
}

// virtual
MachGui::Cursor2dType MachGuiResearchCommand::cursorOnActor(MachActor*, bool, bool, bool)
{
    MachGui::Cursor2dType cursor = MachGui::SELECT_CURSOR;

    return cursor;
}

// virtual
void MachGuiResearchCommand::typeData(MachLog::ObjectType, int, uint)
{
    // Do nothing
}

// virtual
std::unique_ptr<MachGuiCommand> MachGuiResearchCommand::clone() const
{
    return std::make_unique<MachGuiResearchCommand>(&inGameScreen());
}

// virtual
const std::pair<std::string, std::string>& MachGuiResearchCommand::iconNames() const
{
    static std::pair<std::string, std::string> names("gui/commands/research.bmp", "gui/commands/research.bmp");
    return names;
}

// virtual
bool MachGuiResearchCommand::actorsCanExecute() const
{
    // Check there is exactly one selected entity, and it is a factory
    const MachInGameScreen::Actors& selectionSet = inGameScreen().selectedActors();
    return selectionSet.size() == 1 && selectionSet.front()->objectType() == MachLog::HARDWARE_LAB;
}

void MachGuiResearchCommand::start()
{
    MachGuiCommand::start();
    // Cache the current control panel context
    previousMenuContext_ = inGameScreen().currentContext();

    // Enter the hw research menu context
    inGameScreen().hardwareResearchContext();
}

void MachGuiResearchCommand::finish()
{
    MachGuiCommand::finish();
    // If still in the build command context, restore the cached context
    if (inGameScreen().isHardwareResearchContext())
        inGameScreen().currentContext(previousMenuContext_);
}

// virtual
uint MachGuiResearchCommand::cursorPromptStringId() const
{
    return IDS_RESEARCHLEVEL_COMMAND;
}

// virtual
uint MachGuiResearchCommand::commandPromptStringid() const
{
    return IDS_RESEARCHLEVEL_START;
}

// virtual
bool MachGuiResearchCommand::processButtonEvent(const DevButtonEvent& be)
{
    // if ( isVisible() and be.scanCode() == Device::KeyCode::BACK_SPACE and be.action() == DevButtonEvent::PRESS )
    //{
    //   inGameScreen().activeCommand( *this );
    //   return true;
    // }

    return false;
}

/* End  **************************************************/
