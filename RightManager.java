package com.huawei.ci.gateway.common;

import com.huawei.ci.gateway.model.UserContext;
import com.netflix.config.DynamicPropertyFactory;
import io.vertx.core.http.HttpMethod;

public class RightManager {

    private DynamicPropertyFactory propertyFactory=DynamicPropertyFactory.getInstance();

    public static RightManager instance() {
        return new RightManager();
    }

    /**
     * todo 获取认证模式
     */
    public UserContext.AuthType getURLAuthType(String url, HttpMethod method){
        String model=propertyFactory.getStringProperty("ci.model","public").getValue();
        return UserContext.AuthType.PK;
    }
}
