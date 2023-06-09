package com.huawei.operation;

import com.huawei.entity.vo.ciinfo.event.QueryStageModel;
import com.huawei.entity.vo.eventbuild.BuildStage;
import com.huawei.entity.vo.eventbuild.BuildStageVo;
import com.huawei.enums.CodeCheckCollectionName;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.aggregation.Aggregation;
import org.springframework.data.mongodb.core.aggregation.AggregationResults;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.springframework.stereotype.Component;

import java.util.List;

@Component
public class EventBuildAnalyseOperation {

    @Autowired
    @Qualifier("codeCheckMongoTemplate")
    private MongoTemplate mongoTemplate;

    public long countStageByUuid(String uuid, String component) {
        Criteria criteria = Criteria.where("uuid").is(uuid);
        criteria.and("component").is(component);
        return mongoTemplate.count(Query.query(criteria), BuildStage.class, CodeCheckCollectionName.EVENT_BUILD_STAGE);
    }

    public void save(BuildStage buildStage) {
        mongoTemplate.save(buildStage, CodeCheckCollectionName.EVENT_BUILD_STAGE);
    }

    public List<BuildStage> query(QueryStageModel queryStageModel) {
        Criteria criteria = getQuery(queryStageModel);
        Query q = Query.query(criteria);
        return mongoTemplate.find(q, BuildStage.class, CodeCheckCollectionName.EVENT_BUILD_STAGE);
    }

    public List<BuildStageVo> countBuildStageAvg(QueryStageModel queryStageModel) {
        Criteria criteria = getQuery(queryStageModel);
        Aggregation aggregation = Aggregation.newAggregation(
                Aggregation.match(criteria),//条件
                Aggregation.group("component")
                        .first("component").as("component")
                        .last("projectName").as("projectName")
                        .last("gitBranch").as("gitBranch")
                        .count().as("buildNumber")
                        .avg("init").as("init")
                        .avg("downloadCode").as("downloadCode")
                        .avg("fetchPr").as("fetchPr")
                        .avg("gitLfs").as("gitLfs")
                        .avg("preCompile").as("preCompile")
                        .avg("mainCompile").as("mainCompile")
                        .avg("afterCompile").as("afterCompile")
                        .avg("archive").as("archive")
                        .avg("upload").as("upload")
                        .avg("hitRate").as("hitRate")
        ).withOptions(Aggregation.newAggregationOptions().allowDiskUse(true).build());
        AggregationResults<BuildStageVo> aggregationResults = mongoTemplate.aggregate(aggregation, CodeCheckCollectionName.EVENT_BUILD_STAGE, BuildStageVo.class);
        return aggregationResults.getMappedResults();
    }

    public List<BuildStageVo> countBuildStageSum(QueryStageModel queryStageModel) {
        Criteria criteria = getQuery(queryStageModel);
        Aggregation aggregation = Aggregation.newAggregation(
                Aggregation.match(criteria),//条件
                Aggregation.group()
                        .last("projectName").as("projectName")
                        .last("gitBranch").as("gitBranch")
                        .count().as("buildNumber")
                        .sum("init").as("init")
                        .sum("downloadCode").as("downloadCode")
                        .sum("fetchPr").as("fetchPr")
                        .sum("gitLfs").as("gitLfs")
                        .sum("preCompile").as("preCompile")
                        .sum("mainCompile").as("mainCompile")
                        .sum("afterCompile").as("afterCompile")
                        .sum("archive").as("archive")
                        .sum("upload").as("upload")
        ).withOptions(Aggregation.newAggregationOptions().allowDiskUse(true).build());
        AggregationResults<BuildStageVo> aggregationResults = mongoTemplate.aggregate(aggregation, CodeCheckCollectionName.EVENT_BUILD_STAGE, BuildStageVo.class);
        return aggregationResults.getMappedResults();
    }

    private Criteria getQuery(QueryStageModel query) {
        Criteria criteria = new Criteria();
        if (!StringUtils.isEmpty(query.getProjectName())) {
            criteria.and("projectName").is(query.getProjectName());
        }
        if (!StringUtils.isEmpty(query.getManifestBranch())) {
            criteria.and("gitBranch").is(query.getManifestBranch());
        }
        if (!StringUtils.isEmpty(query.getComponent())) {
            criteria.and("component").is(query.getComponent());
        }
        if (!StringUtils.isEmpty(query.getStartTime()) && !StringUtils.isEmpty(query.getEndTime())) {
            criteria.and("timestamp").gte(query.getStartTime()).lte(query.getEndTime());
        }
        return criteria;
    }
}
