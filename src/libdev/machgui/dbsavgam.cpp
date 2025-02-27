/*
 * D B S A V G A M . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/dbsavgam.hpp"
#include "machgui/internal/dbsavgai.hpp"
#include "machgui/dbplayer.hpp"
#include "machgui/dbscenar.hpp"

PER_DEFINE_PERSISTENT(MachGuiDbSavedGame);

MachGuiDbSavedGame::MachGuiDbSavedGame(
    const std::string& userFileName,
    const std::string& fileName,
    MachGuiDbScenario* pDbScenario)
{
    pData_ = new MachGuiDbISavedGame;
    pData_->userFileName_ = userFileName;
    pData_->fileName_ = fileName;
    pData_->pDbScenario_ = pDbScenario;

    TEST_INVARIANT;
}

MachGuiDbSavedGame::~MachGuiDbSavedGame()
{
    TEST_INVARIANT;

    delete pData_;
}

void MachGuiDbSavedGame::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiDbSavedGame& t)
{

    o << "MachGuiDbSavedGame " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiDbSavedGame " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachGuiDbSavedGame& ob)
{
    ostr << ob.pData_;
}

void perRead(PerIstream& istr, MachGuiDbSavedGame& ob)
{
    istr >> ob.pData_;
}

MachGuiDbSavedGame::MachGuiDbSavedGame(PerConstructor)
    : pData_(nullptr)
{
}

const std::string& MachGuiDbSavedGame::userFileName() const
{
    return pData_->userFileName_;
}

void MachGuiDbSavedGame::userFileName(const std::string& name)
{
    pData_->userFileName_ = name;
}

const std::string& MachGuiDbSavedGame::externalFileName() const
{
    return pData_->fileName_;
}

void MachGuiDbSavedGame::externalFileName(const std::string& name)
{
    pData_->fileName_ = name;
}

bool MachGuiDbSavedGame::hasPlayer() const
{
    return pData_->pDbPlayer_ != nullptr;
}

MachGuiDbPlayer& MachGuiDbSavedGame::player() const
{
    PRE(hasPlayer());
    return *(pData_->pDbPlayer_);
}

void MachGuiDbSavedGame::player(MachGuiDbPlayer* pDbPlayer)
{
    pData_->pDbPlayer_ = pDbPlayer;
}

bool MachGuiDbSavedGame::isCampaignGame() const
{
    return pData_->isCampaign_;
}

void MachGuiDbSavedGame::isCampaignGame(bool isIt)
{
    pData_->isCampaign_ = isIt;
}

MachGuiDbScenario& MachGuiDbSavedGame::scenario() const
{
    return *(pData_->pDbScenario_);
}

void MachGuiDbSavedGame::scenario(MachGuiDbScenario* pDbScenario)
{
    PRE(pDbScenario != nullptr);
    pData_->pDbScenario_ = pDbScenario;
}

/* End DBSAVGAM.CPP *************************************************/
