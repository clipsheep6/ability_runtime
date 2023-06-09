package com.huawei.operation;

import com.huawei.entity.vo.ciinfo.codecheck.CICodeCheck;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.springframework.stereotype.Component;

import static com.huawei.enums.CiCollectionName.CODE_CHECK;

/**
 * @Description ci_info codecheck库 操作类
 * @ClassName com.huawei.operation
 * @since 2022/11/8  11:14
 **/
@Component
public class CodeCheckOperation {
    @Autowired
    @Qualifier("ciWriteMongoTemplate")
    private MongoTemplate mongoTemplate;

    /**
     * 通过uuid获取码云codeCheck回写数据
     *
     * @param uuid uuid
     * @return CICodeCheck
     */
    public CICodeCheck getCICodeCheckByUuid(String uuid) {
        return mongoTemplate.findOne(Query.query(Criteria.where("uuid").is(uuid)), CICodeCheck.class, CODE_CHECK);
    }

    /**
     * 通过uuid插入码云codeCheck回写数据
     *
     * @param ciCodeCheck ciCodeCheck
     */
    public void saveCICodeCheck(CICodeCheck ciCodeCheck) {
        mongoTemplate.save(ciCodeCheck, CODE_CHECK);
    }

    /**
     * 通过uuid替换码云codeCheck回写数据
     *
     * @param ciCodeCheck ciCodeCheck
     */
    public void replaceCICodeCheckByUuid(CICodeCheck ciCodeCheck) {
        mongoTemplate.findAndReplace(Query.query(Criteria.where("uuid").is(ciCodeCheck.getUuid())), ciCodeCheck, CODE_CHECK);
    }

    /**
     * 更新整个对象，依次更新单个PR结果
     *
     * @param uuid            uuid
     * @param codeCheckResult codeCheckResult
     */
    public void findAndReplacePRInnerResult(String uuid, CICodeCheck codeCheckResult) {
        Criteria criteria = Criteria.where("uuid").is(uuid);
        Query query = Query.query(criteria);
        mongoTemplate.findAndReplace(query, codeCheckResult, CODE_CHECK);
    }
}
