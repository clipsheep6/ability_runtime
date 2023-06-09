package com.huawei.impl;

import com.huawei.ci.common.contant.ResponseCode;
import com.huawei.ci.common.pojo.vo.Response;
import com.huawei.entity.pojo.DetailsAndTypeInfo;
import com.huawei.entity.pojo.DocDetailsInfo;
import com.huawei.entity.vo.ciinfo.codecheck.EventModel;
import com.huawei.entity.vo.codecheck.eventModule.CustomParameterVo;
import com.huawei.entity.vo.docs.v1.DocsCheckVo;
import com.huawei.entity.vo.docs.v1.FileErrorInfoVo;
import com.huawei.enums.DocsScanErrorType;
import com.huawei.operation.CustomParameterOperation;
import com.huawei.operation.DocsCheckOperation;
import com.huawei.operation.EventCodeCheckOperation;
import com.huawei.service.codecheck.DocsCheckBoardService;
import com.huawei.utils.DateUtil;
import com.huawei.utils.ExcelUtil;
import com.huawei.utils.TimeUtils;
import org.apache.commons.collections4.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.apache.poi.ss.usermodel.Workbook;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.util.*;
import java.util.stream.Collectors;

@Component
public class DocsCheckBoardServiceImpl implements DocsCheckBoardService {

    // 翻译仓
    private static final String FILE_ERROR_EN_TO_CN = "fileErrorEnCn";

    private static final String FILE_ERROR_DESCRIPTION = "fileErrorDescription";

    @Autowired
    private EventCodeCheckOperation eventCodeCheckOperation;

    @Autowired
    private DocsCheckOperation docsCheckOperation;

    @Autowired
    private TimeUtils timeUtil;

    @Autowired
    private CustomParameterOperation customParameterOperation;

    /**
     * 获取所有的增量资料扫描问题缺陷信息
     *
     * @param pageNum     当前页
     * @param pageSize    页大小
     * @param projectName 项目名
     * @param branch      分支
     * @param committer   提交人
     * @param startTime   开始时间
     * @param endTime     结束时间
     * @return Response
     */
    @Override
    public Response getAllDocsCheck(Integer pageNum, Integer pageSize, String projectName, String branch,
                                    String committer, String startTime, String endTime) {
        Map<String, Integer> pageMap = validPaging(pageNum, pageSize);
        Integer pageNumAfter = pageMap.get("pageNum");
        Integer pageSizeAfter = pageMap.get("pageSize");
        // 处理时间格式类型
        HashMap<String, String> mapForTime = dealWithInputDateTime(startTime, endTime);
        String startTimeAft = mapForTime.get("startTime");
        String endTimeAft = mapForTime.get("endTime");
        // 获取门禁资料信息
        List<DocsCheckVo> allDocsCheckData = new ArrayList<>();
        List<DocsCheckVo> docsCheckVoList = new ArrayList<>();
        Map<String, Object> allTotal = new HashMap<>();
        if (StringUtils.isNotBlank(committer)) {
            // 存在committer提交人信息
            List<EventModel> eventByCondition = eventCodeCheckOperation.getEventByCondition(projectName, branch, committer);
            LinkedList<String> currentTriggerEvent = new LinkedList<>();
            for (EventModel eventModel : eventByCondition) {
                if (!currentTriggerEvent.contains(eventModel.getUuid())) {
                    currentTriggerEvent.add(eventModel.getUuid());
                }
            }
            // 资料扫描仓
            List<DocsCheckVo> docsDataByTrigger = docsCheckOperation.getDocsDataByTrigger(currentTriggerEvent, startTimeAft, endTimeAft);
            allDocsCheckData = docsDataByTrigger.stream()
                    .skip((pageNumAfter - 1) * pageSizeAfter).limit(pageSizeAfter).collect(Collectors.toList());
            int size = docsDataByTrigger.size();
            allTotal.put("count", size);
        } else {
            allDocsCheckData = docsCheckOperation.getAllDocsCheckData(pageNumAfter, pageSizeAfter, branch, startTimeAft, endTimeAft);
            long countForDocs = docsCheckOperation.getCountForDocs(branch, startTimeAft, endTimeAft);
            allTotal.put("count", countForDocs);
        }
        docsCheckVoList = dealWithCurrentDocCheckData(allDocsCheckData);
        allTotal.put("docsList", docsCheckVoList);
        return Response.result(ResponseCode.SUCCESS, allTotal);
    }

    @Override
    public Response getDocsScanEventReport(String uuid) {
        LinkedHashMap<String, Integer> afterDealWith = new LinkedHashMap<>();
        DocsCheckVo summaryForCurrentEvent = docsCheckOperation.getSummaryForCurrentEvent(uuid);
        DocsCheckVo docsCheckVo = reBuildConstructForCurrentDoc(summaryForCurrentEvent);
        LinkedHashMap<String, Integer> countDocsProblems = groupCurrentDocs(docsCheckVo);
        Iterator<String> iterator = countDocsProblems.keySet().iterator();
        CustomParameterVo customParametersEn2CN = customParameterOperation.getCustomParameterByConfiguration(FILE_ERROR_EN_TO_CN);
        Map<String, Object> parametersEn2CN = customParametersEn2CN.getParameters();
        while (iterator.hasNext()) {
            String next = iterator.next();
            afterDealWith.put((String) parametersEn2CN.get(next), countDocsProblems.get(next));
        }
        return Response.result(ResponseCode.SUCCESS, afterDealWith);
    }

    @Override
    public Response getDocsScanEventDetails(DocDetailsInfo docDetailsInfo) {
        LinkedList<FileErrorInfoVo> fileErrorInfoVoLinkedList = new LinkedList<>();
        List<LinkedHashMap<String, String>> reportForCurrentEvent = getReportForCurrentEvent(docDetailsInfo.getUuid());
        // cn
        CustomParameterVo customParametersEn2CN = customParameterOperation.getCustomParameterByConfiguration(FILE_ERROR_EN_TO_CN);
        Map<String, Object> parametersEn2CN = customParametersEn2CN.getParameters();
        // en
        CustomParameterVo customParametersDescription = customParameterOperation.getCustomParameterByConfiguration(FILE_ERROR_DESCRIPTION);
        Map<String, Object> parametersDescription = customParametersDescription.getParameters();
        for (LinkedHashMap<String, String> details : reportForCurrentEvent) {
            FileErrorInfoVo fileErrorInfoVo = new FileErrorInfoVo();
            fileErrorInfoVo.setErrorType(details.get("error_type"));
            fileErrorInfoVo.setErrorInfo(details.get("error_info"));
            fileErrorInfoVo.setFile(details.get("file"));
            fileErrorInfoVo.setErrorTypeEN((String) parametersDescription.get(details.get("error_type")));
            fileErrorInfoVo.setErrorTypeCN((String) parametersEn2CN.get(details.get("error_type")));
            fileErrorInfoVoLinkedList.add(fileErrorInfoVo);
        }
        // filter
        Map<String, Object> filterByConditionMap = doFilterByCondition(fileErrorInfoVoLinkedList, docDetailsInfo);
        return Response.result(ResponseCode.SUCCESS, filterByConditionMap);
    }

    /**
     * report
     *
     * @return
     * @param: uuid uuid
     */
    private List<LinkedHashMap<String, String>> getReportForCurrentEvent(String uuid) {
        DocsCheckVo summaryForCurrentEvent = docsCheckOperation.getSummaryForCurrentEvent(uuid);
        DocsCheckVo docsCheckVo = reBuildConstructForCurrentDoc(summaryForCurrentEvent);
        List<LinkedHashMap<String, String>> scanResult = docsCheckVo.getScanResult();
        return scanResult;
    }

    /**
     * filter
     *
     * @param: fileErrorInfoVoLinkedList
     * @param: detailsAndTypeInfo
     */
    private Map<String, Object> doFilterByCondition(LinkedList<FileErrorInfoVo> fileErrorInfoVoLinkedList, DocDetailsInfo docDetailsInfo) {
        Map<String, Object> detailsMap = new HashMap<>();
        List<FileErrorInfoVo> errorInfoVos = new ArrayList<>();
        int errorCount;
        Map<String, Integer> pageMap = validPaging(docDetailsInfo.getPageNum(), docDetailsInfo.getPageSize());
        Integer pageNumAfter = pageMap.get("pageNum");
        Integer pageSizeAfter = pageMap.get("pageSize");
        if (docDetailsInfo.getErrorType() == 0) {
            errorInfoVos = fileErrorInfoVoLinkedList.stream().filter(currentErrorVo ->
                    StringUtils.isNotBlank(currentErrorVo.getErrorType())).collect(Collectors.toList());
        } else {
            String errorTypeByCode = DocsScanErrorType.getErrorTypeByCode(docDetailsInfo.getErrorType());
            errorInfoVos = fileErrorInfoVoLinkedList.stream().filter(currentErrorVo ->
                    currentErrorVo.getErrorType().equalsIgnoreCase(errorTypeByCode)).collect(Collectors.toList());
        }
        errorCount = errorInfoVos.size();
        List<FileErrorInfoVo> collect = errorInfoVos.stream().skip((pageNumAfter - 1) * pageSizeAfter)
                .limit(pageSizeAfter).collect(Collectors.toList());
        detailsMap.put("detailList", collect);
        detailsMap.put("count", errorCount);
        return detailsMap;
    }

    private Map<String, Integer> validPaging(Integer pageNum, Integer pageSize) {
        Map<String, Integer> pageMap = new HashMap<>();
        if (Objects.isNull(pageNum) || Objects.isNull(pageSize)) {
            pageNum = 1;
            pageSize = 10;
        }
        pageMap.put("pageNum", pageNum);
        pageMap.put("pageSize", pageSize);
        return pageMap;
    }

    public HashMap<String, String> dealWithInputDateTime(String inputStartTime, String inputEndTime) {
        HashMap<String, String> timeMap = new HashMap<>();
        String startTime = "";
        String endTime = "";
        if (StringUtils.isNotBlank(inputStartTime)) {
            startTime = dealWithLocalDateTime(inputStartTime);
        }
        if (StringUtils.isNotBlank(inputEndTime)) {
            endTime = dealWithLocalDateTime(inputEndTime);
        }
        timeMap.put("startTime", startTime);
        timeMap.put("endTime", endTime);
        return timeMap;
    }

    private String dealWithLocalDateTime(String inputStartTime) {
        LocalDateTime localDateTime = null;
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        try {
            Date parse = sdf.parse(inputStartTime);
            Instant instant = parse.toInstant();
            ZoneId zoneId = ZoneId.systemDefault();
            localDateTime = instant.atZone(zoneId).toLocalDateTime();
        } catch (ParseException e) {
            e.printStackTrace();
        }
        return localDateTime.toString();
    }

    private List<DocsCheckVo> dealWithCurrentDocCheckData(List<DocsCheckVo> allDocsCheckData) {
        List<DocsCheckVo> rebuildList = new LinkedList<>();
        if (CollectionUtils.isNotEmpty(allDocsCheckData)) {
            allDocsCheckData.forEach(currentDoc -> {
                rebuildList.add(reBuildConstructForCurrentDoc(currentDoc));
            });
        }
        return rebuildList;
    }

    private DocsCheckVo reBuildConstructForCurrentDoc(DocsCheckVo currentDoc) {
        if (Objects.nonNull(currentDoc)) {
            currentDoc.setCreateTime(DateUtil.dateZoneToStandDates(currentDoc.getCreateTime()));
            if (StringUtils.isNotBlank(currentDoc.getUuid())) {
                EventModel eventByUuid = eventCodeCheckOperation.getEventByUuid(currentDoc.getUuid());
                if (Objects.nonNull(eventByUuid)) {
                    currentDoc.setTriggerName(StringUtils.isNoneBlank(eventByUuid.getTriggerUser()) ? eventByUuid.getTriggerUser() : "");
                    if (StringUtils.isNotBlank(eventByUuid.getTimestamp())) {
                        currentDoc.setTriggerTime(timeUtil.linkedDateToString(eventByUuid.getTimestamp()));
                    }
                }
            }
            LinkedHashMap<String, Object> message = currentDoc.getMessage();
            message.forEach((k, v) -> {
                if (Objects.nonNull(v)) {
                    if (v instanceof List<?> && v.equals("mdFiles")) {
                        List<String> mdFileList = (List<String>) message.get(v);
                        currentDoc.setMdFiles(mdFileList);
                    } else if (v instanceof Integer && v.equals("scan_case_num")) {
                        currentDoc.setScanCaseNum((Integer) v);
                    } else if (v instanceof Integer && v.equals("pass_case_num")) {
                        currentDoc.setPassCaseNum((Integer) v);
                    } else if (v instanceof Integer && v.equals("fail_case_num")) {
                        currentDoc.setFailCaseNum((Integer) v);
                    } else if (v instanceof LinkedHashMap && v.equals("fault_summary")) {
                        currentDoc.setFaultSummary((LinkedHashMap<?, ?>) v);
                    } else if (v instanceof List<?> && v.equals("scanResult")) {
                        currentDoc.setScanResult((List<LinkedHashMap<String, String>>) v);
                    }
                }
            });
            currentDoc.setMessage(new LinkedHashMap<>());
        }
        return currentDoc;
    }

    /**
     * filter and export
     *
     * @param detailsAndTypeInfo detailsAndTypeInfo
     * @return byte[]
     */
    public byte[] exportDocsCheckSummary(DetailsAndTypeInfo detailsAndTypeInfo) {
        HashMap<String, String> mapForTime = dealWithInputDateTime(detailsAndTypeInfo.getStartTime(), detailsAndTypeInfo.getEndTime());
        String startTimeAft = mapForTime.get("startTime");
        String endTimeAft = mapForTime.get("endTime");
        List<DocsCheckVo> allDocsCheckData;
        if (StringUtils.isNotBlank(detailsAndTypeInfo.getCommitter())) {
            List<EventModel> eventByCondition = eventCodeCheckOperation.getEventByCondition(detailsAndTypeInfo.getProjectName(),
                    detailsAndTypeInfo.getBranch(), detailsAndTypeInfo.getCommitter());
            LinkedList<String> currentTriggerEvent = new LinkedList<>();
            for (EventModel eventModel : eventByCondition) {
                if (!currentTriggerEvent.contains(eventModel.getUuid())) {
                    currentTriggerEvent.add(eventModel.getUuid());
                }
            }
            allDocsCheckData = docsCheckOperation.getDocsDataByTrigger(currentTriggerEvent, startTimeAft, endTimeAft);
        } else {
            allDocsCheckData = docsCheckOperation.getAllDocsCheckDataExport(detailsAndTypeInfo.getBranch(), startTimeAft, endTimeAft);
        }
        LinkedList<FileErrorInfoVo> fileErrorInfoVoLinkedList = dealWithDocsDataForProblematical(allDocsCheckData,
                detailsAndTypeInfo.getCommitter(), detailsAndTypeInfo.getProjectName());
        Workbook workbook = ExcelUtil.export(fileErrorInfoVoLinkedList, FileErrorInfoVo.class);
        return ExcelUtil.workbookToBytes(workbook, "docsCheck-summary");
    }

    /**
     * 处理有问题的数据
     *
     * @param allDocsCheckData 有问题的数据
     */
    private LinkedList<FileErrorInfoVo> dealWithDocsDataForProblematical(List<DocsCheckVo> allDocsCheckData, String committer, String projectName) {
        LinkedList<FileErrorInfoVo> fileErrorInfoVoLinkedList = new LinkedList<>();
        if (Objects.nonNull(allDocsCheckData)) {
            CustomParameterVo customParametersEn2CN = customParameterOperation.getCustomParameterByConfiguration(FILE_ERROR_EN_TO_CN);
            Map<String, Object> parametersEn2CN = customParametersEn2CN.getParameters();
            CustomParameterVo customParametersDescription = customParameterOperation.getCustomParameterByConfiguration(FILE_ERROR_DESCRIPTION);
            Map<String, Object> parametersDescription = customParametersDescription.getParameters();
            for (DocsCheckVo allDocsCheckDatum : allDocsCheckData) {
                List<LinkedHashMap<String, String>> scanResult;
                scanResult = (List<LinkedHashMap<String, String>>) allDocsCheckDatum.getMessage().get("scanResult");
                String PRInfo = allDocsCheckDatum.getPRList().get(0);
                String createTime = allDocsCheckDatum.getCreateTime().replace("T", " ");
                String uuid = allDocsCheckDatum.getUuid();
                String manifestBranch = allDocsCheckDatum.getManifestBranch();
                // defect details
                if (CollectionUtils.isNotEmpty(scanResult)) {
                    for (LinkedHashMap<String, String> details : scanResult) {
                        FileErrorInfoVo fileErrorInfoVo = new FileErrorInfoVo();
                        fileErrorInfoVo.setCommitter(StringUtils.isNoneBlank(committer) ? committer : "--");
                        fileErrorInfoVo.setUuid(uuid);
                        fileErrorInfoVo.setPrInfo(PRInfo);
                        fileErrorInfoVo.setCreateTime(createTime);
                        fileErrorInfoVo.setProjectName(projectName);
                        fileErrorInfoVo.setBranch(manifestBranch);
                        fileErrorInfoVo.setErrorType(details.get("error_type"));
                        fileErrorInfoVo.setErrorInfo(details.get("error_info"));
                        fileErrorInfoVo.setFile(details.get("file"));
                        fileErrorInfoVo.setErrorTypeEN((String) parametersDescription.get(details.get("error_type")));
                        fileErrorInfoVo.setErrorTypeCN((String) parametersEn2CN.get(details.get("error_type")));
                        fileErrorInfoVoLinkedList.add(fileErrorInfoVo);
                    }
                }
            }

        }
        return fileErrorInfoVoLinkedList;
    }

    private LinkedHashMap<String, Integer> groupCurrentDocs(DocsCheckVo docsCheckVo) {
        LinkedHashMap<String, Integer> reportMap = new LinkedHashMap<>();
        if (Objects.nonNull(docsCheckVo)) {
            LinkedHashMap<?, ?> faultSummary = docsCheckVo.getFaultSummary();
            reportMap = (LinkedHashMap<String, Integer>) faultSummary;
        }
        return reportMap;
    }

    /**
     * filter and export
     *
     * @param detailsAndTypeInfo detailsAndTypeInfo
     * @return byte
     */
    public byte[] exportDocsCheckDetails(DetailsAndTypeInfo detailsAndTypeInfo) {
        LinkedList<FileErrorInfoVo> fileErrorInfoVoLinkedList = new LinkedList<>();
        EventModel eventByUuid = eventCodeCheckOperation.getEventByUuid(detailsAndTypeInfo.getUuid());
        DocsCheckVo summaryForCurrentEvent = docsCheckOperation.getSummaryForCurrentEvent(detailsAndTypeInfo.getUuid());
        CustomParameterVo customParametersEn2CN = customParameterOperation.getCustomParameterByConfiguration(FILE_ERROR_EN_TO_CN);
        Map<String, Object> parametersEn2CN = customParametersEn2CN.getParameters();
        CustomParameterVo customParametersDescription = customParameterOperation.getCustomParameterByConfiguration(FILE_ERROR_DESCRIPTION);
        Map<String, Object> parametersDescription = customParametersDescription.getParameters();
        List<LinkedHashMap<String, String>> scanResult = (List<LinkedHashMap<String, String>>) summaryForCurrentEvent.getMessage()
                .get("scanResult");
        String createTime = summaryForCurrentEvent.getCreateTime().replace("T", " ");
        String uuid = summaryForCurrentEvent.getUuid();
        String manifestBranch = summaryForCurrentEvent.getManifestBranch();
        String PRInfo = summaryForCurrentEvent.getPRList().get(0);
        String triggerUser = "--";
        String projectName = "--";
        if (Objects.nonNull(eventByUuid)) {
            triggerUser = eventByUuid.getTriggerUser();
            projectName = eventByUuid.getProjectName();
        }
        // defect
        if (CollectionUtils.isNotEmpty(scanResult)) {
            for (LinkedHashMap<String, String> details : scanResult) {
                FileErrorInfoVo fileErrorInfoVo = new FileErrorInfoVo();
                fileErrorInfoVo.setUuid(uuid);
                fileErrorInfoVo.setCommitter(triggerUser);
                fileErrorInfoVo.setCreateTime(createTime);
                fileErrorInfoVo.setProjectName("--");
                if (Objects.nonNull(eventByUuid)) {
                    fileErrorInfoVo.setProjectName(projectName);
                }
                fileErrorInfoVo.setPrInfo(PRInfo);
                fileErrorInfoVo.setBranch(manifestBranch);
                fileErrorInfoVo.setErrorType(details.get("error_type"));
                fileErrorInfoVo.setErrorInfo(details.get("error_info"));
                fileErrorInfoVo.setFile(details.get("file"));
                fileErrorInfoVo.setErrorTypeEN((String) parametersDescription.get(details.get("error_type")));
                fileErrorInfoVo.setErrorTypeCN((String) parametersEn2CN.get(details.get("error_type")));
                fileErrorInfoVoLinkedList.add(fileErrorInfoVo);
            }
        }
        Workbook workbook = ExcelUtil.export(fileErrorInfoVoLinkedList, FileErrorInfoVo.class);
        return ExcelUtil.workbookToBytes(workbook, "docsCheck-details");
    }
}
