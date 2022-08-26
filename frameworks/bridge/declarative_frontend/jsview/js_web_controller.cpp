/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "frameworks/bridge/declarative_frontend/jsview/js_web_controller.h"

#include "base/utils/linear_map.h"
#include "base/utils/utils.h"
#include "frameworks/bridge/declarative_frontend/engine/functions/js_webview_function.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_common_def.h"

namespace OHOS::Ace::Framework {
namespace {
void ParseWebViewValueToJsValue(
    std::shared_ptr<WebJSValue> value, std::vector<JSRef<JSVal>>& argv)
{
    auto type = value->GetType();
    switch (type) {
        case WebJSValue::Type::INTEGER:
            argv.push_back(JSRef<JSVal>::Make(ToJSValue(value->GetInt())));
            break;
        case WebJSValue::Type::DOUBLE: {
            argv.push_back(JSRef<JSVal>::Make(ToJSValue(value->GetDouble())));
            break;
        }
        case WebJSValue::Type::BOOLEAN:
            argv.push_back(JSRef<JSVal>::Make(ToJSValue(value->GetBoolean())));
            break;
        case WebJSValue::Type::STRING:
            argv.push_back(JSRef<JSVal>::Make(ToJSValue(value->GetString())));
            break;
        case WebJSValue::Type::NONE:
            break;
        default:
            LOGW("WebJavaScriptResultCallBack: jsvalue type[%{public}d] not support!", (int)type);
            break;
    }
}

std::shared_ptr<WebJSValue> ParseValue(
    const JSRef<JSVal>& resultValue, std::shared_ptr<WebJSValue> webviewValue)
{
    webviewValue->error_ = static_cast<int>(WebJavaScriptBridgeError::NO_ERROR);
    if (resultValue->IsBoolean()) {
        webviewValue->SetType(WebJSValue::Type::BOOLEAN);
        webviewValue->SetBoolean(resultValue->ToBoolean());
    } else if (resultValue->IsNull()) {
        webviewValue->SetType(WebJSValue::Type::NONE);
    } else if (resultValue->IsString()) {
        webviewValue->SetType(WebJSValue::Type::STRING);
        webviewValue->SetString(resultValue->ToString());
    } else if (resultValue->IsNumber()) {
        webviewValue->SetType(WebJSValue::Type::DOUBLE);
        webviewValue->SetInt(resultValue->ToNumber<double>());
    } else if (resultValue->IsArray() || resultValue->IsObject() || resultValue->IsUndefined()) {
        webviewValue->SetType(WebJSValue::Type::NONE);
        webviewValue->error_ = static_cast<int>(WebJavaScriptBridgeError::OBJECT_IS_GONE);
    }
    return webviewValue;
}
}

class JSWebCookie : public Referenced {
public:
    static void JSBind(BindingTarget globalObj)
    {
        JSClass<JSWebCookie>::Declare("WebCookie");
        JSClass<JSWebCookie>::CustomMethod("setCookie", &JSWebCookie::SetCookie);
        JSClass<JSWebCookie>::CustomMethod("getCookie", &JSWebCookie::GetCookie);
        JSClass<JSWebCookie>::CustomMethod("deleteEntireCookie", &JSWebCookie::DeleteEntirelyCookie);
        JSClass<JSWebCookie>::CustomMethod("saveCookieSync", &JSWebCookie::SaveCookieSync);
        JSClass<JSWebCookie>::Bind(globalObj, JSWebCookie::Constructor, JSWebCookie::Destructor);
    }

    void SetWebCookie(WebCookie* manager)
    {
        if (manager != nullptr) {
            manager_ = manager;
        }
    }

    void SetCookie(const JSCallbackInfo& args)
    {
        std::string url;
        std::string value;
        bool result = false;
        if (args.Length() >= 2 && args[0]->IsString() && args[1]->IsString()) {
            url = args[0]->ToString();
            value = args[1]->ToString();
            if (manager_ != nullptr) {
                result = manager_->SetCookie(url, value);
            }
        }
        auto jsVal = JSVal(ToJSValue(result));
        auto returnValue = JSRef<JSVal>::Make(jsVal);
        args.SetReturnValue(returnValue);
    }

    void GetCookie(const JSCallbackInfo& args)
    {
        std::string result = "";
        if (manager_ != nullptr && args.Length() >= 1 && args[0]->IsString()) {
            std::string url = args[0]->ToString();
            result = manager_->GetCookie(url);
        }
        auto jsVal = JSVal(ToJSValue(result));
        auto returnValue = JSRef<JSVal>::Make(jsVal);
        args.SetReturnValue(returnValue);
    }

    void DeleteEntirelyCookie(const JSCallbackInfo& args)
    {
        if (manager_ == nullptr) {
            return;
        }
        manager_->DeleteEntirelyCookie();
    }

    void SaveCookieSync(const JSCallbackInfo& args)
    {
        bool result = false;
        if (manager_ != nullptr) {
            result = manager_->SaveCookieSync();
        }
        auto jsVal = JSVal(ToJSValue(result));
        auto returnValue = JSRef<JSVal>::Make(jsVal);
        args.SetReturnValue(returnValue);
    }

private:
    static void Constructor(const JSCallbackInfo& args)
    {
        auto jsWebCookie = Referenced::MakeRefPtr<JSWebCookie>();
        jsWebCookie->IncRefCount();
        args.SetReturnValue(Referenced::RawPtr(jsWebCookie));
    }

    static void Destructor(JSWebCookie* jsWebCookie)
    {
        if (jsWebCookie != nullptr) {
            jsWebCookie->DecRefCount();
        }
    }
    WebCookie* manager_;
};

class JSHitTestValue : public Referenced {
public:
    static void JSBind(BindingTarget globalObj)
    {
        JSClass<JSHitTestValue>::Declare("HitTestValue");
        JSClass<JSHitTestValue>::CustomMethod("getType", &JSHitTestValue::GetType);
        JSClass<JSHitTestValue>::CustomMethod("getExtra", &JSHitTestValue::GetExtra);
        JSClass<JSHitTestValue>::Bind(globalObj, JSHitTestValue::Constructor, JSHitTestValue::Destructor);
    }

    void SetType(int type)
    {
        type_ = type;
    }

    void SetExtra(const std::string& extra)
    {
        extra_ = extra;
    }

    void GetType(const JSCallbackInfo& args)
    {
        args.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(type_)));
    }

    void GetExtra(const JSCallbackInfo& args)
    {
        args.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(extra_)));
    }

private:
    static void Constructor(const JSCallbackInfo& args)
    {
        auto jSHitTestResult = Referenced::MakeRefPtr<JSHitTestValue>();
        jSHitTestResult->IncRefCount();
        args.SetReturnValue(Referenced::RawPtr(jSHitTestResult));
    }

    static void Destructor(JSHitTestValue* jSHitTestResult)
    {
        if (jSHitTestResult != nullptr) {
            jSHitTestResult->DecRefCount();
        }
    }

    std::string extra_;
    int type_ = static_cast<int>(WebHitTestType::UNKNOWN);
};

JSWebController::JSWebController()
{
    instanceId_ = Container::CurrentId();
}


std::shared_ptr<WebJSValue> JSWebController::GetJavaScriptResult(
    const std::string& objectName,
    const std::string& objectMethod,
    const std::vector<std::shared_ptr<WebJSValue>>& args)
{
    std::vector<JSRef<JSVal>> argv = {};
    std::shared_ptr<WebJSValue> jsResult =
        std::make_shared<WebJSValue>(WebJSValue::Type::NONE);
    if (objectorMap_.find(objectName) == objectorMap_.end()) {
        return jsResult;
    }
    auto jsObject = objectorMap_[objectName];
    if (jsObject->IsEmpty()) {
        return jsResult;
    }
    for (std::shared_ptr<WebJSValue> input: args) {
        ParseWebViewValueToJsValue(input, argv);
    }
    JSRef<JSFunc> func = JSRef<JSFunc>::Cast(jsObject->GetProperty(objectMethod.c_str()));
    if (func->IsEmpty()) {
        LOGE("%{public}s not found or is not a function!", objectMethod.c_str());
        jsResult->error_ = static_cast<int>(WebJavaScriptBridgeError::OBJECT_IS_GONE);
        return jsResult;
    }
    JSRef<JSVal> result = argv.empty() ? func->Call(jsObject, 0, {}) : func->Call(jsObject, argv.size(), &argv[0]);
    return ParseValue(result, jsResult);
}

class JSWebMessageEvent;
class JSWebMessagePort : public Referenced {
public:
    static void JSBind(BindingTarget globalObj)
    {
        JSClass<JSWebMessagePort>::Declare("WebMessagePort");
        JSClass<JSWebMessagePort>::CustomMethod("close", &JSWebMessagePort::Close);
        JSClass<JSWebMessagePort>::CustomMethod("postMessageEvent", &JSWebMessagePort::PostMessage);
        JSClass<JSWebMessagePort>::CustomMethod("onMessageEvent", &JSWebMessagePort::SetWebMessageCallback);
        JSClass<JSWebMessagePort>::Bind(globalObj, JSWebMessagePort::Constructor, JSWebMessagePort::Destructor);
    }

    void PostMessage(const JSCallbackInfo& args);

    void Close(const JSCallbackInfo& args)
    {
        if (port_ != nullptr) {
            port_->Close();
        } else {
            LOGE("port is null");
        }
    }

    void SetWebMessageCallback(const JSCallbackInfo& args)
    {
        LOGI("JSWebController onMessageEvent");
        if (args.Length() < 1 || !args[0]->IsObject()) {
            LOGE("invalid onEvent params");
            return;
        }

        JSRef<JSObject> obj = JSRef<JSObject>::Cast(args[0]);
        JSRef<JSVal> tsCallback = JSRef<JSVal>::Cast(obj);
        std::function<void(std::string)> callback = nullptr;
        if (tsCallback->IsFunction()) {
            auto jsCallback =
                AceType::MakeRefPtr<JsWebViewFunction>(JSRef<JSFunc>::Cast(tsCallback));
            callback = [execCtx = args.GetExecutionContext(), func = std::move(jsCallback)](std::string result) {
                JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
                ACE_SCORING_EVENT("onMessageEvent CallBack");
                LOGI("About to call onMessageEvent callback method on js");
                func->Execute(result);
            };
            if (port_ != nullptr) {
                port_->SetWebMessageCallback(std::move(callback));
            } else {
                LOGE("port is null");
            }
        } else {
            LOGE("JSAPI callback param is not a function");
        }
    }

    void SetWebMessagePort(const RefPtr<WebMessagePort>& port)
    {
        port_ = port;
    }

    RefPtr<WebMessagePort> GetWebMessagePort()
    {
        return port_;
    }

private:
    RefPtr<WebMessagePort> port_;

    static void Constructor(const JSCallbackInfo& args)
    {
        auto jsWebMessagePort = Referenced::MakeRefPtr<JSWebMessagePort>();
        jsWebMessagePort->IncRefCount();
        args.SetReturnValue(Referenced::RawPtr(jsWebMessagePort));
    }

    static void Destructor(JSWebMessagePort* jsWebMessagePort)
    {
        if (jsWebMessagePort != nullptr) {
            jsWebMessagePort->DecRefCount();
        }
    }
};

class JSWebMessageEvent : public Referenced {
public:
    static void JSBind(BindingTarget globalObj)
    {
        JSClass<JSWebMessageEvent>::Declare("WebMessageEvent");
        JSClass<JSWebMessageEvent>::CustomMethod("getData", &JSWebMessageEvent::GetData);
        JSClass<JSWebMessageEvent>::CustomMethod("setData", &JSWebMessageEvent::SetData);
        JSClass<JSWebMessageEvent>::CustomMethod("getPorts", &JSWebMessageEvent::GetPorts);
        JSClass<JSWebMessageEvent>::CustomMethod("setPorts", &JSWebMessageEvent::SetPorts);
        JSClass<JSWebMessageEvent>::Bind(globalObj, JSWebMessageEvent::Constructor, JSWebMessageEvent::Destructor);
    }

    void GetData(const JSCallbackInfo& args)
    {
        auto jsVal = JSVal(ToJSValue(data_));
        auto retVal = JSRef<JSVal>::Make(jsVal);
        args.SetReturnValue(retVal);
    }

    void SetData(const JSCallbackInfo& args)
    {
        if (args.Length() < 1 || !args[0]->IsString()) {
            LOGE("invalid url params");
            return;
        }
        data_ = args[0]->ToString();
    }

    void SetPorts(const JSCallbackInfo& args)
    {
        if (args.Length() <= 0) {
            LOGW("invalid url params");
            return;
        }
        JSRef<JSArray> jsPorts = JSRef<JSArray>::Cast(args[0]);
        std::vector<RefPtr<WebMessagePort>> sendPorts;
        if (jsPorts->IsArray()) {
            JSRef<JSArray> array = JSRef<JSArray>::Cast(jsPorts);
            for (size_t i = 0; i <  array->Length(); i++) {
                JSRef<JSVal> jsValue = array->GetValueAt(i);
                if (!jsValue->IsObject()) {
                    LOGW("invalid not object");
                    continue;
                }
                JSRef<JSObject> jsObj = JSRef<JSObject>::Cast(jsValue);
                RefPtr<JSWebMessagePort> jswebPort = Referenced::Claim(jsObj->Unwrap<JSWebMessagePort>());
                if (jswebPort) {
                    ports_.push_back(jswebPort);
                } else {
                    LOGE("jswebPort is null");
                }
            }
        } else {
            LOGE("jswebPort is not array");
        }
    }

    void GetPorts(const JSCallbackInfo& args)
    {
        JSRef<JSArray> jsPorts = JSRef<JSArray>::New();
        JSRef<JSObject> jsObj;
        RefPtr<JSWebMessagePort> jswebPort;
        for (size_t i = 0; i <  ports_.size(); i++) {
            jsObj = JSClass<JSWebMessagePort>::NewInstance();
            jswebPort = Referenced::Claim(jsObj->Unwrap<JSWebMessagePort>());
            jswebPort->SetWebMessagePort(ports_[i]->GetWebMessagePort());
            jsPorts->SetValueAt(i, jsObj);
        }
        args.SetReturnValue(jsPorts);
    }

    std::string GetDataInternal()
    {
        return data_;
    }

    std::vector<RefPtr<JSWebMessagePort>> GetPortsInternal()
    {
        return ports_;
    }

private:
    std::string data_;
    std::vector<RefPtr<JSWebMessagePort>> ports_;

    static void Constructor(const JSCallbackInfo& args)
    {
        auto jsWebMessageEvent = Referenced::MakeRefPtr<JSWebMessageEvent>();
        jsWebMessageEvent->IncRefCount();
        args.SetReturnValue(Referenced::RawPtr(jsWebMessageEvent));
    }

    static void Destructor(JSWebMessageEvent* jsWebMessageEvent)
    {
        if (jsWebMessageEvent != nullptr) {
            jsWebMessageEvent->DecRefCount();
        }
    }
};

void JSWebMessagePort::PostMessage(const JSCallbackInfo& args)
{
    if (args.Length() <= 0 || !(args[0]->IsObject())) {
        LOGE("invalid PostMessage params");
        return;
    }
    // get ports
    JSRef<JSVal> jsPorts = JSRef<JSVal>::Cast(args[0]);
    if (!jsPorts->IsObject()) {
        LOGE("invalid param, not a object");
        return;
    }
    auto jsRes = Referenced::Claim(JSRef<JSObject>::Cast(jsPorts)->Unwrap<JSWebMessageEvent>());
    std::string data = jsRes->GetDataInternal();
    if (port_) {
        port_->PostMessage(data);
    }
}

void JSWebController::JSBind(BindingTarget globalObj)
{
    JSClass<JSWebController>::Declare("WebController");
    JSClass<JSWebController>::CustomMethod("loadUrl", &JSWebController::LoadUrl);
    JSClass<JSWebController>::CustomMethod("runJavaScript", &JSWebController::ExecuteTypeScript);
    JSClass<JSWebController>::CustomMethod("refresh", &JSWebController::Refresh);
    JSClass<JSWebController>::CustomMethod("stop", &JSWebController::StopLoading);
    JSClass<JSWebController>::CustomMethod("getHitTest", &JSWebController::GetHitTestResult);
    JSClass<JSWebController>::CustomMethod("registerJavaScriptProxy", &JSWebController::AddJavascriptInterface);
    JSClass<JSWebController>::CustomMethod("deleteJavaScriptRegister", &JSWebController::RemoveJavascriptInterface);
    JSClass<JSWebController>::CustomMethod("onInactive", &JSWebController::OnInactive);
    JSClass<JSWebController>::CustomMethod("onActive", &JSWebController::OnActive);
    JSClass<JSWebController>::CustomMethod("zoom", &JSWebController::Zoom);
    JSClass<JSWebController>::CustomMethod("requestFocus", &JSWebController::RequestFocus);
    JSClass<JSWebController>::CustomMethod("loadData", &JSWebController::LoadDataWithBaseUrl);
    JSClass<JSWebController>::CustomMethod("backward", &JSWebController::Backward);
    JSClass<JSWebController>::CustomMethod("forward", &JSWebController::Forward);
    JSClass<JSWebController>::CustomMethod("accessStep", &JSWebController::AccessStep);
    JSClass<JSWebController>::CustomMethod("accessForward", &JSWebController::AccessForward);
    JSClass<JSWebController>::CustomMethod("accessBackward", &JSWebController::AccessBackward);
    JSClass<JSWebController>::CustomMethod("clearHistory", &JSWebController::ClearHistory);
    JSClass<JSWebController>::CustomMethod("getCookieManager", &JSWebController::GetCookieManager);
    JSClass<JSWebController>::CustomMethod("getHitTestValue", &JSWebController::GetHitTestValue);
    JSClass<JSWebController>::CustomMethod("backOrForward", &JSWebController::BackOrForward);
    JSClass<JSWebController>::CustomMethod("zoomIn", &JSWebController::ZoomIn);
    JSClass<JSWebController>::CustomMethod("zoomOut", &JSWebController::ZoomOut);
    JSClass<JSWebController>::CustomMethod("getPageHeight", &JSWebController::GetPageHeight);
    JSClass<JSWebController>::CustomMethod("getTitle", &JSWebController::GetTitle);
    JSClass<JSWebController>::CustomMethod("createWebMessagePorts", &JSWebController::CreateWebMessagePorts);
    JSClass<JSWebController>::CustomMethod("postMessage", &JSWebController::PostWebMessage);
    JSClass<JSWebController>::CustomMethod("getWebId", &JSWebController::GetWebId);
    JSClass<JSWebController>::CustomMethod("getDefaultUserAgent", &JSWebController::GetDefaultUserAgent);
    JSClass<JSWebController>::CustomMethod("searchAllAsync", &JSWebController::SearchAllAsync);
    JSClass<JSWebController>::CustomMethod("clearMatches", &JSWebController::ClearMatches);
    JSClass<JSWebController>::CustomMethod("searchNext", &JSWebController::SearchNext);
    JSClass<JSWebController>::Bind(globalObj, JSWebController::Constructor, JSWebController::Destructor);
    JSWebCookie::JSBind(globalObj);
    JSHitTestValue::JSBind(globalObj);
    JSWebMessagePort::JSBind(globalObj);
    JSWebMessageEvent::JSBind(globalObj);
}

void JSWebController::Constructor(const JSCallbackInfo& args)
{
    auto webController = Referenced::MakeRefPtr<JSWebController>();
    webController->IncRefCount();
    RefPtr<WebController> controller = AceType::MakeRefPtr<WebController>();
    webController->SetController(controller);
    args.SetReturnValue(Referenced::RawPtr(webController));
}

void JSWebController::Destructor(JSWebController* webController)
{
    if (webController != nullptr) {
        webController->DecRefCount();
    }
}

void JSWebController::Reload() const
{
    if (webController_) {
        webController_->Reload();
    }
}

void JSWebController::CreateWebMessagePorts(const JSCallbackInfo& args)
{
    LOGI("JSWebController CreateWebMessagePorts");
    ContainerScope scope(instanceId_);
    if (webController_) {
        std::vector<RefPtr<WebMessagePort>> ports;
        webController_->CreateMsgPorts(ports);
        // 2: port size check.
        if (ports.size() != 2) {
            LOGE("JSWebController ports size wrong");
        }
        JSRef<JSObject> jsPort0 = JSClass<JSWebMessagePort>::NewInstance();
        auto port0 = Referenced::Claim(jsPort0->Unwrap<JSWebMessagePort>());
        port0->SetWebMessagePort(ports.at(0));

        JSRef<JSObject> jsPort1 = JSClass<JSWebMessagePort>::NewInstance();
        auto port1 = Referenced::Claim(jsPort1->Unwrap<JSWebMessagePort>());
        port1->SetWebMessagePort(ports.at(1));

        JSRef<JSArray> result = JSRef<JSArray>::New();
        result->SetValueAt(0, jsPort0);
        result->SetValueAt(1, jsPort1);
        args.SetReturnValue(result);
    } else {
        LOGE("JSWebController webcontroller its null");
    }
}

void JSWebController::PostWebMessage(const JSCallbackInfo& args)
{
    LOGI("JSWebController PostMessage");
    if (args.Length() < 1 || !args[0]->IsObject()) {
        LOGE("invalid postMessage params");
        return;
    }

    JSRef<JSObject> obj = JSRef<JSObject>::Cast(args[0]);
    std::string uri;
    if (!ConvertFromJSValue(obj->GetProperty("uri"), uri)) {
        LOGE("invalid uri param");
        return;
    }

    JSRef<JSVal> jsPorts = obj->GetProperty("message");
    if (!jsPorts->IsObject()) {
        LOGE("invalid message param");
        return;
    }
    auto jsRes = JSRef<JSObject>::Cast(jsPorts)->Unwrap<JSWebMessageEvent>();
    std::string eventData = jsRes->GetDataInternal();
    std::vector<RefPtr<JSWebMessagePort>> eventPorts = jsRes->GetPortsInternal();
    std::vector<RefPtr<WebMessagePort>> sendPorts;
    for (auto jsport : eventPorts) {
        auto webPort = jsport->GetWebMessagePort();
        if (webPort) {
            sendPorts.push_back(webPort);
        }
    }

    if (webController_ && sendPorts.size() >= 1) {
        webController_->PostWebMessage(eventData, sendPorts, uri);
    }
}

void JSWebController::LoadUrl(const JSCallbackInfo& args)
{
    ContainerScope scope(instanceId_);
    if (args.Length() < 1 || !args[0]->IsObject()) {
        LOGW("invalid url params");
        return;
    }

    JSRef<JSObject> obj = JSRef<JSObject>::Cast(args[0]);
    JSRef<JSVal> valUrl = obj->GetProperty("url");
    std::string url;
    if (valUrl->IsObject()) {
        // same as src process of JSWeb::Create
        std::string webSrc;
        if (!JSViewAbstract::ParseJsMedia(valUrl, webSrc)) {
            LOGE("JSWebController failed to parse url object");
            return;
        }
        auto np = webSrc.find_first_of("/");
        url = (np == std::string::npos) ? webSrc : webSrc.erase(np, 1);
    } else if (!ConvertFromJSValue(valUrl, url)) {
        LOGW("can't find url.");
        return;
    }

    JSRef<JSVal> headers = obj->GetProperty("headers");
    std::map<std::string, std::string> httpHeaders;
    if (headers->IsArray()) {
        JSRef<JSArray> array = JSRef<JSArray>::Cast(headers);
        for (size_t i = 0; i < array->Length(); i++) {
            JSRef<JSVal> jsValue = array->GetValueAt(i);
            if (!jsValue->IsObject()) {
                continue;
            }
            JSRef<JSObject> obj = JSRef<JSObject>::Cast(jsValue);
            std::string key;
            if (!ConvertFromJSValue(obj->GetProperty("headerKey"), key)) {
                LOGW("can't find key at index %{public}zu of additionalHttpHeaders, so skip it.", i);
                continue;
            }
            std::string value;
            if (!ConvertFromJSValue(obj->GetProperty("headerValue"), value)) {
                LOGW("can't find value at index %{public}zu of additionalHttpHeaders, so skip it.", i);
                continue;
            }
            httpHeaders[key] = value;
        }
    }
    if (webController_) {
        webController_->LoadUrl(url, httpHeaders);
    }
    LOGI("JSWebController load url:%{public}s, httpHeaders:%{public}d", url.c_str(), (int)httpHeaders.size());
}

void JSWebController::ExecuteTypeScript(const JSCallbackInfo& args)
{
    LOGI("JSWebController execute typescript");
    ContainerScope scope(instanceId_);
    if (args.Length() < 1 || !args[0]->IsObject()) {
        LOGW("invalid execute params");
        return;
    }

    JSRef<JSObject> obj = JSRef<JSObject>::Cast(args[0]);
    std::string script;
    if (!ConvertFromJSValue(obj->GetProperty("script"), script)) {
        LOGW("can't find script.");
        return;
    }
    JSRef<JSVal> tsCallback = obj->GetProperty("callback");
    std::function<void(std::string)> callback = nullptr;
    if (tsCallback->IsFunction()) {
        auto jsCallback =
            AceType::MakeRefPtr<JsWebViewFunction>(JSRef<JSFunc>::Cast(tsCallback));
        callback = [execCtx = args.GetExecutionContext(), func = std::move(jsCallback)](std::string result) {
            JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
            ACE_SCORING_EVENT("ExecuteTypeScript CallBack");
            LOGI("About to call ExecuteTypeScript callback method on js");
            func->Execute(result);
        };
    }
    if (webController_) {
        webController_->ExecuteTypeScript(script, std::move(callback));
    }
}

void JSWebController::LoadDataWithBaseUrl(const JSCallbackInfo& args)
{
    ContainerScope scope(instanceId_);
    if (args.Length() >= 1 && args[0]->IsObject()) {
        JSRef<JSObject> obj = JSRef<JSObject>::Cast(args[0]);

        std::string data;
        if (!ConvertFromJSValue(obj->GetProperty("data"), data)) {
            return;
        }

        std::string mimeType;
        if (!ConvertFromJSValue(obj->GetProperty("mimeType"), mimeType)) {
            return;
        }

        std::string encoding;
        if (!ConvertFromJSValue(obj->GetProperty("encoding"), encoding)) {
            return;
        }

        std::string baseUrl;
        std::string historyUrl;
        ConvertFromJSValue(obj->GetProperty("baseUrl"), baseUrl);
        ConvertFromJSValue(obj->GetProperty("historyUrl"), historyUrl);
        if (webController_) {
            webController_->LoadDataWithBaseUrl(baseUrl, data, mimeType, encoding, historyUrl);
        }
    }
}

void JSWebController::Backward(const JSCallbackInfo& args)
{
    LOGI("JSWebController Start backward.");
    ContainerScope scope(instanceId_);
    if (webController_) {
        webController_->Backward();
    }
}

void JSWebController::Forward(const JSCallbackInfo& args)
{
    LOGI("JSWebController Start forward.");
    ContainerScope scope(instanceId_);
    if (webController_) {
        webController_->Forward();
    }
}

void JSWebController::AccessStep(const JSCallbackInfo& args)
{
    LOGI("JSWebController start accessStep.");
    ContainerScope scope(instanceId_);
    int32_t step = 0;
    if (args.Length() < 1 || !ConvertFromJSValue(args[0], step)) {
        LOGE("AccessStep parameter is invalid.");
        return;
    }
    if (webController_) {
        auto canAccess = webController_->AccessStep(step);
        auto jsVal = JSVal(ToJSValue(canAccess));
        auto returnValue = JSRef<JSVal>::Make(jsVal);
        args.SetReturnValue(returnValue);
    }
}

void JSWebController::AccessBackward(const JSCallbackInfo& args)
{
    LOGI("JSWebController start accessBackward.");
    ContainerScope scope(instanceId_);
    if (webController_) {
        auto canAccess = webController_->AccessBackward();
        auto jsVal = JSVal(ToJSValue(canAccess));
        auto returnValue = JSRef<JSVal>::Make(jsVal);
        args.SetReturnValue(returnValue);
    }
}

void JSWebController::AccessForward(const JSCallbackInfo& args)
{
    LOGI("JSWebController start accessForward.");
    ContainerScope scope(instanceId_);
    if (webController_) {
        auto canAccess = webController_->AccessForward();
        auto jsVal = JSVal(ToJSValue(canAccess));
        auto returnValue = JSRef<JSVal>::Make(jsVal);
        args.SetReturnValue(returnValue);
    }
}

void JSWebController::ClearHistory(const JSCallbackInfo& args)
{
    LOGI("JSWebController clear navigation history.");
    ContainerScope scope(instanceId_);
    if (webController_) {
        webController_->ClearHistory();
    }
}

void JSWebController::Refresh(const JSCallbackInfo& args)
{
    LOGI("JSWebController Refresh");
    ContainerScope scope(instanceId_);
    if (webController_) {
        webController_->Refresh();
    }
}

void JSWebController::StopLoading(const JSCallbackInfo& args)
{
    LOGI("JSWebController StopLoading");
    ContainerScope scope(instanceId_);
    if (webController_) {
        webController_->StopLoading();
    }
}

void JSWebController::GetHitTestResult(const JSCallbackInfo& args)
{
    LOGI("JSWebController get his test result");
    ContainerScope scope(instanceId_);
    if (webController_) {
        int result = webController_->GetHitTestResult();
        args.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(result)));
    }
}

void JSWebController::GetHitTestValue(const JSCallbackInfo& args)
{
    LOGI("JSWebController Start GetHitTestValue");
    ContainerScope scope(instanceId_);
    if (!webController_) {
        return;
    }
    HitTestResult hitResult;
    webController_->GetHitTestValue(hitResult);
    JSRef<JSObject> resultObj = JSClass<JSHitTestValue>::NewInstance();
    auto result = Referenced::Claim(resultObj->Unwrap<JSHitTestValue>());
    result->SetType(hitResult.GetHitType());
    result->SetExtra(hitResult.GetExtraData());
    args.SetReturnValue(resultObj);
}

void JSWebController::GetCookieManager(const JSCallbackInfo& args)
{
    LOGI("JSWebController Start GetCookieManager");
    ContainerScope scope(instanceId_);
    if (webController_) {
        if (!webController_->GetCookieManager()) {
            return;
        }
        if (!jsWebCookieInit_) {
            jsWebCookie_ = JSClass<JSWebCookie>::NewInstance();
            auto jsWebCookieVal = Referenced::Claim(jsWebCookie_->Unwrap<JSWebCookie>());
            jsWebCookieVal->SetWebCookie(webController_->GetCookieManager());
            jsWebCookieInit_ = true;
        }
        args.SetReturnValue(jsWebCookie_);
    }
}

void JSWebController::BackOrForward(const JSCallbackInfo& args)
{
    LOGI("JSWebController BackOrForward");
    ContainerScope scope(instanceId_);
    int32_t step = 0;
    if (args.Length() < 1 || !ConvertFromJSValue(args[0], step)) {
        LOGE("BackOrForward parameter is invalid.");
        return;
    }
    if (webController_) {
        webController_->BackOrForward(step);
    }
}

void JSWebController::ZoomIn(const JSCallbackInfo& args)
{
    LOGI("JSWebController ZoomIn");
    ContainerScope scope(instanceId_);
    if (webController_) {
        bool result = webController_->ZoomIn();
        args.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(result)));
    }
}

void JSWebController::ZoomOut(const JSCallbackInfo& args)
{
    LOGI("JSWebController ZoomOut");
    ContainerScope scope(instanceId_);
    if (webController_) {
        bool result = webController_->ZoomOut();
        args.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(result)));
    }
}

void JSWebController::GetPageHeight(const JSCallbackInfo& args)
{
    LOGI("JSWebController GetPageHeight");
    ContainerScope scope(instanceId_);
    if (webController_) {
        int result = webController_->GetPageHeight();
        args.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(result)));
    }
}

void JSWebController::GetTitle(const JSCallbackInfo& args)
{
    LOGI("JSWebController GetTitle");
    ContainerScope scope(instanceId_);
    if (webController_) {
        std::string result = webController_->GetTitle();
        args.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(result)));
    }
}

void JSWebController::GetWebId(const JSCallbackInfo& args)
{
    LOGI("JSWebController GetWebId");
    ContainerScope scope(instanceId_);
    if (webController_) {
        int result = webController_->GetWebId();
        args.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(result)));
    }
}

void JSWebController::GetDefaultUserAgent(const JSCallbackInfo& args)
{
    LOGI("JSWebController GetDefaultUserAgent");
    ContainerScope scope(instanceId_);
    if (webController_) {
        std::string result = webController_->GetDefaultUserAgent();
        args.SetReturnValue(JSRef<JSVal>::Make(ToJSValue(result)));
    }
}

void JSWebController::SetJavascriptCallBackImpl()
{
    if (!webController_) {
        LOGE("webController_ is null");
        return;
    }

    LOGI("JSWebController set webview javascript CallBack");
    WebController::JavaScriptCallBackImpl callback =
        [weak = WeakClaim(this)](
        const std::string& objectName, const std::string& objectMethod,
        const std::vector<std::shared_ptr<WebJSValue>>& args) {
        auto jsWebController = weak.Upgrade();
        if (jsWebController == nullptr) {
            return std::make_shared<WebJSValue>(WebJSValue::Type::NONE);
        }
        return jsWebController->GetJavaScriptResult(objectName, objectMethod, args);
    };
    webController_->SetJavaScriptCallBackImpl(std::move(callback));
}

void JSWebController::AddJavascriptInterface(const JSCallbackInfo& args)
{
    LOGI("JSWebController add js interface");
    ContainerScope scope(instanceId_);
    if (args.Length() < 1 || !args[0]->IsObject()) {
        return;
    }
    if (webController_ == nullptr) {
        LOGW("JSWebController not ready");
        return;
    }
    // Init webview callback
    SetJavascriptCallBackImpl();

    // options
    JSRef<JSObject> obj = JSRef<JSObject>::Cast(args[0]);
    // options.name
    std::string objName;
    if (!ConvertFromJSValue(obj->GetProperty("name"), objName)) {
        return;
    }
    // options.obj
    JSRef<JSVal> jsClassObj = obj->GetProperty("object");
    if (!jsClassObj->IsObject()) {
        LOGW("JSWebController param obj is not object");
        return;
    }
    if (objectorMap_.find(objName) == objectorMap_.end()) {
        objectorMap_[objName] = JSRef<JSObject>::Cast(jsClassObj);
    }
    // options.methodList
    std::vector<std::string> methods;
    JSRef<JSVal> methodList = obj->GetProperty("methodList");
    JSRef<JSArray> array = JSRef<JSArray>::Cast(methodList);
    if (array->IsArray()) {
        for (size_t i = 0; i < array->Length(); i++) {
            JSRef<JSVal> method = array->GetValueAt(i);
            if (method->IsString()) {
                methods.push_back(method->ToString());
            }
        }
    }

    webController_->AddJavascriptInterface(objName, methods);
}

void JSWebController::InitJavascriptInterface()
{
    if (!webController_) {
        LOGW("JSWebController not ready");
        return;
    }
    // Init webview callback
    SetJavascriptCallBackImpl();
    for (auto& entry : methods_) {
        webController_->AddJavascriptInterface(entry.first, entry.second);
    }
}

void JSWebController::SetJavascriptInterface(const JSCallbackInfo& args)
{
    if (args.Length() < 1 || !args[0]->IsObject()) {
        return;
    }
    if (!webController_) {
        LOGW("JSWebController not ready");
        return;
    }
    // options
    JSRef<JSObject> obj = JSRef<JSObject>::Cast(args[0]);
    // options.name
    std::string objName;
    if (!ConvertFromJSValue(obj->GetProperty("name"), objName)) {
        return;
    }
    // options.obj
    JSRef<JSVal> jsClassObj = obj->GetProperty("object");
    if (!jsClassObj->IsObject()) {
        LOGW("JSWebController param obj is not object");
        return;
    }
    objectorMap_[objName] = JSRef<JSObject>::Cast(jsClassObj);
    std::vector<std::string> methods;
    methods_.clear();
    JSRef<JSVal> methodList = obj->GetProperty("methodList");
    JSRef<JSArray> array = JSRef<JSArray>::Cast(methodList);
    if (array->IsArray()) {
        for (size_t i = 0; i < array->Length(); i++) {
            JSRef<JSVal> method = array->GetValueAt(i);
            if (method->IsString()) {
                methods.emplace_back(method->ToString());
            }
        }
    }
    methods_[objName] = methods;

    webController_->SetInitJavascriptInterface([weak = WeakClaim(this)]() {
        auto jsWebcontroller = weak.Upgrade();
        if (jsWebcontroller) {
            jsWebcontroller->InitJavascriptInterface();
        }
    });
}

void JSWebController::RemoveJavascriptInterface(const JSCallbackInfo& args)
{
    LOGI("JSWebController remove js interface");
    ContainerScope scope(instanceId_);
    std::string objName;
    if (args.Length() < 1 || !ConvertFromJSValue(args[0], objName)) {
        return;
    }
    if (objectorMap_.find(objName) == objectorMap_.end()) {
        return;
    }
    objectorMap_.erase(objName);
    if (webController_) {
        webController_->RemoveJavascriptInterface(objName, {});
    }
}

void JSWebController::OnInactive(const JSCallbackInfo& args)
{
    ContainerScope scope(instanceId_);
    if (webController_) {
        webController_->OnInactive();
    }
}

void JSWebController::OnActive(const JSCallbackInfo& args)
{
    ContainerScope scope(instanceId_);
    if (webController_) {
        webController_->OnActive();
    }
}

void JSWebController::Zoom(const JSCallbackInfo& args)
{
    ContainerScope scope(instanceId_);
    float factor = 1.0;
    if (args.Length() < 1 || !ConvertFromJSValue(args[0], factor)) {
        LOGE("Zoom parameter is invalid.");
        return;
    }
    if (webController_) {
        webController_->Zoom(factor);
    }
}

void JSWebController::RequestFocus(const JSCallbackInfo& args)
{
    LOGI("JSWebController request focus");
    ContainerScope scope(instanceId_);
    if (webController_) {
        webController_->RequestFocus();
    }
}

void JSWebController::SearchAllAsync(const JSCallbackInfo& args)
{
    ContainerScope scope(instanceId_);
    std::string searchStr;
    if (args.Length() < 1 || !ConvertFromJSValue(args[0], searchStr)) {
        LOGE("SearchAllAsync parameter is invalid.");
        return;
    }
    if (webController_) {
        webController_->SearchAllAsync(searchStr);
    }
}
void JSWebController::ClearMatches(const JSCallbackInfo& args)
{
    ContainerScope scope(instanceId_);
    if (webController_) {
        webController_->ClearMatches();
    }
}
void JSWebController::SearchNext(const JSCallbackInfo& args)
{
    ContainerScope scope(instanceId_);
    bool forward = false;
    if (args.Length() < 1 || !ConvertFromJSValue(args[0], forward)) {
        LOGE("SearchNext parameter is invalid.");
        return;
    }

    if (webController_) {
        webController_->SearchNext(forward);
    }
}
} // namespace OHOS::Ace::Framework
