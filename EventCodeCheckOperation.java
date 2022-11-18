package com.huawei.operation;

import com.huawei.entity.vo.ciinfo.codecheck.EventModel;
import com.huawei.entity.vo.ciinfo.event.EventVo;
import com.huawei.entity.vo.ciinfo.event.exportData.EventExportData;
import com.huawei.enums.CodeCheckCollectionName;
import org.apache.commons.lang.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.data.domain.Sort;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.aggregation.Aggregation;
import org.springframework.data.mongodb.core.aggregation.AggregationOperation;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.springframework.data.mongodb.core.query.Update;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

@Component
public class EventCodeCheckOperation {
    @Autowired
    @Qualifier("codeCheckMongoTemplate")
    private MongoTemplate mongoTemplate;

    /**
     * insert
     *
     * @param eventModel eventModel
     */
    public void insert(EventModel eventModel) {
        mongoTemplate.insert(eventModel, CodeCheckCollectionName.EVENT);
    }

    /**
     * delByUuid
     *
     * @param uuid uuid
     */
    public void delByUuid(String uuid) {
        Criteria criteria = Criteria.where("uuid").is(uuid);
        Query query = Query.query(criteria);
        mongoTemplate.remove(query, CodeCheckCollectionName.EVENT);
    }

    /**
     * getByCondition
     *
     * @param pageNum pageNum
     * @param pageSize pageSize
     * @param projectName projectName
     * @param branch branch
     * @param startTime startTime
     * @param endTime endTime
     * @param triggerUser triggerUser
     * @param buildFailType buildFailType
     * @return List<EventModel>
     */
    public List<EventModel> getByCondition(Integer pageNum, Integer pageSize, String projectName, String branch,
                                           String startTime, String endTime, String triggerUser, String buildFailType) {
        Criteria criteria = getCommonCriteria(projectName, branch, startTime, endTime, triggerUser, buildFailType);
        Query limit = Query.query(criteria).skip((pageNum - 1) * pageSize).limit(pageSize).with(Sort.by(
                Sort.Order.desc("timestamp")
        ));
        return mongoTemplate.find(limit, EventModel.class, CodeCheckCollectionName.EVENT);
    }

    /**
     * count
     *
     * @param projectName   projectName
     * @param branch        branch
     * @param startTime     startTime
     * @param endTime       endTime
     * @param triggerUser   triggerUser
     * @param buildFailType buildFailType
     * @return long
     */
    public long count(String projectName, String branch, String startTime, String endTime, String triggerUser,
                      String buildFailType) {
        Criteria criteria = getCommonCriteria(projectName, branch, startTime, endTime, triggerUser, buildFailType);
        return mongoTemplate.count(Query.query(criteria), EventModel.class, CodeCheckCollectionName.EVENT);

    }


    private Criteria getCommonCriteria(String projectName, String branch, String startTime, String endTime,
                                       String triggerUser, String buildFailType) {
        Criteria criteria = Criteria.where("_id").ne("");
        if (StringUtils.isNotBlank(projectName)) {
            criteria.and("projectName").is(projectName);
        }
        if (StringUtils.isNotBlank(branch)) {
            criteria.and("repoBranch").is(branch);
        }
        if (StringUtils.isBlank(startTime)) {
            startTime = "0";
        }
        if (StringUtils.isBlank(endTime)) {
            endTime = "9";
        }
        criteria.andOperator(Criteria.where("timestamp").gte(startTime), Criteria.where("timestamp").lt(endTime));
        if (StringUtils.isNotBlank(triggerUser)) {
            criteria.and("triggerUser").regex("^.*" + triggerUser + ".*$", "i");
        }
        if (StringUtils.isNotBlank(buildFailType)) {
            Criteria criteriaA = Criteria.where("builds.debug.buildFailType").is(buildFailType);
            Criteria criteriaB = Criteria.where("builds.test.buildFailType").is(buildFailType);
            criteria.orOperator(criteriaA, criteriaB);
        }
        return criteria;
    }

    /**
     * getEventById
     *
     * @param eventId eventId
     * @return EventModel
     */
    public EventModel getEventById(String eventId) {
        Criteria criteria = Criteria.where("id").is(eventId);
        return mongoTemplate.findOne(Query.query(criteria), EventModel.class, CodeCheckCollectionName.EVENT);
    }

    /**
     * getEventVoById
     *
     * @param eventId eventId
     * @return EventVo
     */
    public EventVo getEventVoById(String eventId) {
        Criteria criteria = Criteria.where("id").is(eventId);
        return mongoTemplate.findOne(Query.query(criteria), EventVo.class, CodeCheckCollectionName.EVENT);
    }

    /**
     * removeByDate
     *
     * @param monthAgo monthAgo
     * @return long
     */
    public long removeByDate(String monthAgo) {
        Criteria criteria = Criteria.where("timestamp").lte(monthAgo);
        return mongoTemplate.remove(Query.query(criteria), CodeCheckCollectionName.EVENT).getDeletedCount();
    }

    /**
     * getUuidsByDate
     *
     * @param monthAgo monthAgo
     * @return List<String>
     */
    public List<String> getUuidsByDate(String monthAgo) {
        Criteria criteria = Criteria.where("timestamp").lte(monthAgo);
        List<EventModel> eventModels = mongoTemplate.find(Query.query(criteria), EventModel.class, CodeCheckCollectionName.EVENT);
        return eventModels.stream().map(EventModel::getUuid).collect(Collectors.toList());
    }

    /**
     * getEventByUuid
     *
     * @param uuid uuid
     * @return EventModel
     */
    public EventModel getEventByUuid(String uuid) {
        Criteria criteria = Criteria.where("uuid").is(uuid);
        return mongoTemplate.findOne(Query.query(criteria), EventModel.class, CodeCheckCollectionName.EVENT);
    }

    /**
     * getEventByUuidTest
     *
     * @param uuid uuid
     * @return List<EventModel>
     */
    public List<EventModel> getEventByUuidTest(String uuid) {
        Criteria criteria = Criteria.where("uuid").is(uuid);
        return mongoTemplate.find(Query.query(criteria), EventModel.class, CodeCheckCollectionName.EVENT);
    }

    /**
     * updateCodeCheckInfo
     *
     * @param eventModel eventModel
     */
    public void updateCodeCheckInfo(EventModel eventModel) {
        Query query = Query.query(Criteria.where("uuid").is(eventModel.getUuid()));
        Update update = Update.update("codeCheckInfo", eventModel.getCodeCheckInfo());
        mongoTemplate.upsert(query, update, EventModel.class, CodeCheckCollectionName.EVENT);
    }

/*    *//**
     * 获取门禁pr结果
     *
     * @param buildResultModel 度量统计对象
     * @return 结果集
     *//*
    public List<EventResultVo> getEventsResults(BuildResultModel buildResultModel) {
        List<AggregationOperation> operations = new ArrayList<>();
        Criteria criteria = getCommonCriteria(buildResultModel.getProjectName(), buildResultModel.getBranch(),
                buildResultModel.getStartTime(), buildResultModel.getEndTime(), null, null);

        operations.add(Aggregation.match(criteria));
        operations.add(Aggregation.unwind("prMsg")); // 扁平化pr_msg
        operations.add(Aggregation.group("_id", "uuid", "projectName", "repoBranch", "result", "timestamp",
                "prMsg.url", "prMsg.committer", "prMsg.repoName"));

        operations.add(Aggregation.project("_id.uuid", "projectName", "repoBranch", "result", "timestamp",
                "url", "committer", "repoName").andExclude("_id._id"));
        if (StringUtils.isNotBlank(buildResultModel.getCommitter())) {
            operations.add(Aggregation.match(Criteria.where("committer").regex("^.*" + buildResultModel.getCommitter() + ".*$")));
        }
        if (StringUtils.isNotBlank(buildResultModel.getRepoName())) {
            operations.add(Aggregation.match(Criteria.where("repoName").regex("^.*" + buildResultModel.getRepoName() + ".*$")));
        }

        return mongoTemplate.aggregate(Aggregation.newAggregation(operations),
                CodeCheckCollectionName.EVENT, EventResultVo.class).getMappedResults();
    }

    *//**
     * 获取门禁触发次数
     *//*
    public List<EventResultVo> getEventTriggerNum(BuildResultModel buildResultModel) {
        List<AggregationOperation> operations = new ArrayList<>();
        Criteria criteria = getCommonCriteria(buildResultModel.getProjectName(), buildResultModel.getBranch(),
                buildResultModel.getStartTime(), buildResultModel.getEndTime(), null, null);

        operations.add(Aggregation.match(criteria));
        operations.add(Aggregation.unwind("prMsg")); // 扁平化pr_msg
        operations.add(Aggregation.group("repoBranch", "prMsg.repoName").count().as("count"));
        operations.add(Aggregation.project("repoName", "repoBranch", "count"));
        if (StringUtils.isNotBlank(buildResultModel.getRepoName())) {
            operations.add(Aggregation.match(Criteria.where("repoName").regex("^.*" + buildResultModel.getRepoName() + ".*$")));
        }

        return mongoTemplate.aggregate(Aggregation.newAggregation(operations),
                CodeCheckCollectionName.EVENT, EventResultVo.class).getMappedResults();
    }*/

    /**
     * getNoTimeData
     *
     * @return List<EventModel>
     */
    public List<EventModel> getNoTimeData() {
        Criteria timestamp = Criteria.where("timestamp").exists(false);
        Query query = Query.query(timestamp);
        query.fields().include("uuid").exclude("_id");
        return mongoTemplate.find(query, EventModel.class, CodeCheckCollectionName.EVENT);
    }

    /**
     * getEventsByDate
     *
     * @param date date
     * @return List<EventModel>
     */
    public List<EventModel> getEventsByDate(String date) {
        Criteria criteria = Criteria.where("timestamp").gte(date);
        Query query = Query.query(criteria);
        return mongoTemplate.find(query, EventModel.class, CodeCheckCollectionName.EVENT);
    }

    /**
     * 资料门禁获取当前关联的门禁任务详情信息
     *
     * @param projectName projectName
     * @param branch      branch
     * @param committer   committer
     * @return List<EventModel> 门禁详情
     */
    public List<EventModel> getEventByCondition(String projectName, String branch, String committer) {
        Criteria criteria = Criteria.where("projectName").is(projectName).and("repoBranch").is(branch);
        // 提交人信息
        if (StringUtils.isNotBlank(committer)) {
            criteria.and("triggerUser").regex(committer, "i");
        }
        Query query = Query.query(criteria);
        query.with(Sort.by(Sort.Direction.DESC, "timestamp"));
        return mongoTemplate.find(query, EventModel.class, CodeCheckCollectionName.EVENT);

    }

    /**
     * 获取所有的uuid信息
     *
     * @return List<EventModel>
     */
    public List<EventModel> getAllEvents() {
        List<EventModel> allEvents = mongoTemplate.findAll(EventModel.class, CodeCheckCollectionName.EVENT);
        return allEvents;
    }

    /**
     * 获取门禁导出数据
     */
    public List<EventExportData> getExportData(String projectName, String branch, String startTime, String endTime,
                                               String triggerUser, String buildFailType) {
        List<AggregationOperation> operations = new ArrayList<>();
        Criteria criteria = getCommonCriteria(projectName, branch, startTime, endTime, triggerUser, buildFailType);
        criteria.and("exportData").exists(true);
        operations.add(Aggregation.match(criteria));
        operations.add(Aggregation.sort(Sort.by(Sort.Order.desc("timestamp"))));
        operations.add(Aggregation.project("exportData.triggerUser", "exportData.timestamp",
                "exportData.duration", "exportData.builds", "exportData.codeCheckTotalResult", "exportData.prStatus",
                "exportData.prCommitter", "exportData.prUrl", "exportData.eventDetailUrl").andExclude("_id"));

        return mongoTemplate.aggregate(Aggregation.newAggregation(operations), CodeCheckCollectionName.EVENT,
                EventExportData.class).getMappedResults();
    }
}
