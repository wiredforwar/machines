/*
 * D I S P M E S S . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiMessageDisplay

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_DISPMESS_HPP
#define _MACHGUI_DISPMESS_HPP

#include "base/base.hpp"
#include "phys/phys.hpp"
#include "ctl/pvector.hpp"
#include "stdlib/string.hpp"

class MachGuiMessageDisplay
// Canonical form revoked
{
public:
    //  Singleton class
    static MachGuiMessageDisplay& instance();
    ~MachGuiMessageDisplay();

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiMessageDisplay& t);
    void addMessage(const char* displayText, PhysRelativeTime timeToDisplay);
    void doOutput(std::ostream&);

private:
    MachGuiMessageDisplay(const MachGuiMessageDisplay&);
    MachGuiMessageDisplay& operator=(const MachGuiMessageDisplay&);
    bool operator==(const MachGuiMessageDisplay&);

    MachGuiMessageDisplay();
    struct DisplayMessage
    {
        std::string messageText_;
        PhysAbsoluteTime startTime_;
        PhysRelativeTime duration_;
        DisplayMessage& operator=(const DisplayMessage& rhs)
        {
            messageText_ = rhs.messageText_;
            startTime_ = rhs.startTime_;
            duration_ = rhs.duration_;
            return *this;
        }
    };

    ctl_pvector<DisplayMessage> messages_;
};

#endif

/* End DISPMESS.HPP *************************************************/
