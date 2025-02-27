#include "afx/AfxConfigurationLoader.hpp"

#include "system/registry.hpp"

IAfxConfigurationLoader::~IAfxConfigurationLoader() = default;

AfxConfigurationLoader::AfxConfigurationLoader()
{
}

// virtual
AfxConfigurationLoader::~AfxConfigurationLoader()
{
}

// virtual
AfxConfigurationData* AfxConfigurationLoader::load()
{
    // Caller will be responsible for cleanup
    AfxConfigurationData* config = new AfxConfigurationData();

    constexpr char msaaBuffers[] = "Options\\MSAA\\Buffers";
    constexpr char msaaSamples[] = "Options\\MSAA\\Samples";
    auto configValue = std::string {};

    // defaults for when not present in config
    int buffers = 1;
    int samples = 2;

    if (SysRegistry::SUCCESS == SysRegistry::instance().queryValueNoRecord(msaaBuffers, configValue))
    {
        buffers = std::atoi(configValue.c_str());
        configValue.clear();
    }
    if (SysRegistry::SUCCESS == SysRegistry::instance().queryValueNoRecord(msaaSamples, configValue))
    {
        samples = std::atoi(configValue.c_str());
        configValue.clear();
    }

    config->multisampleBuffers = buffers;
    config->multisampleSamples = samples;

    return config;
}
