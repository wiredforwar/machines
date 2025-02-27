/*
 * C M D D E F C N . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "cmddefcn.hpp"

#include "mathex/point3d.hpp"
#include "machgui/ingame.hpp"
#include "machgui/internal/strings.hpp"
#include "world4d/domain.hpp"
#include "machlog/actor.hpp"
#include "machlog/machine.hpp"
#include "system/pathname.hpp"
#include "device/butevent.hpp"

// Statics
bool MachGuiDefconCommand::defconLow_ = false;
bool MachGuiDefconCommand::defconNormal_ = false;
bool MachGuiDefconCommand::defconHigh_ = false;
bool MachGuiDefconCommand::goHighFromNormal_ = true;

MachGuiDefconCommand::MachGuiDefconCommand(MachInGameScreen* pInGameScreen)
    : MachGuiCommand(pInGameScreen)
{
    TEST_INVARIANT;
}

MachGuiDefconCommand::~MachGuiDefconCommand()
{
    TEST_INVARIANT;

    inGameScreen().setCursorFilter(W4dDomain::EXCLUDE_NOT_SOLID);
}

void MachGuiDefconCommand::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiDefconCommand& t)
{

    o << "MachGuiDefconCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiDefconCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachGuiDefconCommand::canActorEverExecute(const MachActor& actor) const
{
    // Machines can have defcon set
    return actor.objectIsMachine();
}

// virtual
bool MachGuiDefconCommand::isInteractionComplete() const
{
    return true;
}

// virtual
MachGui::Cursor2dType
MachGuiDefconCommand::cursorOnTerrain(const MexPoint3d& /*location*/, bool /*ctrlPressed*/, bool, bool)
{
    return MachGui::INVALID_CURSOR;
}

// virtual
MachGui::Cursor2dType MachGuiDefconCommand::cursorOnActor(MachActor* /*pActor*/, bool, bool, bool)
{
    return MachGui::INVALID_CURSOR;
}

// virtal
void MachGuiDefconCommand::typeData(MachLog::ObjectType /*objectType*/, int /*subType*/, uint /*level*/)
{
}

// virtual
bool MachGuiDefconCommand::doApply(MachActor* pActor, std::string*)
{
    MachLog::DefCon newDefcon = MachLog::DEFCON_NORMAL;

    if (defconLow_ && defconNormal_ && defconHigh_)
        newDefcon = MachLog::DEFCON_HIGH;
    else if (defconLow_ && defconNormal_)
        newDefcon = MachLog::DEFCON_NORMAL;
    else if (defconNormal_ && defconHigh_)
        newDefcon = MachLog::DEFCON_HIGH;
    else if (defconLow_ && defconHigh_)
        newDefcon = MachLog::DEFCON_HIGH;
    else if (defconLow_)
        newDefcon = MachLog::DEFCON_NORMAL;
    else if (defconNormal_)
    {
        if (goHighFromNormal_)
        {
            newDefcon = MachLog::DEFCON_HIGH;
        }
        else
        {
            newDefcon = MachLog::DEFCON_LOW;
        }
    }

    DEBUG_STREAM(DIAG_NEIL, "MachGuiDefconCommand::doApply " << newDefcon << std::endl);

    pActor->asMachine().defCon(newDefcon);

    return true;
}

// virtual
std::unique_ptr<MachGuiCommand> MachGuiDefconCommand::clone() const
{
    return std::make_unique<MachGuiDefconCommand>(&inGameScreen());
}

// virtual
const std::pair<std::string, std::string>& MachGuiDefconCommand::iconNames() const
{
    static std::pair<std::string, std::string> names("gui/commands/defco1.bmp", "gui/commands/defco1.bmp");
    return names;
}

// virtual
uint MachGuiDefconCommand::cursorPromptStringId() const
{
    if (defconLow_)
        return IDS_DEFCON3_COMMAND;
    else if (defconNormal_)
        return IDS_DEFCON2_COMMAND;

    return IDS_DEFCON1_COMMAND;
}

// virtual
uint MachGuiDefconCommand::commandPromptStringid() const
{
    if (defconLow_)
        return IDS_DEFCON3_COMMAND;
    else if (defconNormal_)
        return IDS_DEFCON2_COMMAND;

    return IDS_DEFCON1_COMMAND;
}

// virtual
bool MachGuiDefconCommand::canAdminApply() const
{
    return false;
}

// virtual
bool MachGuiDefconCommand::doAdminApply(MachLogAdministrator* /*pAdministrator*/, std::string*)
{
    PRE(canAdminApply());

    return false;
}

void MachGuiDefconCommand::update(const Actors& actors)
{
    // Work out which lights are on
    defconLow_ = false;
    defconNormal_ = false;
    defconHigh_ = false;

    for (Actors::const_iterator iter = actors.begin(); iter != actors.end(); ++iter)
    {
        if ((*iter)->objectIsMachine()) // Defcon only applies to machines
        {
            switch ((*iter)->asMachine().defCon())
            {
                case MachLog::DEFCON_HIGH:
                    defconHigh_ = true;
                    break;
                case MachLog::DEFCON_NORMAL:
                    defconNormal_ = true;
                    break;
                case MachLog::DEFCON_LOW:
                    defconLow_ = true;
                    break;
                    DEFAULT_ASSERT_BAD_CASE((*iter)->asMachine().defCon());
            }
        }
    }

    // now may need to alter the goHighFromNormal_ movement direction flag
    if (defconLow_ && ! defconNormal_ && ! defconHigh_)
    {
        goHighFromNormal_ = true;
    }
    else if (defconHigh_ && ! defconNormal_ && ! defconLow_)
    {
        goHighFromNormal_ = false;
    }

    DEBUG_STREAM(
        DIAG_NEIL,
        "MachGuiDefconCommand::update " << defconLow_ << " " << defconNormal_ << " " << defconHigh_ << std::endl);
}

// static
bool MachGuiDefconCommand::defconLow()
{
    return defconLow_;
}

// static
bool MachGuiDefconCommand::defconNormal()
{
    return defconNormal_;
}

// static
bool MachGuiDefconCommand::defconHigh()
{
    return defconHigh_;
}

// virtual
bool MachGuiDefconCommand::processButtonEvent(const DevButtonEvent& be)
{
    if (isVisible() && be.scanCode() == Device::KeyCode::TAB && be.action() == DevButtonEvent::PRESS && be.previous() == 0)
    {
        inGameScreen().activeCommand(*this);
        return true;
    }

    return false;
}

void MachGuiDefconCommand::resetDirectionFromDefConNormal()
{
    goHighFromNormal_ = true;
}

/* End CMDDEFCN.CPP **************************************************/
