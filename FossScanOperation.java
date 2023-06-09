package com.huawei.operation;

import com.huawei.ci.common.codecheck.RestCodeCheckUtils;
import com.huawei.entity.pojo.CodeCheckResultSummaryVo;
import com.huawei.entity.vo.codecheck.fossscan.FossScan;
import com.huawei.entity.vo.codecheck.fossscan.FossscanFragment;
import com.huawei.entity.vo.codecheck.fossscan.FossscanModel;
import com.huawei.entity.vo.codecheck.fossscan.Hit;
import com.huawei.enums.CodeCheckAccessConstant;
import com.huawei.enums.CodeCheckCollectionName;
import com.huawei.enums.CodeCheckConstants;
import com.huaweicloud.sdk.core.utils.JsonUtils;
import com.huaweicloud.sdk.core.utils.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.data.domain.Sort;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.springframework.data.mongodb.core.query.Update;
import org.springframework.stereotype.Component;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

@Component
public class FossScanOperation {

    private final static String HASH_FILE = "https://codecheck-ext.cn-north-4.myhuaweicloud.com/v2/opensource/file/content/";

    @Autowired
    @Qualifier("codeCheckMongoTemplate")
    private MongoTemplate mongoTemplate;

    @Autowired
    private RestCodeCheckUtils restCodeCheckUtils;

    /**
     * insertFossFragment
     *
     * @param fossscanFragment fossscanFragment
     * @return boolean
     */
    public boolean insertFossFragment(FossscanFragment fossscanFragment) {
        mongoTemplate.insert(fossscanFragment, CodeCheckCollectionName.FOSSSCAN_FRAGMENT);
        return true;
    }

    /**
     * queryFossscan
     *
     * @param taskId    taskId
     * @param uuid      uuid
     * @param date      date
     * @param isConfirm isConfirm
     * @param pageSize  pageSize
     * @param pageNum   pageNum
     * @return List<FossscanFragment>
     */
    public List<FossscanFragment> queryFossscan(String taskId, String uuid, String date, Boolean isConfirm, Integer pageSize, Integer pageNum) {
        Criteria criteria = Criteria.where("taskId").is(taskId).and("isConfirm").is(isConfirm);
        if (StringUtils.isEmpty(uuid)) {
            criteria.and("date").is(date);
        } else {
            criteria.and("uuid").is(uuid);
        }
        Query query = new Query(criteria);
        if (pageSize != null && pageNum != null) {
            int front = (pageNum - 1) * pageSize;
            query.skip(front);
            query.limit(pageSize);
        }
        return mongoTemplate.find(query, FossscanFragment.class, CodeCheckCollectionName.FOSSSCAN_FRAGMENT);
    }

    /**
     * queryFossscan
     *
     * @param taskIds   taskIds
     * @param uuid      uuid
     * @param isConfirm isConfirm
     * @return List<FossscanFragment>
     */
    public List<FossscanFragment> queryFossscan(List<String> taskIds, String uuid, Boolean isConfirm) {
        Query query = new Query(Criteria.where("taskId").in(taskIds).and("uuid").is(uuid).and("isConfirm").is(isConfirm));
        return mongoTemplate.find(query, FossscanFragment.class, CodeCheckCollectionName.FOSSSCAN_FRAGMENT);
    }

    /**
     * queryOneByKey
     *
     * @param issueKey issueKey
     * @return FossscanFragment
     */
    public FossscanFragment queryOneByKey(String issueKey) {
        Query query = new Query(Criteria.where("issueKey").is(issueKey).and("isConfirm").is(true)).with(Sort.by(Sort.Order.desc("_id")));
        return mongoTemplate.findOne(query, FossscanFragment.class, CodeCheckCollectionName.FOSSSCAN_FRAGMENT);
    }

   /* public void updateFoss(FossscanModel fossscanModel, String userId, String userName) {
        Criteria criteria = Criteria.where("defectId").in(fossscanModel.getDefectIds());
        if (!StringUtils.isEmpty(fossscanModel.getDate())) {
            criteria.and("date").is(fossscanModel.getDate());
        }
        Query query = new Query(criteria);
        FossscanFragment fossscanFragment = mongoTemplate.findOne(query, FossscanFragment.class, CodeCheckCollectionName.FOSSSCAN_FRAGMENT);
        Update update = new Update();
        update.set("fossType", fossscanModel.getFossType());
        update.set("componentName", fossscanModel.getComponentName());
        update.set("componentVersion", fossscanModel.getComponentVersion());
        update.set("isOpen", fossscanModel.getIsOpen());
        update.set("remarks", fossscanModel.getRemarks());
        update.set("result", fossscanModel.getResult());
        update.set("userId", userId);
        update.set("userName", userName);
        update.set("isConfirm", true);
        LocalDateTime time = LocalDateTime.now();
        update.set("confirmTime", time.format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss")));
        mongoTemplate.updateMulti(query, update, CodeCheckCollectionName.FOSSSCAN_FRAGMENT);
        StringBuilder ids = new StringBuilder(fossscanModel.getDefectIds().get(0) == null ? "" : fossscanModel.getDefectIds().get(0));
        for (int i = 1; i < fossscanModel.getDefectIds().size(); i++) {
            ids.append(",").append(fossscanModel.getDefectIds().get(i) == null ? "" : fossscanModel.getDefectIds().get(i));
        }

        if (fossscanFragment != null && !fossscanFragment.isConfirm()) {
            restCodeCheckUtils.updateDefectStatus(fossscanFragment.getTaskId(), ids.toString(), 2, CodeCheckConstants.REGION);
            if (fossscanModel.getDate() != null) {
                Query fossQuery = new Query(Criteria.where("taskId").is(fossscanFragment.getTaskId())
                        .and("date").is(fossscanFragment.getDate()));
                //修改summary
                CodeCheckResultSummaryVo codeCheckResultSummary = mongoTemplate.findOne(fossQuery, CodeCheckResultSummaryVo.class,
                        CodeCheckCollectionName.TASK_RESULT_SUMMARY);
                if (codeCheckResultSummary != null) {
                    int issueCount = codeCheckResultSummary.getIssueCount();
                    int ignoreCount = StringUtils.isEmpty(codeCheckResultSummary.getIgnoreCount()) ? 0 : Integer.parseInt(codeCheckResultSummary.getIgnoreCount());
                    Update fossUpdate = new Update();
                    fossUpdate.set("issueCount", issueCount - fossscanModel.getDefectIds().size());
                    fossUpdate.set("ignoreCount", ignoreCount + fossscanModel.getDefectIds().size());
                    mongoTemplate.updateMulti(fossQuery, fossUpdate, CodeCheckCollectionName.TASK_RESULT_SUMMARY);
                }
            }
        }
    }*/

    /**
     * count
     *
     * @param taskId      taskId
     * @param uuid        uuid
     * @param date        date
     * @param isConfirmed isConfirmed
     * @return long
     */
    public long count(String taskId, String uuid, String date, boolean isConfirmed) {
        Criteria criteria = Criteria.where("taskId").is(taskId).and("isConfirm").is(isConfirmed);
        if (StringUtils.isEmpty(uuid)) {
            criteria.and("date").is(date);
        } else {
            criteria.and("uuid").is(uuid);
        }
        Query query = new Query(criteria);
        return mongoTemplate.count(query, CodeCheckCollectionName.FOSSSCAN_FRAGMENT);
    }

/*    public Map<String, Object> fossscanOptionList(FossscanModel fossscanModel) {
        Query query = new Query(Criteria.where("defectId").in(fossscanModel.getDefectIds()));
        List<FossscanFragment> defectList = mongoTemplate.find(query, FossscanFragment.class, CodeCheckCollectionName.FOSSSCAN_FRAGMENT);
        List<String> componentList = new ArrayList<>();
        for (FossscanFragment defect : defectList) {
            List<FossScan> fossList = defect.getScanResults();
            if (fossList != null) {
                for (FossScan foss : fossList) {
                    if (foss.getComponentName() != null && foss.getComponentVersion() != null) {
                        componentList.add(foss.getComponentName() + " " + foss.getComponentVersion());
                    }
                }
            }
        }
        Map<String, Object> result = new HashMap<>();
        result.put("componentList", componentList.stream().distinct().collect(Collectors.toList()));
        return result;
    }*/

    /**
     * queryOpensource
     *
     * @param detectId detectId
     * @param hash     hash
     * @return List<Map < String, Object>>
     */
    public List<Map<String, Object>> queryOpensource(String detectId, String hash) {
        Query query = new Query(Criteria.where("defectId").is(detectId));
        FossscanFragment fossscanFragment = mongoTemplate.findOne(query, FossscanFragment.class, CodeCheckCollectionName.FOSSSCAN_FRAGMENT);
        FossScan fossfileItem = fossscanFragment.getScanResults().stream()
                .filter(fossScan -> fossScan.getHash().equals(hash)).collect(Collectors.toList()).get(0);
        String result = restCodeCheckUtils.queryOpenFile(hash, CodeCheckAccessConstant.REGION);
        Map<String, String> jsonResult = JsonUtils.toObject(result, Map.class);
        List<Map<String, Object>> textList = new ArrayList<>();
        if (jsonResult != null) {
            String allText = jsonResult.get("file_content");
            String[] texts = allText.split("\\\n");
            //存所有需要高亮行
            boolean[] sameLine = new boolean[texts.length + 1];
            for (Hit hit : fossfileItem.getHits()) {
                int start = hit.getHitStartLine();
                int end = hit.getHitEndLine() > texts.length ? texts.length : hit.getHitEndLine();
                while (start <= end) {
                    sameLine[start] = true;
                    start++;
                }
            }
            int line = 1;
            for (String text : texts) {
                Map<String, Object> textMap = new HashMap<>();
                textMap.put("issame", sameLine[line]);
                textMap.put("line", line);
                textMap.put("txt", text);
                line++;
                textList.add(textMap);
            }
            return textList;
        }
        return null;
    }

    /**
     * deleteFossFragment
     *
     * @param issueKey issueKey
     * @param uuid     uuid
     * @param date     date
     */
    public void deleteFossFragment(String issueKey, String uuid, String date) {
        Criteria criteria = Criteria.where("issueKey").is(issueKey);
        if (org.apache.commons.lang.StringUtils.isNotBlank(uuid)) {
            criteria.and("uuid").is(uuid);
        } else {
            criteria.and("date").is(date);
        }
        mongoTemplate.remove(Query.query(criteria), FossscanFragment.class);
    }

    /**
     * 已经确认，并且没有在当前任务列表里面的
     */
    public void queryByCondition() {
        Criteria criteria = Criteria.where("isConfirm").is(true);
        Query query = Query.query(criteria);
        mongoTemplate.remove(query, FossscanFragment.class, CodeCheckCollectionName.FOSSSCAN_FRAGMENT);
    }

    /**
     * 确认当前FossScan已经确认的数量
     *
     * @return long
     */
    public long querFossScanHasConfirmed() {
        Criteria criteria = Criteria.where("isConfirm").is(true);
        Query query = Query.query(criteria);
        long count = mongoTemplate.count(query, FossscanFragment.class, CodeCheckCollectionName.FOSSSCAN_FRAGMENT);
        return count;
    }

    public Map<String, Object> fossscanOptionList(FossscanModel fossscanModel) {
        Query query = new Query(Criteria.where("defectId").in(fossscanModel.getDefectIds()));
        List<FossscanFragment> defectList = mongoTemplate.find(query, FossscanFragment.class, CodeCheckCollectionName.FOSSSCAN_FRAGMENT);
        List<String> componentList = new ArrayList<>();
        for (FossscanFragment defect : defectList) {
            List<FossScan> fossList = defect.getScanResults();
            if (fossList != null) {
                for (FossScan foss : fossList) {
                    if (foss.getComponentName() != null && foss.getComponentVersion() != null) {
                        componentList.add(foss.getComponentName() + " " + foss.getComponentVersion());
                    }
                }
            }
        }
        Map<String, Object> result = new HashMap<>();
        result.put("componentList", componentList.stream().distinct().collect(Collectors.toList()));
        return result;
    }

    public void updateFoss(FossscanModel fossscanModel, String userId, String userName) {
        Criteria criteria = Criteria.where("defectId").in(fossscanModel.getDefectIds());
        if (!StringUtils.isEmpty(fossscanModel.getDate())) {
            criteria.and("date").is(fossscanModel.getDate());
        }
        Query query = new Query(criteria);
        FossscanFragment fossscanFragment = mongoTemplate.findOne(query, FossscanFragment.class, CodeCheckCollectionName.FOSSSCAN_FRAGMENT);
        Update update = new Update();
        update.set("fossType", fossscanModel.getFossType());
        update.set("componentName", fossscanModel.getComponentName());
        update.set("componentVersion", fossscanModel.getComponentVersion());
        update.set("isOpen", fossscanModel.getIsOpen());
        update.set("remarks", fossscanModel.getRemarks());
        update.set("result", fossscanModel.getResult());
        update.set("userId", userId);
        update.set("userName", userName);
        update.set("isConfirm", true);
        LocalDateTime time = LocalDateTime.now();
        update.set("confirmTime", time.format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss")));
        mongoTemplate.updateMulti(query, update, CodeCheckCollectionName.FOSSSCAN_FRAGMENT);
        StringBuilder ids = new StringBuilder(fossscanModel.getDefectIds().get(0) == null ? "" : fossscanModel.getDefectIds().get(0));
        for (int i = 1; i < fossscanModel.getDefectIds().size(); i++) {
            ids.append(",").append(fossscanModel.getDefectIds().get(i) == null ? "" : fossscanModel.getDefectIds().get(i));
        }

        if (fossscanFragment != null && !fossscanFragment.isConfirm()) {
            restCodeCheckUtils.updateDefectStatus(fossscanFragment.getTaskId(), ids.toString(), 2, CodeCheckConstants.REGION);
            if (fossscanModel.getDate() != null) {
                Query fossQuery = new Query(Criteria.where("taskId").is(fossscanFragment.getTaskId())
                        .and("date").is(fossscanFragment.getDate()));
                //修改summary
                CodeCheckResultSummaryVo codeCheckResultSummary = mongoTemplate.findOne(fossQuery, CodeCheckResultSummaryVo.class,
                        CodeCheckCollectionName.TASK_RESULT_SUMMARY);
                if (codeCheckResultSummary != null) {
                    int issueCount = codeCheckResultSummary.getIssueCount();
                    int ignoreCount = StringUtils.isEmpty(codeCheckResultSummary.getIgnoreCount()) ? 0 : Integer.parseInt(codeCheckResultSummary.getIgnoreCount());
                    Update fossUpdate = new Update();
                    fossUpdate.set("issueCount", issueCount - fossscanModel.getDefectIds().size());
                    fossUpdate.set("ignoreCount", ignoreCount + fossscanModel.getDefectIds().size());
                    mongoTemplate.updateMulti(fossQuery, fossUpdate, CodeCheckCollectionName.TASK_RESULT_SUMMARY);
                }
            }
        }
    }
}
