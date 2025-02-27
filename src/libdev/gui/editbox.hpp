/*
 * E D I T B O X . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    GuiSingleLineEditBox

    A brief description of the class should go in here
*/

#ifndef _GUI_EDITBOX_HPP
#define _GUI_EDITBOX_HPP

#include "base/base.hpp"
#include "stdlib/string.hpp"
#include "gui/displaya.hpp"
#include "gui/font.hpp"
#include "gui/gui.hpp"

class GuiSingleLineEditBox : public GuiDisplayable
// Canonical form revoked
{
public:
    using Callback = std::function<void(GuiSingleLineEditBox*)>;

    GuiSingleLineEditBox(GuiDisplayable* pParent, const Gui::Box& box);
    GuiSingleLineEditBox(GuiDisplayable* pParent, const Gui::Box& box, const GuiBmpFont& font);
    ~GuiSingleLineEditBox() override;

    // Sets the maximum number of characters that can be typed into the edit box.
    // zero = no limit
    void maxChars(size_t maxChars);

    // Call every frame. Controls the flashing of the caret etc.
    void update();

    void backgroundColour(const GuiColour& colour);
    void caretColour(const GuiColour& colour);

    // Set text that is displayed in edit box
    void setText(const std::string&);
    // PRE( std::string.length() <= maxChars_ );
    std::string text() const;
    void setTextChangedCallback(Callback callback);

    // Turn edit box border on.
    void border(bool);

    // Select edit box border colour ( default is white ).
    void borderColour(const GuiColour& colour);

    void CLASS_INVARIANT;

protected:
    void doDisplay() override;
    bool doHandleCharEvent(const GuiCharEvent& e) override;
    bool doHandleKeyEvent(const GuiKeyEvent& e) override;
    void doHandleMouseClickEvent(const GuiMouseEvent& e) override;

    void leftArrowEvent();
    void rightArrowEvent();
    void homeEvent();
    void endEvent();
    void backspaceEvent();
    void deleteEvent();

    void onTextChanged();
    void forceRedraw();

    virtual void drawBackground();

    const std::string& leftText() const;
    const std::string& rightText() const;
    Gui::XCoord caretPos() const;
    bool showCaret() const;
    const GuiColour& caretColour() const;

    virtual int maxWidth() const;

protected:
    friend std::ostream& operator<<(std::ostream& o, const GuiSingleLineEditBox& t);

    GuiSingleLineEditBox(const GuiSingleLineEditBox&);
    GuiSingleLineEditBox& operator=(const GuiSingleLineEditBox&);

    Callback textChangedCallback_{};

    // Data members...
    std::string rightText_;
    std::string leftText_;
    Gui::XCoord caretPos_ = 0;
    GuiBmpFont font_;
    bool showCaret_;
    size_t maxChars_ = 0;
    GuiColour backgroundColour_;
    GuiColour caretColour_;
    bool dontUpdate_;
    bool border_ = false;
    GuiColour borderColour_;
};

#endif

/* End EDITBOX.HPP **************************************************/
