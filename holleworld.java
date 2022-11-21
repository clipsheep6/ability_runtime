package com.huawei.test.controller;

import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RestController;

import static com.huawei.test.commons.constant.Constants.CODE_FORMAT_NAME;
import static com.huawei.test.commons.constant.Constants.CONSUMES_FORMAT_NAME;

@RestController
@RequestMapping(path = "/hello", produces = CODE_FORMAT_NAME, consumes = CONSUMES_FORMAT_NAME)
public class holleworld {
    @RequestMapping(value = "", method = RequestMethod.GET, produces = CODE_FORMAT_NAME)
    public String check() {
        return "HelloWorld";
    }
}
