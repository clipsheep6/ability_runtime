package com.huawei.operation;


import com.huawei.entity.vo.ciinfo.event.PipelineVo;
import com.huawei.entity.vo.ciinfo.event.PipelineIdentity;
import com.huawei.entity.vo.ciinfo.event.PipelineQuery;
import com.huawei.entity.vo.ciinfo.event.QueryStageModel;
import com.huawei.enums.CiCollectionName;
import org.apache.commons.collections4.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;

@Component
public class PipelineOperation {
    @Autowired
    @Qualifier("ciMongoTemplate")
    private MongoTemplate mongoTemplate;

    /**
     * 查询流水线列表
     *
     * @param pipelineQuery 查询的条件
     * @return list
     */
    public List<PipelineVo> getPipelines(PipelineQuery pipelineQuery) {
        if (CollectionUtils.isEmpty(pipelineQuery.getPipelineIds())) {
            return Collections.emptyList();
        }
        Criteria criteria = Criteria.where("uuid").is(pipelineQuery.getUuid())
                .and("pipelineId").in(pipelineQuery.getPipelineIds());
        return mongoTemplate.find(Query.query(criteria), PipelineVo.class, CiCollectionName.PIPELINE);
    }

    /**
     * getPipelinesByComponent
     *
     * @param queryStageModel queryStageModel
     * @return List<PipelineVo>
     */
    public List<PipelineVo> getPipelinesByComponent(QueryStageModel queryStageModel) {
        Criteria criteria = Criteria.where("namespace").is(queryStageModel.getProjectName())
                .and("manifest_branch").is(queryStageModel.getManifestBranch())
                .and("component").is(queryStageModel.getComponent());
        if (!StringUtils.isEmpty(queryStageModel.getStartTime())) {
            criteria.and("timestamp").gte(queryStageModel.getStartTime()).lte(queryStageModel.getEndTime());
        }
        return mongoTemplate.find(Query.query(criteria), PipelineVo.class, CiCollectionName.PIPELINE);
    }

    /**
     * 获取
     *
     * @param queryStageModel queryStageModel
     * @return List<PipelineVo>
     */
    public List<PipelineVo> getUpgradeResultByDate(QueryStageModel queryStageModel){
        Criteria criteria = Criteria.where("upgradeResult").exists(true);
        if (!StringUtils.isEmpty(queryStageModel.getStartTime())) {
            criteria.and("timestamp").gte(queryStageModel.getStartTime()).lt(queryStageModel.getEndTime());
        }
        return mongoTemplate.find(Query.query(criteria), PipelineVo.class, CiCollectionName.PIPELINE);
    }

    /**
     * 根据 uuid和pipelineId的组合查询 对应的流水线信息
     *
     * @param allPipeline allPipeline
     * @return List<PipelineVo>
     */
    public List<PipelineVo> getPipeline(List<PipelineIdentity> allPipeline) {
        Criteria[] criteriaList = allPipeline.stream().map(identity ->
             Criteria.where("uuid").is(identity.getUuid()).and("pipelineId").is(identity.getPipelineId())
                    .and("component").is(identity.getComponent())
        ).toArray(Criteria[]::new);
        Criteria criteria = new Criteria();
        criteria.orOperator(criteriaList);
        Query query = Query.query(criteria);
        return mongoTemplate.find(query, PipelineVo.class,  CiCollectionName.PIPELINE);
    }
}
