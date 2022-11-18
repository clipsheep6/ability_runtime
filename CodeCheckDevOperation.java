package com.huawei.operation;


import com.huawei.entity.vo.ciinfo.event.CodeCheckDevCloud;
import com.huawei.enums.CiCollectionName;
import com.huawei.utils.TimeUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * @Description: 获取黄区 Codecheck内部打点数据
 * @ClassName: com.huawei.ci.portal.provider.operation.codecheck
 * @Author: hwx1123794/hexiangyun
 * @DateTime: 2022/7/27  15:56
 * @Params:
 **/
@Component
public class CodeCheckDevOperation {

    @Autowired
    @Qualifier("ciMongoTemplate")
    private MongoTemplate mongoTemplate;

    @Autowired
    private TimeUtils timeUtils;

    /**
     * 通过UUid 获取当前增量任务的内部codecheck数据
     *
     * @param uuid uuid
     * @return CodeCheckDevCloud
     */
    public CodeCheckDevCloud getDevCloudByUuid(String uuid) {
        Criteria criteria = Criteria.where("uuid").is(uuid);
        Query query = Query.query(criteria);
        return mongoTemplate.findOne(query, CodeCheckDevCloud.class);
    }

    /**
     * 获取时间范围内的codecheck内部检查结果
     *
     * @param dateBeforeStr
     */
    public List<CodeCheckDevCloud> getInnerCodeCheckVo(String dateBeforeStr) {
        Criteria criteria = Criteria.where("running.timestamp").gte(dateBeforeStr);
        Query query = Query.query(criteria);
        List<CodeCheckDevCloud> codeCheckDevClouds = mongoTemplate.find(query, CodeCheckDevCloud.class, CiCollectionName.CODE_CHECK_DEV_CLOUD);
        return codeCheckDevClouds;

    }
}
