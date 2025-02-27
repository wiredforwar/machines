/*
 * D B E L E M E N . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/dbelemen.hpp"
#include "machgui/internal/dbelemei.hpp"
#include "machgui/dbtexdat.hpp"
#include "gui/restring.hpp"
#include <algorithm>

PER_DEFINE_PERSISTENT(MachGuiDbElement);

MachGuiDbElement::MachGuiDbElement(uint menuStringId)
{
    // Store data in the implementation object
    pData_ = new MachGuiDbIElement;
    pData_->menuStringId_ = menuStringId;

    TEST_INVARIANT;
}

MachGuiDbElement::~MachGuiDbElement()
{
    TEST_INVARIANT;

    delete pData_;
}

void MachGuiDbElement::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
    INVARIANT(pData_ != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiDbElement& t)
{

    o << "MachGuiDbElement " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiDbElement " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void MachGuiDbElement::addDependency(MachGuiDbElement* pAntecedent)
{
    // Add to the collection
    pData_->antecedents_.push_back(pAntecedent);
}

bool MachGuiDbElement::isAvailable() const
{
    bool result = true;

    // Check for any incomplete antecedents
    size_t nAntecedents = pData_->antecedents_.size();
    for (size_t i = nAntecedents; result && i--;)
        result = pData_->antecedents_[i]->isComplete();

    return result;
}

void MachGuiDbElement::name(const std::string& name)
{
    pData_->name_ = name;
}

const std::string& MachGuiDbElement::name() const
{
    return pData_->name_;
}

void MachGuiDbElement::isComplete(bool complete)
{
    pData_->isComplete_ = complete;
}

bool MachGuiDbElement::isComplete() const
{
    return pData_->isComplete_;
}

void MachGuiDbElement::isCustom(bool custom)
{
    pData_->isCustom_ = custom;
}

bool MachGuiDbElement::isCustom() const
{
    return pData_->isCustom_;
}

void MachGuiDbElement::textDataFileName(const std::string& filename)
{
    pData_->textDataFileName_ = filename;
}

const std::string& MachGuiDbElement::textDataFileName() const
{
    return pData_->textDataFileName_;
}

const MachGuiDbTextData& MachGuiDbElement::textData() const
{
    // Ensure the data object has been loaded
    if (pData_->pTextData_ == nullptr)
        pData_->pTextData_ = MachGuiDbTextData::pNewTextData(pData_->textDataFileName_);

    return *(pData_->pTextData_);
}

void MachGuiDbElement::clearTextData()
{
    if (pData_->pTextData_ != nullptr)
    {
        delete pData_->pTextData_;
        pData_->pTextData_ = nullptr;
    }
}

void perWrite(PerOstream& ostr, const MachGuiDbElement& ob)
{
    ostr << ob.pData_;
}

void perRead(PerIstream& istr, MachGuiDbElement& ob)
{
    istr >> ob.pData_;
}

MachGuiDbElement::MachGuiDbElement(PerConstructor)
    : pData_(nullptr)
{
}

uint MachGuiDbElement::menuStringId() const
{
    return pData_->menuStringId_;
}

void MachGuiDbElement::menuString(const std::string& name)
{
    pData_->menuName_ = name;
    // Use spacebars instead of underscores
    std::replace(pData_->menuName_.begin(), pData_->menuName_.end(), '_', ' ');
}

std::string MachGuiDbElement::menuString() const
{
    if (!menuStringId())
        return pData_->menuName_;
    GuiResourceString str(menuStringId());
    return str.asString();
}

const ctl_vector<MachGuiDbElement*>& MachGuiDbElement::antecedents() const
{
    return pData_->antecedents_;
}

/* End DBELEMEN.CPP *************************************************/
