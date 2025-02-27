/*
 * C H A T M S G S . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiInGameChatMessages

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CHATMSGS_HPP
#define _MACHGUI_CHATMSGS_HPP

#include "base/base.hpp"
#include "machphys/machphys.hpp"

#include <string>
#include <vector>

template <class T> class ctl_list;
template <class T> class ctl_vector;

struct MachGuiInGameChatMessagesImpl;
class MachGuiMessageBroker;
class MachGuiStartupScreens;

class MachGuiInGameChatMessages
// Canonical form revoked
{
public:
    //  Singleton class
    static MachGuiInGameChatMessages& instance();
    ~MachGuiInGameChatMessages();

    void initialise(MachGuiMessageBroker*, MachGuiStartupScreens*);

    void CLASS_INVARIANT;

    // Call every frame. Returns true if it has been updated.
    bool update();

    void addMessage(const std::string&);

    void clearAllMessages();

    const ctl_list<std::string>& messages() const;

    void sendMessage(const std::string& message, MachPhys::Race intendedRace);
    // PRE( pMessageBroker_ );

    const std::string& playerName() const;
    MachPhys::Race playerRace() const;

    bool opponentExists(int index) const;
    std::string opponentName(int index) const;
    MachPhys::Race opponentRace(int index) const;

    static int reqWidth();
    static int reqHeight();

    // Get a list of standard messages that can be sent. This saves typing
    const std::vector<std::string>& standardMessages() const;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiInGameChatMessages& t);

    MachGuiInGameChatMessages(const MachGuiInGameChatMessages&);
    MachGuiInGameChatMessages& operator=(const MachGuiInGameChatMessages&);

    MachGuiInGameChatMessages();

    // Data members...
    MachGuiInGameChatMessagesImpl* pImpl_;
};

#endif

/* End CHATMSGS.HPP *************************************************/
