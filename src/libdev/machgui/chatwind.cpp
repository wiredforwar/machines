/*
 * C H A T W I N D . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "stdlib/string.hpp"
#include "machgui/chatwind.hpp"
#include "machgui/startup.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "machgui/ui/MenuText.hpp"
#include "gui/font.hpp"
#include "gui/painter.hpp"
#include "machgui/menus_helper.hpp"

MachGuiChatWindow::MachGuiChatWindow(GuiRoot* pRootParent, MachGuiStartupScreens* pStartupScreens, const Gui::Box& box)
    : GuiDisplayable(pStartupScreens, box)
    , pRootParent_(pRootParent)
    , pStartupScreens_(pStartupScreens)
{
    GuiBmpFont font(GuiBmpFont::getFont(MachGui::Menu::smallFontLight()));
    // Work out how may lines we can display in the chat window
    numLines() = height() / (font.charHeight() + 1.0 /*spacing*/);

    chatWidth() = width();

    chatWindow() = this;

    TEST_INVARIANT;
}

MachGuiChatWindow::~MachGuiChatWindow()
{
    TEST_INVARIANT;

    chatWindow() = nullptr;
}

// static
size_t& MachGuiChatWindow::numLines()
{
    static size_t numLines = 1;
    return numLines;
}

// static
MachGuiChatWindow::strings& MachGuiChatWindow::linesOfText()
{
    static strings lines;
    return lines;
}

// static
size_t& MachGuiChatWindow::chatWidth()
{
    static size_t width = 315;
    return width;
}

// static
void MachGuiChatWindow::clearAllText()
{
    linesOfText().erase(linesOfText().begin(), linesOfText().end());
}

void MachGuiChatWindow::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiChatWindow& t)
{

    o << "MachGuiChatWindow " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiChatWindow " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiChatWindow::doDisplay()
{
    GuiBmpFont font(GuiBmpFont::getFont(MachGui::Menu::smallFontLight()));

    size_t startY = absoluteBoundary().maxCorner().y();
    size_t loop = linesOfText().size();

    // Display each line of text, starting from the bottom of the GuiDisplayable
    while (loop)
    {
        --loop;
        startY -= font.charHeight() + 1 * Gui::uiScaleFactor();
        font.drawText(linesOfText()[loop], Gui::Coord(absoluteBoundary().minCorner().x(), startY), width());
    }
}

// static
void MachGuiChatWindow::addText(const std::string& text)
{
    MachGuiMenuText::chopUpText(text, chatWidth(), GuiBmpFont::getFont(MachGui::Menu::smallFontLight()), &linesOfText());

    while (linesOfText().size() > numLines())
    {
        linesOfText().erase(linesOfText().begin());
    }

    if (chatWindow())
    {
        // Don't update if message box is filling screen
        if (! chatWindow()->pStartupScreens_->msgBoxIsBeingDisplayed())
        {
            chatWindow()->changed();
        }
    }
}

// static
MachGuiChatWindow*& MachGuiChatWindow::chatWindow()
{
    static MachGuiChatWindow* pChatWindow = nullptr;
    return pChatWindow;
}

/* End CHATWIND.CPP *************************************************/
