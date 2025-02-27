/*
 * A N I M D A T A . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "world4d/internal/animdata.hpp"
#include "stdlib/string.hpp"

PER_DEFINE_PERSISTENT_ABSTRACT(W4dAnimationData);

W4dAnimationData::W4dAnimationData(const std::string& name, W4dLOD maxLod)
    : textureName_(name)
    , maxLod_(maxLod)
{

    TEST_INVARIANT;
}

W4dAnimationData::~W4dAnimationData()
{
    TEST_INVARIANT;
}

void W4dAnimationData::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const W4dAnimationData& t)
{

    o << "W4dAnimationData " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "W4dAnimationData " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void W4dAnimationData::name(const std::string& name)
{
    textureName_ = name;
}

const std::string& W4dAnimationData::name() const
{
    return textureName_;
}

const W4dLOD& W4dAnimationData::maxLod() const
{
    return maxLod_;
}

// static
const PhysRelativeTime& W4dAnimationData::forever()
{
    static const PhysRelativeTime oneYear = 31536000;
    return oneYear;
}

W4dAnimationData::W4dAnimationData(PerConstructor)
{
}

void perWrite(PerOstream& ostr, const W4dAnimationData& t)
{
    writeAllocatedStringFromPointer(ostr, &t.textureName_);
    ostr << t.maxLod_;
}

void perRead(PerIstream& istr, W4dAnimationData& t)
{
    // t.textureName_ used to be a heap-allocated string. Use special read function
    // to load PerDataType::PER_OBJECT_POINTER into a stack-allocated object
    readAllocatedStringFromPointer(istr, &t.textureName_);
    istr >> t.maxLod_;
}

/* End ANIMDATA.CPP *************************************************/
