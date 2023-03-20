package com.huawei.operation;


import com.huawei.entity.vo.ciinfo.event.EventVo;
import com.huawei.enums.CiCollectionName;
import org.apache.commons.lang.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.stream.Collectors;

@Component
public class EventOperation {
    @Autowired
    @Qualifier("ciMongoTemplate")
    private MongoTemplate mongoTemplate;

    private final String collectionName = CiCollectionName.EVENT;

    /**
     * getEventByUuid
     *
     * @param uuid uuid
     * @return EventVo
     */
    public EventVo getEventByUuid(String uuid) {
        return mongoTemplate.findOne(Query.query(Criteria.where("uuid").is(uuid)), EventVo.class);
    }

    /**
     * 获得五月二号之后的uuid
     *
     * @param monthAgo monthAgo
     * @return List<String>
     */
    public List<String> getUuidsByDate(String monthAgo) {

        Criteria criteria = Criteria.where("timestamp").gte(monthAgo);
        List<EventVo> eventVos = mongoTemplate.find(Query.query(criteria), EventVo.class);

        return eventVos.stream().map(EventVo::getUuid).collect(Collectors.toList());
    }

    /**
     * getEventByRepo
     *
     * @param offset    offset
     * @param limit     limit
     * @param startTime startTime
     * @param endTime   endTime
     * @param repoName  repoName
     * @return List<EventVo>
     */
    public List<EventVo> getEventByRepo(Integer offset, Integer limit, String startTime, String endTime, String repoName) {
        Criteria criteria = getCriteriaByRepo(startTime, endTime, repoName);
        Query query = Query.query(criteria);
        query.limit(limit).skip(offset);
        return mongoTemplate.find(query, EventVo.class, collectionName);
    }

    /**
     * getCountByRepo
     *
     * @param startTime startTime
     * @param endTime   endTime
     * @param repoName  repoName
     * @return long
     */
    public long getCountByRepo(String startTime, String endTime, String repoName) {
        Criteria criteria = getCriteriaByRepo(startTime, endTime, repoName);
        Query query = Query.query(criteria);
        return mongoTemplate.count(query, collectionName);
    }

    private Criteria getCriteriaByRepo(String startTime, String endTime, String repoName) {
        Criteria criteria = new Criteria();
        if (StringUtils.isNotBlank(repoName)){
            String repo = "/" + repoName + "/";
            criteria.and("causeUrl").regex(repo);
        }
        if (StringUtils.isNotBlank(startTime) && StringUtils.isBlank(endTime)) {
            criteria.and("end_timestamp").gte(startTime);
        } else if (StringUtils.isBlank(startTime) && StringUtils.isNotBlank(endTime)) {
            criteria.and("end_timestamp").lte(endTime);
        } else if (StringUtils.isNotBlank(startTime) && StringUtils.isNotBlank(endTime)) {
            criteria.and("end_timestamp").gte(startTime).lte(endTime);
        }
        criteria.and("result").exists(true).ne("");
        return criteria;
    }
}
