/*
 * D B S Y S T E M . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/dbsystem.hpp"
#include "machgui/internal/dbsystei.hpp"
#include "machgui/dbplanet.hpp"

PER_DEFINE_PERSISTENT(MachGuiDbSystem);

MachGuiDbSystem::MachGuiDbSystem(const std::string& systemName, uint menuStringId)
    : MachGuiDbElement(menuStringId)
{
    pData_ = new MachGuiDbISystem;

    isCustom(false);
    name(systemName);

    TEST_INVARIANT;
}

MachGuiDbSystem::~MachGuiDbSystem()
{
    TEST_INVARIANT;

    // Delete all the dependent planets
    for (size_t i = pData_->planets_.size(); i--;)
    {
        delete pData_->planets_[i];
    }

    delete pData_;
}

void MachGuiDbSystem::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiDbSystem& t)
{

    o << "MachGuiDbSystem " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiDbSystem " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void MachGuiDbSystem::campaignPicture(const std::string& filename)
{
    pData_->campaignPicture_ = filename;
}

const std::string& MachGuiDbSystem::campaignPicture() const
{
    return pData_->campaignPicture_;
}

uint MachGuiDbSystem::nPlanets() const
{
    return pData_->planets_.size();
}

MachGuiDbPlanet& MachGuiDbSystem::planet(uint index)
{
    PRE(index < nPlanets());

    return *(pData_->planets_[index]);
}

void MachGuiDbSystem::addPlanet(MachGuiDbPlanet* pPlanet)
{
    // Add to own list
    pData_->planets_.push_back(pPlanet);

    // Store back pointer in planet
    pPlanet->system(this);
}

void perWrite(PerOstream& ostr, const MachGuiDbSystem& ob)
{
    const MachGuiDbElement& base = ob;

    ostr << base;

    ostr << ob.pData_;
}

void perRead(PerIstream& istr, MachGuiDbSystem& ob)
{
    MachGuiDbElement& base = ob;

    istr >> base;

    istr >> ob.pData_;
}

MachGuiDbSystem::MachGuiDbSystem(PerConstructor con)
    : MachGuiDbElement(con)
    , pData_(nullptr)
{
}
/* End DBSYSTEM.CPP *************************************************/
