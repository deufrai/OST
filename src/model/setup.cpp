#include "setup.h"

#include "boost/log/trivial.hpp"

void Setup::addDevice(const Device* pDevice) {

    if( ! _devicesMap.contains(pDevice->getName()) ) {
        _devicesMap[pDevice->getName()] = pDevice;
        BOOST_LOG_TRIVIAL(info) << "New device added: " << pDevice->getName();
    } else {
        BOOST_LOG_TRIVIAL(warning) << "Duplicate device name: " << pDevice->getName() << ". This should not happen";
    }
}

const Device *Setup::getDeviceByName(const std::string &deviceName) {
    return _devicesMap.contains(deviceName) ? _devicesMap[deviceName] : nullptr;
}