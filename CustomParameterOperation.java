package com.huawei.operation;

import com.huawei.entity.vo.codecheck.eventModule.CustomParameterVo;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.springframework.stereotype.Component;


@Component
public class CustomParameterOperation {

    @Autowired
    @Qualifier("codeCheckMongoTemplate")
    private MongoTemplate mongoTemplate;

    /**
     * 根据配置项查询自定义参数
     *
     * @param configuration 配置项
     * @return CustomParameterVo 自定义参数数据
     */
    public CustomParameterVo getCustomParameterByConfiguration(String configuration) {
        Query query = Query.query(Criteria.where("configuration").is(configuration));
        return mongoTemplate.findOne(query, CustomParameterVo.class);
    }
}
