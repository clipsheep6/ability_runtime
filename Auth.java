package com.huawei.ci.gateway.common;


import com.huawei.ci.gateway.model.UserContext;

import java.io.IOException;

public interface Auth {
    UserContext auth(String au) throws IOException;
    String getLoginUrl();
}
