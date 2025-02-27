/*
 * E D I T L I S T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/editlist.hpp"
#include "machgui/sslistbx.hpp"
#include "machgui/gui.hpp"
#include "gui/editbox.hpp"
#include "gui/manager.hpp"
#include "gui/painter.hpp"
#include "machgui/internal/mgsndman.hpp"

class MachGuiEditBoxItem : public GuiSingleLineEditBox
{
public:
    MachGuiEditBoxItem(
        GuiDisplayable* pParent,
        const Gui::Box& box,
        const GuiBmpFont& font,
        MachGuiSingleSelectionListBox* pListBox)
        : GuiSingleLineEditBox(pParent, box, font)
        , pListBox_(pListBox)
    {
        changed();
    }

    void drawBackground() override
    {
        if (pListBox_ && pListBox_->isFocusControl())
        {
            GuiPainter::instance().blit(
                MachGui::longYellowGlowBmp(),
                Gui::Box(0, 0, width(), height()),
                absoluteBoundary().minCorner());
        }
        else
        {
            GuiPainter::instance().blit(
                MachGui::longGlowBmp(),
                Gui::Box(0, 0, width(), height()),
                absoluteBoundary().minCorner());
        }
    }

private:
    MachGuiSingleSelectionListBox* pListBox_;
};

MachGuiEditBoxListBoxItem::MachGuiEditBoxListBoxItem(
    MachGuiStartupScreens* pStartupScreens,
    MachGuiSingleSelectionListBox* pListBox,
    size_t width,
    const std::string& text)
    : MachGuiSingleSelectionListBoxItem(pStartupScreens, pListBox, width, text)
    , pSingleLineEditBox_(nullptr)
    , maxChars_(0)
{

    TEST_INVARIANT;
}

MachGuiEditBoxListBoxItem::~MachGuiEditBoxListBoxItem()
{
    TEST_INVARIANT;

    delete pSingleLineEditBox_;
    GuiManager::instance().removeCharFocus();
}

void MachGuiEditBoxListBoxItem::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiEditBoxListBoxItem& t)
{

    o << "MachGuiEditBoxListBoxItem " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiEditBoxListBoxItem " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void MachGuiEditBoxListBoxItem::update()
{
    if (pSingleLineEditBox_)
    {
        pSingleLineEditBox_->update();
    }
}

// virtual
void MachGuiEditBoxListBoxItem::select()
{
    // Play select sound
    MachGuiSoundManager::instance().playSound("gui/sounds/listclik.wav");

    pSingleLineEditBox_
        = new MachGuiEditBoxItem(this, Gui::Box(1, 1, width() - 1, height() - 1), getUnderlineFont(), myListBox());
    pSingleLineEditBox_->caretColour(Gui::BLACK());
    pSingleLineEditBox_->maxChars(maxChars_);
    pSingleLineEditBox_->setText(text_);
    pSingleLineEditBox_->setTextChangedCallback([this](GuiSingleLineEditBox* pLineEdit) { text_ = pLineEdit->text(); });
    GuiManager::instance().charFocus(pSingleLineEditBox_);
}

// virtual
void MachGuiEditBoxListBoxItem::unselect()
{
    delete pSingleLineEditBox_;
    pSingleLineEditBox_ = nullptr;
    GuiManager::instance().removeCharFocus();
    changed();
}

GuiSingleLineEditBox* MachGuiEditBoxListBoxItem::singleLineEditBox()
{
    return pSingleLineEditBox_;
}

std::string MachGuiEditBoxListBoxItem::text() const
{
    return text_;
}

void MachGuiEditBoxListBoxItem::setText(const std::string& str)
{
    if (singleLineEditBox())
    {
        singleLineEditBox()->setText(str);
        // Existing line editor 'll set the text_ directly via callback
        return;
    }

    text_ = str;
}

size_t MachGuiEditBoxListBoxItem::maxChars() const
{
    return maxChars_;
}

void MachGuiEditBoxListBoxItem::maxChars(size_t newMaxChars)
{
    maxChars_ = newMaxChars;
    if (singleLineEditBox())
    {
        singleLineEditBox()->maxChars(maxChars_);
    }
}

/* End EDITLIST.CPP *************************************************/
