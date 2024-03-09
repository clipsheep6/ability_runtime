/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ecmascript/module/napi_module_loader.h"
#include "ecmascript/module/module_path_helper.h"
#include "ecmascript/module/js_module_manager.h"
#include "ecmascript/jspandafile/js_pandafile.h"
#include "ecmascript/jspandafile/js_pandafile_manager.h"
#include "ecmascript/jspandafile/js_pandafile_executor.h"

namespace panda::ecmascript {
JSHandle<JSTaggedValue> NapiModuleLoader::LoadModuleNameSpaceWithModuleInfo(EcmaVM *vm, CString &requestPath,
    CString &modulePath)
{
    CString moduleStr = ModulePathHelper::GetModuleNameWithPath(modulePath);
    CString abcFilePath = ModulePathHelper::ConcatPandaFilePath(moduleStr);
    JSThread *thread = vm->GetJSThread();
    std::shared_ptr<JSPandaFile> curJsPandaFile;
    if (modulePath.size() != 0) {
        curJsPandaFile = JSPandaFileManager::GetInstance()->LoadJSPandaFile(thread, abcFilePath, requestPath);
        if (curJsPandaFile == nullptr) {
            CString msg = "Load file with filename '" + abcFilePath +
                "' failed, module name '" + requestPath + "'" + ", from napi load module";
            THROW_NEW_ERROR_AND_RETURN_HANDLE(thread, ErrorType::REFERENCE_ERROR, JSTaggedValue, msg.c_str());
        }
        if (ModulePathHelper::NeedTranstale(requestPath)) {
            ModulePathHelper::TranstaleExpressionInput(curJsPandaFile.get(), requestPath);
        }
    }
    JSHandle<JSTaggedValue> nameSp = LoadModuleNameSpaceWithPath(thread, abcFilePath, requestPath, modulePath,
        curJsPandaFile.get());
    return nameSp;
}

JSHandle<JSTaggedValue> NapiModuleLoader::LoadModuleNameSpaceWithPath(JSThread *thread, CString &abcFilePath,
    CString &requestPath, CString &modulePath, const JSPandaFile *pandaFile)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    auto [isNative, moduleType] = SourceTextModule::CheckNativeModule(requestPath);
    ModuleManager *moduleManager = thread->GetCurrentEcmaContext()->GetModuleManager();
    if (isNative) {
        std::string key = ConvertToStdString(requestPath);
        JSHandle<JSTaggedValue> moduleHandle = moduleManager->LoadNativeModule(thread, key);
        return moduleHandle;
    }
    JSMutableHandle<JSTaggedValue> moduleName(thread, thread->GlobalConstants()->GetUndefined());
    CString entryPoint = ModulePathHelper::ConcatFileNameWithMerge(thread, pandaFile,
        abcFilePath, modulePath, requestPath);
    RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread);
    JSTaggedValue entryPointValue = factory->NewFromUtf8(entryPoint).GetTaggedValue();
    moduleName.Update(entryPointValue);

    std::shared_ptr<JSPandaFile> jsPandaFile =
        JSPandaFileManager::GetInstance()->LoadJSPandaFile(thread, abcFilePath, entryPoint);
    if (jsPandaFile == nullptr) {
        CString msg = "Load file with filename '" + abcFilePath +
            "' failed, module name '" + requestPath + "'" + ", from napi load module";
        THROW_NEW_ERROR_AND_RETURN_HANDLE(thread, ErrorType::REFERENCE_ERROR, JSTaggedValue, msg.c_str());
    }

    JSRecordInfo recordInfo;
    bool hasRecord = jsPandaFile->CheckAndGetRecordInfo(entryPoint, recordInfo);
    if (!hasRecord) {
        LOG_FULL(ERROR) << "cannot find record '" << entryPoint <<"' in basefileName " << abcFilePath << ","
            << "from napi load module";
        CString msg = "cannot find record '" + entryPoint + "' in basefileName " + abcFilePath + "," +
            "from napi load module";
        THROW_NEW_ERROR_AND_RETURN_HANDLE(thread, ErrorType::REFERENCE_ERROR, JSTaggedValue, msg.c_str());
    }

    if (!moduleManager->IsImportedModuleLoaded(moduleName.GetTaggedValue())) {
        if (!JSPandaFileExecutor::ExecuteFromAbcFile(thread, abcFilePath.c_str(), entryPoint.c_str(), false, true)) {
            CString msg = "Cannot execute request from napi load module : " + entryPoint +
                ", from napi load module";
            THROW_NEW_ERROR_AND_RETURN_HANDLE(thread, ErrorType::REFERENCE_ERROR, JSTaggedValue, msg.c_str());
        }
    }
    JSHandle<SourceTextModule> moduleRecord = moduleManager->HostGetImportedModule(moduleName.GetTaggedValue());
    JSHandle<JSTaggedValue> nameSp = SourceTextModule::GetModuleNamespace(thread, moduleRecord);
    RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread);
    return nameSp;
}
}