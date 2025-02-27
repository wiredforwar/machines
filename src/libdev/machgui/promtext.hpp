/*
 * P R O M T E X T . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachPromptText

    Implements the prompt text displayed in the prompt text area of the in-game screen.
*/

#ifndef _MACHGUI_PROMTEXT_HPP
#define _MACHGUI_PROMTEXT_HPP

#include "base/base.hpp"
#include "stdlib/string.hpp"
#include "gui/displaya.hpp"
#include "gui/gui.hpp"
#include "gui/editbox.hpp"

class MachCameras;
class MachPromptTextImpl;
class MachInGameScreen;

// orthodox canonical (revoked)
class MachPromptText : public GuiSingleLineEditBox
{
public:
    // ctor. Owner is pParent. Area covered is bounded by relativeBoundary.
    MachPromptText(
        MachInGameScreen* pParent,
        const Gui::Boundary& relativeBoundary,
        MachCameras*,
        GuiDisplayable* pPassEventsTo);

    // dtor.
    ~MachPromptText() override;

    // Set/get the prompt text related to cursor position
    void setCursorPromptText(const std::string& prompt);
    void setCursorPromptText(const std::string& prompt, bool restartScroll);
    const std::string& cursorPromptText() const;

    // Clear any prompt text associated with the cursor position
    void clearCursorPromptText();

    // Set/get the prompt text related to current command state
    void setCommandPromptText(const std::string& prompt);
    const std::string& commandPromptText() const;

    // Clear any prompt text associated with the current command state
    void clearCommandPromptText();

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachPromptText& t);

    enum PromptDisplayed
    {
        CURSORTEXT,
        COMMANDTEXT,
        NOTEXT
    };

    bool doHandleKeyEvent(const GuiKeyEvent& event) override;

protected:
    // Inherited from GuiDisplayable...
    void doDisplay() override;

    // virtual void doHandleContainsMouseEvent( const GuiMouseEvent& rel );
    // virtual void doHandleMouseClickEvent( const GuiMouseEvent& rel );
    // virtual void doHandleMouseEnterEvent( const GuiMouseEvent& rel );
    // virtual void doHandleMouseExitEvent( const GuiMouseEvent& rel );

    bool doHandleCharEvent(const GuiCharEvent& event) override;

    void displayChatMessage();
    void displayPromptText(PromptDisplayed textType, const std::vector<std::string>& textLines);

    int maxWidth() const override;
    Gui::Coord getPromptTextAbsolutePosition() const;

    // Pass mouse position in as relative to this GuiDisplayable. Updates
    // mouse position to be relative to pPassEventsTo_.
    // bool passEventTo( GuiMouseEvent* );

    bool processesMouseEvents() const override;

private:
    // Operations deliberately revoked
    MachPromptText(const MachPromptText&);
    MachPromptText& operator=(const MachPromptText&);
    bool operator==(const MachPromptText&);

    // Data members...
    MachPromptTextImpl* pImpl_;
};

#endif

/* End PROMTEXT.HPP *************************************************/
