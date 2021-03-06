// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "common/concepts.h"
#include "core/hle/kernel/k_port.h"
#include "core/hle/result.h"
#include "core/hle/service/service.h"

namespace Core {
class System;
}

namespace Kernel {
class KClientPort;
class KClientSession;
class KernelCore;
class KPort;
class KServerPort;
class SessionRequestHandler;
} // namespace Kernel

namespace Service::SM {

class Controller;

/// Interface to "sm:" service
class SM final : public ServiceFramework<SM> {
public:
    explicit SM(std::shared_ptr<ServiceManager> service_manager_, Core::System& system_);
    ~SM() override;

private:
    void Initialize(Kernel::HLERequestContext& ctx);
    void GetService(Kernel::HLERequestContext& ctx);
    void RegisterService(Kernel::HLERequestContext& ctx);
    void UnregisterService(Kernel::HLERequestContext& ctx);

    std::shared_ptr<ServiceManager> service_manager;
    Kernel::KernelCore& kernel;
};

class ServiceManager {
public:
    static void InstallInterfaces(std::shared_ptr<ServiceManager> self, Core::System& system);

    explicit ServiceManager(Kernel::KernelCore& kernel_);
    ~ServiceManager();

    ResultVal<Kernel::KServerPort*> RegisterService(std::string name, u32 max_sessions);
    ResultCode UnregisterService(const std::string& name);
    ResultVal<Kernel::KPort*> GetServicePort(const std::string& name);

    template <Common::DerivedFrom<Kernel::SessionRequestHandler> T>
    std::shared_ptr<T> GetService(const std::string& service_name) const {
        auto service = registered_services.find(service_name);
        if (service == registered_services.end()) {
            LOG_DEBUG(Service, "Can't find service: {}", service_name);
            return nullptr;
        }
        auto* port = service->second;
        if (port == nullptr) {
            return nullptr;
        }
        return std::static_pointer_cast<T>(port->GetServerPort().GetHLEHandler());
    }

    void InvokeControlRequest(Kernel::HLERequestContext& context);

private:
    std::weak_ptr<SM> sm_interface;
    std::unique_ptr<Controller> controller_interface;

    /// Map of registered services, retrieved using GetServicePort.
    std::unordered_map<std::string, Kernel::KPort*> registered_services;

    /// Kernel context
    Kernel::KernelCore& kernel;
};

} // namespace Service::SM
