/*
 * C M D T R A N S . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "cmddestr.hpp"

#include "machgui/ingame.hpp"
#include "device/time.hpp"
#include "machgui/internal/strings.hpp"
#include "machlog/actor.hpp"
#include "machlog/machvman.hpp"
#include "device/butevent.hpp"

MachGuiSelfDestructCommand::MachGuiSelfDestructCommand(MachInGameScreen* pInGameScreen)
    : MachGuiDefaultCommand(pInGameScreen)
{
    TEST_INVARIANT;
}

MachGuiSelfDestructCommand::~MachGuiSelfDestructCommand()
{
    TEST_INVARIANT;
}

void MachGuiSelfDestructCommand::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiSelfDestructCommand& t)
{

    o << "MachGuiSelfDestructCommand " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiSelfDestructCommand " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachGuiSelfDestructCommand::canActorEverExecute(const MachActor& actor) const
{
    // All Machines and Constructions can SelfDestruct
    return actor.objectIsMachine() || actor.objectIsConstruction();
}

// virtual
bool MachGuiSelfDestructCommand::isInteractionComplete() const
{
    return hadFinalPick();
}

// virtual
bool MachGuiSelfDestructCommand::doApply(MachActor* pActor, std::string*)
{
    PRE(pActor->objectIsMachine() || pActor->objectIsConstruction());

    if (buttonState() == BTN_SHOWN)
    {
        if (! hasPlayedVoiceMail() && pActor->objectIsMachine())
        {
            MachLogMachineVoiceMailManager::instance().postNewMail(*pActor, MachineVoiceMailEventID::SELF_DESTRUCT);
            hasPlayedVoiceMail(true);
        }

        // Start self-destruct
        pActor->selfDestruct(5.0);

        if (pActor == inGameScreen().selectedActors().back())
        {
            privateHadFinalPick() = false;
            privateButtonState() = BTN_ACTIVE;
            MachGuiSelfDestructCommandIcon::refreshAllIcons();
        }
    }
    else
    {
        // Cancel self-destruct
        pActor->selfDestruct(0.0);

        if (pActor == inGameScreen().selectedActors().back())
        {
            privateHadFinalPick() = false;
            privateButtonState() = BTN_SHOWN;
            MachGuiSelfDestructCommandIcon::refreshAllIcons();
        }
    }

    return true;
}

// virtual
void MachGuiSelfDestructCommand::typeData(MachLog::ObjectType, int, uint)
{
    // Do nothing
}

// virtual
std::unique_ptr<MachGuiCommand> MachGuiSelfDestructCommand::clone() const
{
    if (buttonState() == BTN_SHOWN || buttonState() == BTN_ACTIVE)
    {
        privateHadFinalPick() = true;
        MachGuiSelfDestructCommandIcon::refreshAllIcons();
    }
    else
    {
        privateButtonState() = BTN_SHOWN;
        MachGuiSelfDestructCommandIcon::refreshAllIcons();
    }

    return std::make_unique<MachGuiSelfDestructCommand>(&inGameScreen());
}

// virtual
const std::pair<std::string, std::string>& MachGuiSelfDestructCommand::iconNames() const
{
    static std::pair<std::string, std::string> names("gui/commands/selfdes1.bmp", "gui/commands/selfdes1.bmp");
    return names;
}

// virtual
uint MachGuiSelfDestructCommand::cursorPromptStringId() const
{
    return IDS_SELF_DESTRUCT_COMMAND;
}

// virtual
uint MachGuiSelfDestructCommand::commandPromptStringid() const
{
    return IDS_SELF_DESTRUCT_START;
}

// virtual
bool MachGuiSelfDestructCommand::processButtonEvent(const DevButtonEvent& be)
{
    if (isVisible() && be.scanCode() == Device::KeyCode::BACK_SPACE && be.wasCtrlPressed() && be.wasShiftPressed()
        && be.action() == DevButtonEvent::PRESS && be.previous() == 0)
    {
        inGameScreen().activeCommand(*this);
        return true;
    }

    return false;
}

// static
MachGuiSelfDestructCommand::ButtonState& MachGuiSelfDestructCommand::privateButtonState()
{
    static ButtonState state = BTN_HIDDEN;
    return state;
}

// static
bool& MachGuiSelfDestructCommand::privateHadFinalPick()
{
    static bool finalPick = false;
    return finalPick;
}

// static
MachGuiSelfDestructCommand::ButtonState MachGuiSelfDestructCommand::buttonState()
{
    return privateButtonState();
}

// static
bool MachGuiSelfDestructCommand::hadFinalPick()
{
    return privateHadFinalPick();
}

// static
void MachGuiSelfDestructCommand::resetButtonState()
{
    privateHadFinalPick() = false;
    privateButtonState() = BTN_HIDDEN;
    privateRotateFrame() = 0;
    privateRotateFrameTime() = DevTime::instance().time();
    MachGuiSelfDestructCommandIcon::refreshAllIcons();
}

// static
void MachGuiSelfDestructCommand::stopPressed()
{
    if (buttonState() == BTN_ACTIVE)
    {
        privateHadFinalPick() = false;
        privateButtonState() = BTN_SHOWN;
        privateRotateFrame() = 7;
        MachGuiSelfDestructCommandIcon::refreshAllIcons();
    }
}

// static
int& MachGuiSelfDestructCommand::privateRotateFrame()
{
    static int rotateFrame = 0;
    return rotateFrame;
}

// static
double& MachGuiSelfDestructCommand::privateRotateFrameTime()
{
    static double timer = DevTime::instance().time();
    return timer;
}

// static
void MachGuiSelfDestructCommand::update(const Actors& actors)
{
    for (Actors::const_iterator iter = actors.begin(); iter != actors.end(); ++iter)
    {
        MachActor& actor = *(*iter);

        if ((actor.objectIsMachine() || actor.objectIsConstruction()) && actor.selfDestruct() == true)
        {
            privateButtonState() = BTN_ACTIVE;
            MachGuiSelfDestructCommand::privateRotateFrame() = 7;
        }
    }
    MachGuiSelfDestructCommandIcon::refreshAllIcons();
}

MachGuiSelfDestructCommandIcon::MachGuiSelfDestructCommandIcon(
    GuiDisplayable* pParent,
    const Gui::Coord& rel,
    const MachGuiCommand& command,
    MachInGameScreen* pInGameScreen)
    : MachCommandIcon(pParent, rel, command, pInGameScreen)
{
    icons().push_back(this);
}

MachGuiSelfDestructCommandIcon::~MachGuiSelfDestructCommandIcon()
{
    // TBD : remove from list
}

// virtual
const GuiBitmap& MachGuiSelfDestructCommandIcon::getBitmap() const
{
    PRE(isVisible());

    static GuiBitmap retVal;
    const char* pBitmapPath = nullptr;
    // GuiDisplayable* pThis = _CONST_CAST( GuiDisplayable*, this );
    GuiDisplayable* pThis = _STATIC_CAST(GuiDisplayable*, _CONST_CAST(MachGuiSelfDestructCommandIcon*, this));

    // Display hidden self-destruct button
    if (MachGuiSelfDestructCommand::buttonState() == MachGuiSelfDestructCommand::BTN_HIDDEN)
    {
        pThis->redrawEveryFrame(false);
        pBitmapPath = "gui/commands/selfdes1.bmp";
    }
    // Display rotating self-destruct button
    else if (MachGuiSelfDestructCommand::buttonState() == MachGuiSelfDestructCommand::BTN_SHOWN)
    {
        // Work-ou if it is time to switch to the next frame of the rotation animation
        int& rotateFrame = MachGuiSelfDestructCommand::privateRotateFrame();
        if (rotateFrame != 7)
        {
            double currentTime = DevTime::instance().time();
            double timeDiff = currentTime - MachGuiSelfDestructCommand::privateRotateFrameTime();

            if (timeDiff > 0.05 || rotateFrame == 0)
            {
                ++rotateFrame;
                MachGuiSelfDestructCommand::privateRotateFrameTime() = currentTime;
            }
            pThis->redrawEveryFrame(true);
        }
        // Display relevant frame

        switch (rotateFrame)
        {
            case 1:
                pBitmapPath = "gui/commands/selfdes2.bmp";
                break;
            case 2:
                pBitmapPath = "gui/commands/selfdes3.bmp";
                break;
            case 3:
                pBitmapPath = "gui/commands/selfdes4.bmp";
                break;
            case 4:
                pBitmapPath = "gui/commands/selfdes5.bmp";
                break;
            case 5:
                pBitmapPath = "gui/commands/selfdes6.bmp";
                break;
            case 6:
                pBitmapPath = "gui/commands/selfdes7.bmp";
                break;
            case 7:
                pBitmapPath = "gui/commands/selfdes7.bmp";
                // Stop redraws
                pThis->redrawEveryFrame(false);
                break;
        }
    }
    // Display flashing self-destruct button
    else if (MachGuiSelfDestructCommand::buttonState() == MachGuiSelfDestructCommand::BTN_ACTIVE)
    {
        pThis->redrawEveryFrame(true);

        double time = DevTime::instance().time();
        int itime = time * 4;

        if (itime % 2)
        {
            pBitmapPath = "gui/commands/selfdes8.bmp";
        }
        else
        {
            pBitmapPath = "gui/commands/selfdes7.bmp";
        }
    }

    retVal = MachGui::getScaledImage(pBitmapPath);

    return retVal;
}

// static
MachGuiSelfDestructCommandIcon::MachGuiSelfDestructCommandIcons& MachGuiSelfDestructCommandIcon::icons()
{
    static MachGuiSelfDestructCommandIcons icons;
    static bool firstTime = true;
    if (firstTime)
    {
        firstTime = false;
        icons.reserve(8);
    }

    return icons;
}

// static
void MachGuiSelfDestructCommandIcon::refreshAllIcons()
{
    for (MachGuiSelfDestructCommandIcons::iterator iter = icons().begin(); iter != icons().end(); ++iter)
    {
        (*iter)->changed();
    }
}

// virtual
bool MachGuiSelfDestructCommand::actorsCanExecute() const
{
    return MachGuiCommand::actorsCanExecute();
}

/* End CMDDESTR.CPP **************************************************/
