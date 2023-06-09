package com.huawei.operation;

import com.huawei.ci.common.entity.CodeCheckResultSummary;
import com.huawei.entity.pojo.CodeCheckResultSummaryVo;
import com.huawei.enums.CodeCheckCollectionName;
import com.huawei.enums.CodeCheckStatus;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.data.mongodb.core.FindAndReplaceOptions;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.springframework.data.mongodb.core.query.Update;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * 单个增量任务检查详情操作类
 *
 * @since 2022/11/8
 */
@Component
public class IncResultSummaryOperation {
    @Autowired
    @Qualifier("codeCheckMongoTemplate")
    private MongoTemplate mongoTemplate;

    /**
     * updateCodeCheckStatus
     *
     * @param uuid   uuid
     * @param taskId taskId
     * @param status status
     */
    public void updateCodeCheckStatus(String uuid, String taskId, CodeCheckStatus status) {
        Criteria criteria = Criteria.where("uuid").is(uuid).and("taskId").is(taskId);
        Update codeCheckStatus = Update.update("codeCheckStatus", status.value());
        mongoTemplate.updateFirst(Query.query(criteria), codeCheckStatus, CodeCheckCollectionName.TASK_INC_RESULT_SUMMARY);
    }

    /**
     * insertSummary
     *
     * @param summary summary
     */
    public void insertSummary(CodeCheckResultSummary summary) {
        mongoTemplate.save(summary, CodeCheckCollectionName.TASK_INC_RESULT_SUMMARY);
    }

    /**
     * 增量模式代码检查概要
     *
     * @param summary 对象信息
     */
    public void save(CodeCheckResultSummary summary) {
        Criteria criteria = Criteria.where("uuid").is(summary.getUuid()).and("taskId").is(summary.getTaskId());
        Query query = Query.query(criteria);
        FindAndReplaceOptions upsert = new FindAndReplaceOptions().upsert();
        mongoTemplate.findAndReplace(query, summary, upsert, CodeCheckCollectionName.TASK_INC_RESULT_SUMMARY);
    }

    /**
     * 查询
     *
     * @param taskId taskId
     * @return {@link CodeCheckResultSummary}
     */
    public CodeCheckResultSummary getByTaskId(String taskId) {
        return mongoTemplate.findOne(Query.query(Criteria.where("taskId").is(taskId)), CodeCheckResultSummary.class,
                CodeCheckCollectionName.TASK_INC_RESULT_SUMMARY);
    }

    /**
     * getByUuid
     *
     * @param uuid uuid
     * @return List<CodeCheckResultSummary>
     */
    public List<CodeCheckResultSummary> getByUuid(String uuid) {
        return mongoTemplate.find(Query.query(Criteria.where("uuid").is(uuid)), CodeCheckResultSummary.class,
                CodeCheckCollectionName.TASK_INC_RESULT_SUMMARY);
    }

    /**
     * getByUuidAndTaskId
     *
     * @param uuid   uuid
     * @param taskId taskId
     * @return CodeCheckResultSummaryVo
     */
    public CodeCheckResultSummaryVo getByUuidAndTaskId(String uuid, String taskId) {
        Query query = Query.query(Criteria.where("uuid").is(uuid).and("taskId").is(taskId));
        return mongoTemplate.findOne(query, CodeCheckResultSummaryVo.class,
                CodeCheckCollectionName.TASK_INC_RESULT_SUMMARY);
    }

    /**
     * removeByUuids
     *
     * @param uuids uuids
     * @return long
     */
    public long removeByUuids(List<String> uuids) {
        Query query = Query.query(Criteria.where("uuid").in(uuids));
        return mongoTemplate.remove(query, CodeCheckCollectionName.TASK_INC_RESULT_SUMMARY).getDeletedCount();
    }

    /**
     * removeSummaryByDate
     *
     * @param date date
     * @return long
     */
    public long removeSummaryByDate(String date) {
        Query query = Query.query(Criteria.where("date").lt(date));
        return mongoTemplate.remove(query, CodeCheckCollectionName.TASK_INC_RESULT_SUMMARY).getDeletedCount();
    }

    /**
     * updateIgnoreCount
     *
     * @param taskId taskId
     * @param uuid   uuid
     * @param size   size
     */
    public void updateIgnoreCount(String taskId, String uuid, int size) {
        Criteria criteria = Criteria.where("taskId").is(taskId).and("uuid").is(uuid);
        Update ignoreCount = Update.update("ignoreCount", size);
        mongoTemplate.updateFirst(Query.query(criteria), ignoreCount, CodeCheckCollectionName.TASK_INC_RESULT_SUMMARY);
    }

    /**
     * updateIncResultSummary
     *
     * @param codeCheckResultSummary codeCheckResultSummary
     */
    public void updateIncResultSummary(CodeCheckResultSummary codeCheckResultSummary) {
        Query query = Query.query(Criteria.where("taskId").is(codeCheckResultSummary.getTaskId()).and("uuid").is(codeCheckResultSummary));
        mongoTemplate.findAndReplace(query, codeCheckResultSummary);
    }

    /**
     * getByUrlAndUUid
     *
     * @param prUrl prUrl
     * @param uuid  uuid
     * @return CodeCheckResultSummaryVo
     */
    public CodeCheckResultSummaryVo getByUrlAndUUid(String prUrl, String uuid) {
        Criteria criteria = Criteria.where("uuid").is(uuid).and("mrUrl").is(prUrl);
        Query query = Query.query(criteria);
        CodeCheckResultSummaryVo currentOne = mongoTemplate.findOne(query, CodeCheckResultSummaryVo.class, CodeCheckCollectionName.TASK_INC_RESULT_SUMMARY);
        return currentOne;
    }

    /**
     * getAllSummaryVo
     *
     * @return List<CodeCheckResultSummaryVo>
     */
    public List<CodeCheckResultSummaryVo> getAllSummaryVo() {
        List<CodeCheckResultSummaryVo> all = mongoTemplate.findAll(CodeCheckResultSummaryVo.class, CodeCheckCollectionName.TASK_INC_RESULT_SUMMARY);
        return all;
    }

    /**
     * getSummaryByUuid
     *
     * @param uuid uuid
     * @return List<CodeCheckResultSummaryVo>
     */
    public List<CodeCheckResultSummaryVo> getSummaryByUuid(String uuid) {
        return mongoTemplate.find(Query.query(Criteria.where("uuid").is(uuid)), CodeCheckResultSummaryVo.class,
                CodeCheckCollectionName.TASK_INC_RESULT_SUMMARY);
    }
}
