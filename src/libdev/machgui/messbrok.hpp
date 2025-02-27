/*
 * M E S S B R O K . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiMessageBroker

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_MESSBROK_HPP
#define _MACHGUI_MESSBROK_HPP

#include "base/base.hpp"
#include "mathex/point2d.hpp"
#include "machlog/messbrok.hpp"

struct MachGuiChatMessage;
struct MachGuiJoinMessage;
struct MachGuiUpdatePlayersMessage;
struct MachGuiUpdateGameSettingsMessage;
struct MachGuiRaceChangeRequestMessage;
struct MachGuiImReadyMessage;
struct MachGuiStartMessage;
struct MachGuiHostCancelMessage;
struct MachGuiClientCancelMessage;
struct MachGuiInGameChatMessage;
struct MachGuiHasMachinesCDMessage;
struct MachGuiIveLostMessage;
struct MachGuiNameChangeMessage;

class MachGuiStartupData;

class MachGuiMessageBroker : public MachLogMessageBroker
// Canonical form revoked
{
public:
    enum SystemType : uint8
    {
        SYSTEM_MACHLOG_MESSAGE = 0,
        SYSTEM_MACHGUI_MESSAGE
    };
    enum MessageType : uint8
    {
        MT_CHATMESSAGE,
        MT_JOINMESSAGE,
        MT_UPDATEPLAYERSMESSAGE,
        MT_RACECHANGEREQUESTMESSAGE,
        MT_IMREADYMESSAGE,
        MT_STARTMESSAGE,
        MT_HOSTCANCELMESSAGE,
        MT_CLIENTCANCELMESSAGE,
        MY_UPDATEGAMESETTINGSMESSAGE,
        MT_INGAMECHATMESSAGE,
        MT_HASMACHINESCDMESSAGE,
        MT_IVELOSTMESSAGE,
        MT_NAMECHANGEMESSAGE
    };

    MachGuiMessageBroker(MachGuiStartupData*);
    ~MachGuiMessageBroker() override;

    void processMessage(NetMessage*) override;

    void sendChatMessage(const std::string& chat);

    void sendJoinMessage(const std::string& playerName, int uniqueMachineNumber);

    void sendUpdatePlayersMessage();

    void sendUpdateGameSettingsMessage();

    void sendRaceChangeRequest(const std::string& playerName, size_t playerIndex, MachPhys::Race newRace);

    void sendImReadyMessage(const std::string& playerName, bool ready);

    // When all players have sent an "ImReady" message then the host can start the game. This message
    // will instruct all players to switch into game mode.
    void sendStartMessage();

    void sendHostCancelMessage();

    void sendClientCancelMessage(const std::string& playerName);

    void sendInGameChatMessage(const std::string& message, MachPhys::Race intendedForRace);

    void sendHasMachinesCDMessage(const std::string& playerName, bool hasMachinesCD);

    void sendIveLostMessage(const std::string& message);

    void sendNameChangeMessage(const std::string& newPlayerName, int uniqueMachineNumber);

protected:
    void processMachGuiMessage(NetMessage* pMessage);
    void processChatMessage(MachGuiChatMessage* pMessage);
    void processJoinMessage(MachGuiJoinMessage* pMessage);
    void processUpdatePlayersMessage(MachGuiUpdatePlayersMessage* pMessage);
    void processRaceChangeRequest(MachGuiRaceChangeRequestMessage* pMessage);
    void processImReadyMessage(MachGuiImReadyMessage* pMessage);
    void processStartMessage(MachGuiStartMessage* pMessage);
    void processHostCancelMessage(MachGuiHostCancelMessage* pMessage);
    void processClientCancelMessage(MachGuiClientCancelMessage* pMessage);
    void processUpdateGameSettingsMessage(MachGuiUpdateGameSettingsMessage* pMessage);
    void processInGameChatMessage(MachGuiInGameChatMessage* pMessage);
    void processHasMachinesCDMessage(MachGuiHasMachinesCDMessage* pMessage);
    void processIveLostMessage(MachGuiIveLostMessage* pMessage);
    void processNameChangeMessage(MachGuiNameChangeMessage* pMessage);

    SystemType getSystemType(NetMessage*);
    MessageType getMessageType(NetMessage*);

private:
    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiMessageBroker& t);
    friend std::ostream& operator<<(std::ostream& o, const MessageType& t);

    MachGuiMessageBroker(const MachGuiMessageBroker&);
    MachGuiMessageBroker& operator=(const MachGuiMessageBroker&);
    bool operator==(const MachGuiMessageBroker&);

    // Data members
    MachGuiStartupData* pStartupData_;
};

#endif

/* End MESSBROK.HPP *************************************************/
