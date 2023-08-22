
void FAAbilityThread::HandleConnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }

    sptr<IRemoteObject> service = abilityImpl_->ConnectAbility(want);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleConnectAbilityDone(token_, service);
    if (err != ERR_OK) {
        HILOG_ERROR("failed err = %{public}d", err);
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleDisconnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }

    abilityImpl_->DisconnectAbility(want);
    HILOG_DEBUG("Handle done");
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleDisconnectAbilityDone(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("err = %{public}d", err);
    }
}

void FAAbilityThread::HandleCommandAbility(const Want &want, bool restart, int32_t startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->CommandAbility(want, restart, startId);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleCommandAbilityDone(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("failed err = %{public}d", err);
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleConnectExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("extensionImpl_ is nullptr");
        return;
    }
    bool isAsyncCallback = false;
    sptr<IRemoteObject> service = extensionImpl_->ConnectExtension(want, isAsyncCallback);
    if (!isAsyncCallback) {
        extensionImpl_->ConnectExtensionCallback(service);
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleDisconnectExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("extensionImpl_ is nullptr");
        return;
    }

    bool isAsyncCallback = false;
    extensionImpl_->DisconnectExtension(want, isAsyncCallback);
    if (!isAsyncCallback) {
        extensionImpl_->DisconnectExtensionCallback();
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleCommandExtension(const Want &want, bool restart, int32_t startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("extensionImpl_ is nullptr");
        return;
    }
    extensionImpl_->CommandExtension(want, restart, startId);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleCommandAbilityDone(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("failed err = %{public}d", err);
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleCommandExtensionWindow(
    const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("extensionImpl_ is nullptr");
        return;
    }
    extensionImpl_->CommandExtensionWindow(want, sessionInfo, winCmd);
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleRestoreAbilityState(const AppExecFwk::PacMap &state)
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }

    abilityImpl_->DispatchRestoreAbilityState(state);
    HILOG_DEBUG("end");
}

void FAAbilityThread::ScheduleSaveAbilityState()
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }

    abilityImpl_->DispatchSaveAbilityState();
    HILOG_DEBUG("end");
}

void FAAbilityThread::ScheduleRestoreAbilityState(const AppExecFwk::PacMap &state)
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->DispatchRestoreAbilityState(state);
    HILOG_DEBUG("end");
}

void FAAbilityThread::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    HILOG_DEBUG("begin");
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, config]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }

        if (abilityThread->isExtension_) {
            abilityThread->HandleExtensionUpdateConfiguration(config);
        } else {
            abilityThread->HandleUpdateConfiguration(config);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }

    abilityImpl_->ScheduleUpdateConfiguration(config);
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleExtensionUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    HILOG_DEBUG("begin");
    if (!extensionImpl_) {
        HILOG_ERROR("extensionImpl_ is nullptr");
        return;
    }

    extensionImpl_->ScheduleUpdateConfiguration(config);
    HILOG_DEBUG("end");
}

void FAAbilityThread::ScheduleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AppExecFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("name:%{public}s,targeState:%{public}d,isNewWant:%{public}d",
        want.GetElement().GetAbilityName().c_str(),
        lifeCycleStateInfo.state,
        lifeCycleStateInfo.isNewWant);

    if (token_ == nullptr) {
        HILOG_ERROR("token_ is nullptr");
        return;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want, lifeCycleStateInfo, sessionInfo]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleExtensionTransaction(want, lifeCycleStateInfo, sessionInfo);
            Want newWant(want);
            newWant.CloseAllFd();
        } else {
            abilityThread->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
}

void FAAbilityThread::ScheduleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!token_) {
        HILOG_ERROR("token_ is nullptr");
        return;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, uniqueId]() {
        auto abilityThread = weak.promote();
        if (!abilityThread) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        abilityThread->HandleShareData(uniqueId);
    };

    if (!abilityHandler_) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("postTask error");
    }
}

void FAAbilityThread::ScheduleConnectAbility(const Want &want)
{
    HILOG_DEBUG("begin, isExtension_: %{public}d", isExtension_);
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleConnectExtension(want);
        } else {
            abilityThread->HandleConnectAbility(want);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::ScheduleDisconnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin, isExtension: %{public}d", isExtension_);
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleDisconnectExtension(want);
        } else {
            abilityThread->HandleDisconnectAbility(want);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
}

void FAAbilityThread::ScheduleCommandAbility(const Want &want, bool restart, int startId)
{
    HILOG_DEBUG("begin. startId: %{public}d", startId);
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want, restart, startId]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        if (abilityThread->isExtension_) {
            Want newWant(want);
#ifdef WITH_DLP
            bool sandboxFlag = Security::DlpPermission::DlpFileKits::GetSandboxFlag(newWant);
            newWant.SetParam(DLP_PARAMS_SANDBOX, sandboxFlag);
            if (sandboxFlag) {
                newWant.CloseAllFd();
            }
#endif // WITH_DLP
            abilityThread->HandleCommandExtension(newWant, restart, startId);
            newWant.CloseAllFd();
        } else {
            abilityThread->HandleCommandAbility(want, restart, startId);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
    HILOG_DEBUG("end");
}

bool FAAbilityThread::SchedulePrepareTerminateAbility()
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return true;
    }
    if (getpid() == gettid()) {
        bool ret = abilityImpl_->PrepareTerminateAbility();
        HILOG_DEBUG("end, ret = %{public}d", ret);
        return ret;
    } else {
        wptr<FAAbilityThread> weak = this;
        auto task = [weak]() {
            auto abilityThread = weak.promote();
            if (abilityThread == nullptr) {
                HILOG_ERROR("abilityThread is nullptr");
                return;
            }
            abilityThread->HandlePrepareTermianteAbility();
        };

        if (abilityHandler_ == nullptr) {
            HILOG_ERROR("abilityHandler_ is nullptr");
            return false;
        }

        bool ret = abilityHandler_->PostTask(task);
        if (!ret) {
            HILOG_ERROR("PostTask error");
            return false;
        }

        std::unique_lock<std::mutex> lock(mutex_);
        auto condition = [this] {
            if (this->isPrepareTerminateAbilityDone_) {
                return true;
            } else {
                return false;
            }
        };
        if (!cv_.wait_for(lock, std::chrono::milliseconds(PREPARE_TO_TERMINATE_TIMEOUT_MILLISECONDS), condition)) {
            HILOG_WARN("Wait timeout");
        }
        HILOG_DEBUG("end, ret = %{public}d", isPrepareTerminate_);
        return isPrepareTerminate_;
    }
}

void FAAbilityThread::ScheduleCommandAbilityWindow(
    const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HILOG_DEBUG("begin");
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want, sessionInfo, winCmd]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        abilityThread->HandleCommandExtensionWindow(want, sessionInfo, winCmd);
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::SendResult(int requestCode, int resultCode, const Want &want)
{
    HILOG_DEBUG("begin");
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, requestCode, resultCode, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        if (requestCode == -1) {
            HILOG_ERROR("requestCode is -1");
            return;
        }
        if (abilityThread->isExtension_ && abilityThread->extensionImpl_ != nullptr) {
            abilityThread->extensionImpl_->SendResult(requestCode, resultCode, want);
            return;
        } else if (!abilityThread->isExtension_ && abilityThread->abilityImpl_ != nullptr) {
            abilityThread->abilityImpl_->SendResult(requestCode, resultCode, want);
            return;
        }
        HILOG_ERROR("%{public}simpl is nullptr", abilityThread->isExtension_ ? "extension" : "ability");
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
    HILOG_DEBUG("end");
}

std::vector<std::string> FAAbilityThread::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    HILOG_DEBUG("begin");
    std::vector<std::string> types;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return types;
    }

    types = abilityImpl_->GetFileTypes(uri, mimeTypeFilter);
    HILOG_DEBUG("end");
    return types;
}

int FAAbilityThread::OpenFile(const Uri &uri, const std::string &mode)
{
    HILOG_DEBUG("begin");
    int32_t fd = -1;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return fd;
    }

    fd = abilityImpl_->OpenFile(uri, mode);
    HILOG_DEBUG("end");
    return fd;
}

