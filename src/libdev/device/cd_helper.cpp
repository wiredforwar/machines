#include "device/cd_helper.hpp"

#include "base/compiler.hpp" // For WEAK_SYMBOL def
#include "device/DevCDImpl.hpp"
#include "system/registry.hpp"

// WEAK_SYMBOL
void device::helper::cd::configure(DevCD* cd)
{
    constexpr char configEnableMusic[] = "Options\\Music\\enabled";
    auto configValue = std::string {};

    // by default enable, unless otherwise specified
    bool enableMusic = true;

    if (SysRegistry::SUCCESS == SysRegistry::instance().queryValueNoRecord(configEnableMusic, configValue))
    {
        enableMusic &= (std::atoi(configValue.c_str()) != 0);
    }

    DevCDImpl* pImpl = DevCDImpl::getInstance(cd);
    pImpl->musicEnabled_ = enableMusic;
}
