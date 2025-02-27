#include "machgui/MachGuiFPCommand.hpp"

#include "system/pathname.hpp"
#include "gui/painter.hpp"
#include "gui/font.hpp"
#include "machlog/machine.hpp"
#include "machgui/gui.hpp"
#include "world4d/manager.hpp"
#include "world4d/scenemgr.hpp"
#include "render/device.hpp"
#include "machlog/MachLog1stPersonActiveSquad.hpp"

// Helpers ////////////////////////////////////////////////////////////////////////////////////////

// This will create the number text only once.
template <int64_t NUM> static RenSurface* createNumberText(const bool showText = true)
{
    // createNumberText<1>(), for example, would always give us a pointer to the same item
    static RenSurface surfaceNUM
        = RenSurface::createAnonymousSurface(32, 32, W4dManager::instance().sceneManager()->pDevice()->backSurface());
    static bool initializedNUM = false;

    if (! initializedNUM)
    {
        constexpr auto rectangle = Ren::Rect { 0, 0, 32, 32 };

        surfaceNUM.enableColourKeying();
        surfaceNUM.filledRectangle(rectangle, Gui::MAGENTA());

        if (showText)
        {
            // RenSurface drawText doesn't work... :o
            auto font = GuiBmpFont { GuiBmpFont::getFont(Gui::getScaledImagePath("gui/menu/largyfnt.bmp")) };
            font.drawText(&surfaceNUM, std::to_string(NUM), Gui::Coord(0, 0), 32);
        }

        initializedNUM = true;
    }

    return &surfaceNUM;
}

//-/////////////////////////////////////////////////////////////////////////////////////////////////

MachGuiFPCommand::MachGuiFPCommand(GuiDisplayable* pParent, const Gui::Coord& relPos)
    : GuiDisplayable(pParent, Gui::Box(relPos, 1, 1))
{
    pActiveSquadIcon_ = &noSquadronSelected();
    activeSquadNumber_ = 0;

    attackCommandState_ = CommandIconState::INVALID;
    followCommandState_ = CommandIconState::INVALID;
    moveCommandState_ = CommandIconState::INVALID;

    mapSquadNumbers_[0] = createNumberText<-1>(false); // No Squad
    mapSquadNumbers_[1] = createNumberText<0>(); // Squadron Corral Icon: 0
    mapSquadNumbers_[2] = createNumberText<1>(); // Squadron Corral Icon: 1
    mapSquadNumbers_[3] = createNumberText<2>(); // Squadron Corral Icon: 2
    mapSquadNumbers_[4] = createNumberText<3>(); // Squadron Corral Icon: 3
    mapSquadNumbers_[5] = createNumberText<4>(); // Squadron Corral Icon: 4
    mapSquadNumbers_[6] = createNumberText<5>(); // Squadron Corral Icon: 5
    mapSquadNumbers_[7] = createNumberText<6>(); // Squadron Corral Icon: 6
    mapSquadNumbers_[8] = createNumberText<7>(); // Squadron Corral Icon: 7
    mapSquadNumbers_[9] = createNumberText<8>(); // Squadron Corral Icon: 8
    mapSquadNumbers_[10] = createNumberText<9>(); // Squadron Corral Icon: 9
}

MachGuiFPCommand::~MachGuiFPCommand()
{
}

void MachGuiFPCommand::logHandler(MachLog1stPersonHandler* logHandler)
{
    pLogHandler_ = logHandler;
}

void MachGuiFPCommand::resetLogHandler()
{
    pLogHandler_ = nullptr;
}

void MachGuiFPCommand::updateSquadIcon()
{
    if (!pLogHandler_)
    {
        return;
    }

    MachLogMachine* squadLeader = pLogHandler_->getActiveSquadron().getLeadingMachine();

    if (squadLeader)
    {
        activeSquadIcon_ = MachGui::createIconForMachine(squadLeader, true);
        pActiveSquadIcon_ = &activeSquadIcon_;
        return;
    }

    // No squad leader? no squad!
    pActiveSquadIcon_ = &noSquadronSelected();
}

void MachGuiFPCommand::clearSquadIcon()
{
    pActiveSquadIcon_ = &noSquadronSelected();
}

void MachGuiFPCommand::updateSquadNumber()
{
    if (!pLogHandler_)
    {
        return;
    }

    activeSquadNumber_ = pLogHandler_->getActiveSquadron().getActiveSquadronId();
}

void MachGuiFPCommand::resetSquadNumber()
{
    activeSquadNumber_ = 0;
}

void MachGuiFPCommand::setAttackIconState(CommandIconState state)
{
    attackCommandState_ = state;
}
void MachGuiFPCommand::setFollowIconState(CommandIconState state)
{
    followCommandState_ = state;
}
void MachGuiFPCommand::setMoveIconState(CommandIconState state)
{
    moveCommandState_ = state;
}

// virtual
void MachGuiFPCommand::doDisplay()
{
    // Display widget body, active squad icon, etc...
    const auto topLeft = absoluteBoundary().topLeft();

    GuiPainter::instance().blit(widgetBody(), topLeft);
    GuiPainter::instance().blit(*pActiveSquadIcon_, topLeft + Gui::Coord(43, 50) * Gui::uiScaleFactor());
    GuiPainter::instance().blit(
        *mapSquadNumbers_[activeSquadNumber_],
        topLeft + Gui::Coord(64, 20) * Gui::uiScaleFactor());

    // widget.bmp: 130x130
    // the command icons: 64x24
    int state = static_cast<int>(moveCommandState_);
    GuiPainter::instance().blit(moveCommandIcons()[state], topLeft + Gui::Coord(0, 130) * Gui::uiScaleFactor());
    state = static_cast<int>(followCommandState_);
    GuiPainter::instance().blit(followCommandIcons()[state], topLeft + Gui::Coord(66, 130) * Gui::uiScaleFactor());
    state = static_cast<int>(attackCommandState_);
    GuiPainter::instance().blit(
        attackCommandIcons()[state],
        topLeft + Gui::Coord(130 / 2 - 64 / 2, 130 + 24) * Gui::uiScaleFactor());
}

// static
GuiBitmap& MachGuiFPCommand::noSquadronSelected()
{
    static GuiBitmap blankIcon = Gui::getScaledImage("gui/fstpersn/machines/backgrnd.bmp");
    return blankIcon;
}

// static
GuiBitmap& MachGuiFPCommand::widgetBody()
{
    static GuiBitmap widget = Gui::getScaledImage("gui/fstpersn/command/widget.bmp");
    return widget;
}

// static
GuiBitmap* MachGuiFPCommand::attackCommandIcons()
{
    static GuiBitmap attackIcons[NumCommandIconStates] = {
        /* INVALID   */ Gui::getScaledImage("gui/fstpersn/command/attack_invalid.bmp"),
        /* VALID     */ Gui::getScaledImage("gui/fstpersn/command/attack_valid.bmp"),
        /* ACTIVATED */ Gui::getScaledImage("gui/fstpersn/command/attack_activate.bmp"),
    };

    return attackIcons;
}

// static
GuiBitmap* MachGuiFPCommand::followCommandIcons()
{
    static GuiBitmap followIcons[NumCommandIconStates] = {
        /* INVALID   */ Gui::getScaledImage("gui/fstpersn/command/follow_invalid.bmp"),
        /* VALID     */ Gui::getScaledImage("gui/fstpersn/command/follow_valid.bmp"),
        /* ACTIVATED */ Gui::getScaledImage("gui/fstpersn/command/follow_activate.bmp"),
    };

    return followIcons;
}

// static
GuiBitmap* MachGuiFPCommand::moveCommandIcons()
{
    static GuiBitmap moveIcons[NumCommandIconStates] = {
        /* INVALID   */ Gui::getScaledImage("gui/fstpersn/command/move_invalid.bmp"),
        /* VALID     */ Gui::getScaledImage("gui/fstpersn/command/move_valid.bmp"),
        /* ACTIVATED */ Gui::getScaledImage("gui/fstpersn/command/move_activate.bmp"),
    };

    return moveIcons;
}
