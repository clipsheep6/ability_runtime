package com.huawei.operation;

import com.huawei.entity.vo.codecheck.codecheckdetails.CodeCheckResultDetailsVo;
import com.huawei.entity.vo.ciinfo.codecheck.DefectVo;
import com.huawei.entity.vo.ciinfo.event.QueryIncDetailModel;
import com.huawei.enums.CodeCheckCollectionName;
import org.apache.commons.lang.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.data.mongodb.core.FindAndReplaceOptions;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;

@Component
public class IncResultDetailsOperation {

    @Autowired
    @Qualifier("codeCheckMongoTemplate")
    private MongoTemplate mongoTemplate;

    private static final String PROGRAM_SPECIFICATION = "coding standards";


    /**
     * saveByUpsert
     *
     * @param defectVo defectVo
     */
    public void saveByUpsert(DefectVo defectVo, int dayOfMonth) {
        Criteria criteria = Criteria.where("uuid").is(defectVo.getUuid())
                .and("taskId").is(defectVo.getTaskId())
                .and("defectId").is(defectVo.getDefectId());
        Query query = Query.query(criteria);
        FindAndReplaceOptions upsert = new FindAndReplaceOptions().upsert();
        // 确定当前代码检查的数据要拆分在哪个表
        StringBuffer sbTable = doJudgementForCurrentData(dayOfMonth);
        mongoTemplate.findAndReplace(query, defectVo, upsert, String.valueOf(sbTable));
    }

    public void saveByUpsertInto(DefectVo defectVo) {
        Criteria criteria = Criteria.where("uuid").is(defectVo.getUuid())
                .and("taskId").is(defectVo.getTaskId())
                .and("defectId").is(defectVo.getDefectId());
        Query query = Query.query(criteria);
        FindAndReplaceOptions upsert = new FindAndReplaceOptions().upsert();
        mongoTemplate.findAndReplace(query, defectVo, upsert, CodeCheckCollectionName.TASK_INC_RESULT_DETAILS);
    }

    /**
     * 确定当前表需要拆分到哪个库中
     *
     * @param dayOfMonth 今天是这个月的哪天
     * @return
     */
    private StringBuffer doJudgementForCurrentData(int dayOfMonth) {
        // 首先确定拆分10个表，从0-9 从第9天开始 删除第一天的数据
        int tableName = dayOfMonth % 10;
        StringBuffer sb = new StringBuffer();
        sb.append(CodeCheckCollectionName.T_TASK_INC_RESULT_DETAILS).append(tableName);
        return sb;
    }



    /**
     * 删除
     *
     * @param taskId 任务id
     * @param uuid   uuid
     */
    public void remove(String uuid, String taskId) {
        Criteria criteria = Criteria.where("task_id").is(taskId).and("uuid").is(uuid);
        // 有可能多个表都有当前uuid下的taskid 任务的数据
        for (int i = 0; i < 10; i++) {
            String currentTable = CodeCheckCollectionName.T_TASK_INC_RESULT_DETAILS;
            String tableFullName = currentTable + i;
            mongoTemplate.remove(Query.query(criteria), tableFullName);
        }
    }


    /**
     * removeByUuids
     *
     * @param uuids uuids
     * @return long
     */
    public long removeByUuids(List<String> uuids) {
        Query query = Query.query(Criteria.where("uuid").in(uuids));
        return mongoTemplate.remove(query, CodeCheckCollectionName.T_TASK_INC_RESULT_DETAILS).getDeletedCount();
    }

    /**
     * getResultListByUuid
     *
     * @param uuid                uuid
     * @param taskId              taskId
     * @param queryIncDetailModel queryIncDetailModel
     * @return CodeCheckResultDetailsVo
     */
    public CodeCheckResultDetailsVo getResultListByUuid(String uuid, String taskId, QueryIncDetailModel queryIncDetailModel) {
        Criteria criteria = Criteria.where("uuid").is(uuid);
        criteria.and("taskId").is(taskId);
        if (StringUtils.isNotBlank(queryIncDetailModel.getDefectLevel())) {
            criteria.and("defectLevel").is(queryIncDetailModel.getDefectLevel());
        }
        if (StringUtils.isNotBlank(queryIncDetailModel.getDefectStatus())) {
            criteria.and("defectStatus").is(queryIncDetailModel.getDefectStatus());
        }
        if (StringUtils.isNotBlank(queryIncDetailModel.getFilePath())) {
            criteria.and("filepath").is(queryIncDetailModel.getFilePath());
        }
        if (StringUtils.isNotBlank(queryIncDetailModel.getFileName())) {
            criteria.and("fileName").is(queryIncDetailModel.getFileName());
        }
        if (StringUtils.isNotBlank(queryIncDetailModel.getRuleSystemTags())) {
            //因为codecheck华为编程规范返回的时空串
            if (PROGRAM_SPECIFICATION.equals(queryIncDetailModel.getRuleSystemTags())) {
                criteria.and("ruleSystemTags").is("");
            } else {
                criteria.and("ruleSystemTags").is(queryIncDetailModel.getRuleSystemTags());
            }
        }
        if (StringUtils.isNotBlank(queryIncDetailModel.getDefectContent())) {
            criteria.and("defectContent").is(queryIncDetailModel.getDefectContent());
        }
        if (StringUtils.isNotBlank(queryIncDetailModel.getRuleName())) {
            criteria.and("ruleName").is(queryIncDetailModel.getRuleName());
        }
        Query query = Query.query(criteria);
        query.skip((long) (queryIncDetailModel.getPageNum() - 1) * queryIncDetailModel.getPageSize());
        query.limit(queryIncDetailModel.getPageSize());
        // 总数量
        long count = 0;
        for (int i = 0; i < 10; i++) {
            count += mongoTemplate.count(Query.query(criteria),
                    CodeCheckCollectionName.T_TASK_INC_RESULT_DETAILS + i);
        }

        List<DefectVo> defectVos = new ArrayList<>();
        List<DefectVo> list = null;
        if (count > 0) {
            for (int i = 0; i < 10; i++) {
                defectVos.addAll(mongoTemplate.find(query, DefectVo.class,
                        CodeCheckCollectionName.T_TASK_INC_RESULT_DETAILS + i));
            }
        }
        return new CodeCheckResultDetailsVo(Long.valueOf(count).intValue(), defectVos);
    }
}
