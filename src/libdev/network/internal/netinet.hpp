#ifndef _NETINETWORK_HPP
#define _NETINETWORK_HPP

#define IDIRECTPLAY2_OR_GREATER

#include "ctl/vector.hpp"
#include "ctl/list.hpp"
#include "ctl/map.hpp"

#include "device/timer.hpp"

#include <enet/enet.h>

#include "network/netdefs.hpp"
#include "network/netnet.hpp"
#include "network/comport.hpp"
#include "network/node.hpp"
// #include "network/pinghelp.hpp"

class NetNode;
class NetNodeUid;
class NetProcessUid;
class NetAppSession;
class NetAppSessionUid;
class NetSystemMessageHandler;

struct NetMessageShortInfo
{
    double time_;
    int length_;
};
bool operator<(const NetMessageShortInfo& a, const NetMessageShortInfo& b);
bool operator==(const NetMessageShortInfo& a, const NetMessageShortInfo& b);

class NetINetwork
{
public:
    ///////////////////////////////

    enum class Update
    {
        No,
        Yes,
    };

    enum ObjectActionType
    {
        RETURN,
        RESET
    };

    ///////////////////////////////

    NetINetwork();
    ~NetINetwork();

    ///////////////////////////////

    const NetNetwork::ProtocolMap& availableProtocols(Update update = Update::Yes);
    void chooseProtocol(const std::string&, NetNetwork::InitialiseConnection);
    void initialiseConnection();

    static const NetNetwork::NetNetworkStatus& currentStatus();
    static void currentStatus(NetNetwork::NetNetworkStatus);
    static void resetStatus();

    ///////////////////////////////

    NetAppSession* createAppSession(const std::string& gameName);
    NetAppSession* joinAppSession(const std::string& addressStr);
    NetAppSession* connectAppSession();
    void resetAppSession();

    NetAppUid appUid() const;
    bool hasAppSession(const NetAppSessionUid&) const;
    //  Use the no record version in assertions
    bool hasAppSessionNoRecord(const NetAppSessionUid&) const;
    NetAppSession& session();
    void update();
    void clearSessions();
    void updateSessions();
    void pollMessages();
    const NetNetwork::Sessions& sessions() const;

    bool isValid() const;
    bool hasActiveSession() const;

    void systemMessageHandler(NetSystemMessageHandler*);
    bool hasSystemMessageHandler() const;
    NetSystemMessageHandler& systemMessageHandler();

    // mainly used with lobby technology - but the fields should be valid for normal create/join mechanism as well
    bool isLobbiedGame() const;
    bool isLogicalHost() const;
    const std::string& localPlayerName() const;
    void setLocalPlayerName(const std::string&);

    ///////////////////////////////
    bool imStuffed() const;
    bool imStuffedNoRecord() const;

    bool deterministicPingDropoutAllowed() const;
    void setDeterministicPingDropoutAllowed(bool);

private:
    ///////////////////////////////

    using Peers = std::vector<ENetPeer*>;
    using NetMessageBuffer = ctl_pvector<NetMessage>;

    ///////////////////////////////

    void clearProtocols();
    const std::string& protocolName();
    std::string& protocolNameNoRecord();
    void setProtocolName(const std::string&);
    void setAppUid();

    ///////////////////////////////

    bool hasAppSession(const NetAppSessionName&) const;
    //  Use the no record version in assertions
    bool hasAppSessionNoRecord(const NetAppSessionName&) const;
    void enterAppSession(NetNode*, NetAppSessionUid);
    void doAbort(const std::string& reasonCode);
    void initHost(bool asServer = false);
    void resetHost();
    NetMessage* getMessage();
    bool haveMessages();
    void sendMessage(const NetPriority& priority, const NetMessageRecipients& /*to*/, const NetMessageBody& body);

    NetNetwork::NetworkProtocol currentProtocol() const;

    // Ip addresses may be in the form of numerical IP addresses or domain net
    const std::string& IPAddress() const;
    void setIPAddress(const std::string& newIPAddress);

    bool isValidNoRecord() const;
    bool hasLocalNodeNoRecord(const NetNode*) const;
    static NetNetwork::NetNetworkStatus& currentStatusNoRecord();

    NetAppUid appUidNoRecord() const;

    NetProcessUid& processUidMaster() const;
    void addSentMessage(int length);
    void computeSentMessageStuffedNess();
    double deterministicTimeoutPeriod() const;
    size_t maxSentMessagesPerSecond() const;
    void autoAdjustMaxSentMessagesPerSecond(size_t numberOfPlayers);

    void disableNewPlayers();
    bool pingAllAllowed() const;

    void sendInitPacket(ENetPeer* pPeer);

    void initServersDiscoverySocket();
    void deinitServersDiscoverySocket();
    void sendLocalServersDiscoveryBroadcast();
    void acceptLocalServersReplies();
    bool initLocalServerDiscovery();
    void deinitLocalServerDiscovery();
    void replyToServerDiscoveryRequests();

    void determineStandardSendFlags();

    void messageThrottlingActive(bool);
    bool messageThrottlingActive() const;

    ///////////////////////////////

    static NetAppUid appUid_;

    ///////////////////////////////

    NetNode::NetMessageBuffer messageBuffer_;
    NetNetwork::Sessions sessions_;
    double lastSessionsUpdate_{};
    Peers peers_;
    NetAppSession* pLocalSession_{};
    NetSystemMessageHandler* pSystemMessageHandler_{};

    bool lobbyAware_{};

    // lobby description fields
    std::string descAppName_;
    std::string descFileName_;
    std::string descCommandLine_;
    std::string descPath_;
    std::string descCurrentDirectory_;
    std::string descDescription_;

    bool isLobbiedGame_{};
    bool isLogicalHost_{};
    std::string localPlayerName_;
    std::string gameName_;

    std::string IPAddress_;

    NetNetwork::NetworkProtocol currentProtocol_;

    bool imStuffed_{};

    //  NetPingHelper                   pingHelper_;

    ENetSocket lanDiscoveryServerSocket_{ENET_SOCKET_NULL};
    ENetSocket lanDiscoveryClientSocket_{ENET_SOCKET_NULL};
    ENetAddress address_{};
    ENetHost* pHost_{};

    bool deterministicPingDropoutAllowed_{};

    ///////////////////////////////

    friend class NetNode;
    friend class NetNetwork;
    friend class NetIAppSession;
    friend class NetSystemMessageHandler;
    ///////////////////////////////
    DevTimer timer_;

    using SentMessages = ctl_list<NetMessageShortInfo*>;
    SentMessages sentMessages_;

    int maxBytesPerSecond_;
    bool pingAllAllowed_;
    int maxSentMessagesPerSecond_;
    int originalMaxSentMessagesPerSecond_;

    bool messageThrottlingActive_{};
    NetNetwork::ProtocolMap availableProtocols_;
};

#endif //_NetINetwork_HPP
