/*
 * V M A I L . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogVoiceMail
    A class that represents a single piece of voice mail

*/

#ifndef _MACHLOG_VMAIL_HPP
#define _MACHLOG_VMAIL_HPP

#include "base/base.hpp"
#include "utility/id.hpp"
#include "machlog/vmdata.hpp"
#include "phys/phys.hpp"

class MachLogVoiceMailImpl;
class MachLogVoiceMailInfo;
class MexPoint3d;

class MachLogVoiceMail
// Canonical form revoked
{
public:
    // Construct with VoiceMailID
    MachLogVoiceMail(const MachLogVoiceMailInfo &info);
    // Construct with VoiceMailID and actor Id
    MachLogVoiceMail(const MachLogVoiceMailInfo &info, UtlId actorId);
    // Construct with VoiceMailID, actor Id, and position
    MachLogVoiceMail(const MachLogVoiceMailInfo &info, UtlId actorId, MexPoint3d& position);
    // Construct with VoiceMailID, and position
    MachLogVoiceMail(const MachLogVoiceMailInfo &info, MexPoint3d& position);

    ~MachLogVoiceMail();

    // Return VEMail Id
    VoiceMailID id() const;

    // Has got an associated actor id?
    bool hasActorId() const;
    // Return actor id
    UtlId actorId() const;

    // Has got an associated position?
    bool hasPosition() const;
    // Return associated position
    MexPoint3d position() const;

    // Play mail
    void play();
    // PRE(!isSampleValid());
    // POST(isSampleValid());

    // Is mail playing?
    bool isPlaying() const;
    // Stop mail playing

    void stop();
    // PRE(isSampleValid());
    // PRE(isPlaying());
    // POST(!isPlaying());

    // Has mail started playing
    bool hasStarted() const;

    // Return the saved mail time stamp
    // If this is not relevent then return 0
    PhysAbsoluteTime timeStamp() const;

    void timeStamp(const PhysAbsoluteTime&);

    // Is this sample valid ie does it have resources
    // and a valid handle
    bool isSampleValid() const;

    // Invalidate the sample handle and free the sample resources
    void invalidateSample();
    // PRE(isSampleValid());
    // PRE(!isPlaying());
    // POST(!isSampleValid());

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachLogVoiceMail& t);

    MachLogVoiceMail(const MachLogVoiceMail&);
    MachLogVoiceMail& operator=(const MachLogVoiceMail&);

    MachLogVoiceMailImpl* pImpl_;
};

#endif

/* End VMAIL.HPP ****************************************************/
