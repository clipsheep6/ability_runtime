/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 */

package com.huawei.impl;

import com.huawei.ci.common.contant.ResponseCode;
import com.huawei.ci.common.pojo.vo.Response;

import com.huawei.ci.common.utils.JsonUtils;
import com.huawei.entity.dto.GraphNode;
import com.huawei.entity.pojo.ChildrenForCurrentTask;
import com.huawei.entity.pojo.CodeCheckResultSummaryVo;
import com.huawei.entity.vo.cicd.CiBranch;
import com.huawei.entity.vo.cicd.CiBuild;
import com.huawei.entity.vo.cicd.CiTitle;
import com.huawei.entity.vo.ciinfo.event.exportData.EventBuildTarget;
import com.huawei.entity.vo.ciinfo.event.exportData.EventExportData;
import com.huawei.entity.vo.codecheck.codecheckdetails.CodeCheckResultDetailsVo;
import com.huawei.entity.vo.ciinfo.codecheck.CodeCheckInfo;
import com.huawei.entity.vo.ciinfo.codecheck.EventModel;
import com.huawei.entity.vo.ciinfo.codecheck.PrMsg;
import com.huawei.entity.vo.ciinfo.event.CodeCheckDevCloud;
import com.huawei.entity.vo.ciinfo.event.CodeCheckDevVo;
import com.huawei.entity.vo.ciinfo.event.EventVo;
import com.huawei.entity.vo.ciinfo.event.QueryIncDetailModel;
import com.huawei.entity.vo.ciinfo.event.SubCodeCheckItemsForPrMsg;
import com.huawei.entity.vo.codecheck.eventModule.CustomParameterVo;
import com.huawei.entity.vo.codecheck.fossscan.FossscanFragment;
import com.huawei.entity.vo.codecheck.fossscan.FossscanModel;
import com.huawei.enums.GraphNodeType;
import com.huawei.mapper.EventCodeCheckMapper;
import com.huawei.mapper.SystemUnstableMapper;
import com.huawei.operation.CodeCheckDevOperation;
import com.huawei.operation.CustomParameterOperation;
import com.huawei.operation.EventCodeCheckOperation;
import com.huawei.operation.FossScanOperation;
import com.huawei.operation.IncResultDetailsOperation;
import com.huawei.operation.IncResultSummaryOperation;
import com.huawei.operation.ResultSummaryOperation;
import com.huawei.service.EventService;
import com.huawei.utils.ExcelUtil;
import com.huawei.utils.TimeUtils;
import com.huawei.utils.ValidatorUtil;
import org.apache.commons.collections4.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.apache.poi.ss.usermodel.Workbook;
import org.apache.poi.ss.util.CellRangeAddress;
import org.apache.poi.xssf.usermodel.XSSFCell;
import org.apache.poi.xssf.usermodel.XSSFCellStyle;
import org.apache.poi.xssf.usermodel.XSSFRow;
import org.apache.poi.xssf.usermodel.XSSFSheet;
import org.apache.poi.xssf.usermodel.XSSFWorkbook;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

import static com.huawei.ci.common.enums.CodeCheckConstants.ACCESS_TOKEN;
import static com.huawei.enums.CodeCheckAccessConstant.PASS;
import static com.huawei.enums.CodeCheckAccessConstant.USERNAME;
import static com.huawei.enums.CodeCheckConstants.ARK;
import static com.huawei.enums.CodeCheckConstants.ARK_PROJECT;
import static com.huawei.enums.CodeCheckConstants.DOWNLOAD_URL;
import static com.huawei.enums.CodeCheckConstants.FAILED;
import static com.huawei.enums.CodeCheckConstants.NO_PASS;
import static com.huawei.enums.CodeCheckConstants.OHOS_REPO_CONFIG_KEY;
import static com.huawei.enums.CodeCheckConstants.OHPG_GIT_URL;
import static com.huawei.enums.CodeCheckConstants.RUNNING;
import static com.huawei.enums.CodeCheckConstants.SUCCESS;
import static com.huawei.enums.SymbolConstants.WAVE;

/**
 * @Description: EventService
 * @ClassName: com.huawei.service
 * @since : 2022/11/3  10:38
 **/
@Component
public class EventServiceImpl implements EventService {
    private static final Logger LOGGER = LoggerFactory.getLogger(EventServiceImpl.class);

    @Autowired
    private CodeCheckDevOperation codeCheckDevOperation;

    @Autowired
    private EventCodeCheckOperation eventCodeCheckOperation;

    @Autowired
    private EventCodeCheckMapper eventCodeCheckMapper;

    @Autowired
    private SystemUnstableMapper systemUnstableMapper;

    @Autowired
    private CustomParameterOperation customParameterOperation;

    @Autowired
    private IncResultSummaryOperation incResultSummaryOperation;

    @Autowired
    private ScheduleDelegateImpl scheduleDelegate;

    @Autowired
    private TimeUtils timeUtils;

    @Autowired
    private ValidatorUtil validatorUtil;

    @Autowired
    private IncResultDetailsOperation incResultDetailsOperation;

    @Autowired
    private ResultSummaryOperation resultSummaryOperation;

    @Autowired
    private FossScanOperation fossScanOperation;

    @Autowired
    private ProjectImpl projectDelegate;

    /**
     * 代码门禁运行状况列表
     *
     * @param pageNum       pageNum
     * @param pageSize      pageSize
     * @param projectName   projectName
     * @param branch        branch
     * @param startTime     startTime
     * @param endTime       endTime
     * @param triggerUser   triggerUser
     * @param buildFailType buildFailType
     * @return Response
     */
    @Override
    public Response getEvents(Integer pageNum, Integer pageSize, String projectName, String branch, String startTime,
                              String endTime, String triggerUser, String buildFailType) {
        List<EventModel> events = eventCodeCheckOperation.getByCondition(pageNum, pageSize, projectName, branch,
                startTime, endTime, triggerUser, buildFailType);
        long total = eventCodeCheckOperation.count(projectName, branch, startTime, endTime, triggerUser,
                buildFailType);
        long dateNow = new Date().getTime();
        CustomParameterVo customParameterByConfiguration =
                customParameterOperation.getCustomParameterByConfiguration("innerProjectRepos");
        List<String> reposForInner = (List<String>) customParameterByConfiguration.getParameters().get("gitUrls");
        for (EventModel event : events) {
            if (Objects.nonNull(event)) {
                // 获取当前任务的门禁信息
                EventModel eventModel = eventCodeCheckOperation.getEventById(event.getId());
                // 蓝区
                List<CodeCheckResultSummaryVo> byUuid =
                        incResultSummaryOperation.getSummaryByUuid(eventModel.getUuid());
                // 黄区
                CodeCheckDevCloud devCloudByUuid = codeCheckDevOperation.getDevCloudByUuid(event.getUuid());
                // 静态检查
                CodeCheckInfo codeCheckInfo =JsonUtils.fromJson(JsonUtils.toJson(event.getCodeCheckInfo()), CodeCheckInfo.class);
                String innerCodeCheckResult = null;
                String innerCodeCheckStatus;
                String codeCheckStatus;
                String resultSetForYCheck;
                String filterForCommunityCheckResult;
                // 黄蓝区过滤后 结果
                if (Objects.nonNull(devCloudByUuid)) {
                    resultSetForYCheck =
                            scheduleDelegate.doFilterForInnerReposTotalResult(devCloudByUuid.getSubCodeCheckList(),
                                    reposForInner, codeCheckInfo, byUuid, devCloudByUuid);
                } else {
                    resultSetForYCheck = PASS;
                }
                filterForCommunityCheckResult = scheduleDelegate.doFilterForCommunityCheckResult(byUuid);
                long dateCreate = 0L;
                long dateCreates;
                // 触发了静态检查的情况,并且黄区的结果不为空的情况下
                if (Objects.nonNull(codeCheckInfo) && Objects.nonNull(devCloudByUuid)) {
                    innerCodeCheckResult = devCloudByUuid.getTotalResult();
                    innerCodeCheckStatus = checkEffectiveInnerCodeCheckStatus(innerCodeCheckResult, devCloudByUuid);
                    dateCreates = checkTimeExist(devCloudByUuid, dateCreate, eventModel);
                    codeCheckStatus = codeCheckInfo.getCodeCheckStatus();
                    // 根据任务执行状态来设定对应的任务执行过程
                    checkCurrentStatusForEvents(devCloudByUuid, dateNow, innerCodeCheckStatus, dateCreates,
                            resultSetForYCheck,
                            filterForCommunityCheckResult, event, codeCheckStatus);
                } else if (Objects.nonNull(codeCheckInfo)) {
                    // 特定项目只要蓝区的结果
                    if (CollectionUtils.isNotEmpty(byUuid) || eventModel.getProjectName().equals(ARK)) {
                        codeCheckStatus = codeCheckInfo.getCodeCheckStatus();
                        // 获取每一个对应的ark pr的蓝区结果,排除非ARK pr结果
                        Set<String> incResult = new HashSet<>();
                        byUuid.forEach(incEvent -> {
                            if (incEvent.getMrUrl().startsWith(ARK_PROJECT)) {
                                incResult.add(incEvent.getResult());
                            }
                        });
                        // 特定项目的蓝区结果
                        String codecheckResultForSpecial = checkFinalResultForSpecialProject(incResult);
                        String finalStatus = checkSpecialProjectStatus(codeCheckStatus, codecheckResultForSpecial);
                        event.setCodeCheckStatusWithYCodeCheck(finalStatus);
                    }
                    // 未触发静态检查
                } else {
                    event.setCodeCheckStatusWithYCodeCheck("noCheck");
                }
            }
        }
        Map<String, Object> resultMap = new HashMap<>();
        resultMap.put("events", events);
        resultMap.put("total", total);
        // 旧backend读全局配置参数
        resultMap.put("downloadUrl", "http://download.ci.openharmony.cn/");
        return Response.result(ResponseCode.SUCCESS, resultMap);
    }

    /**
     * 代码门禁详情
     *
     * @param eventId eventId
     * @return Response
     */
    @Override
    public Response getEvent(String eventId) {
        long dateNow = new Date().getTime();
        validatorUtil.validId(eventId);
        // 内部仓信息
        CustomParameterVo customParameterByConfiguration =
                customParameterOperation.getCustomParameterByConfiguration(OHOS_REPO_CONFIG_KEY);
        List<String> reposForInner = (List<String>) customParameterByConfiguration.getParameters().get("gitUrls");
        // 获取当前门禁信息
        EventModel eventModel = eventCodeCheckOperation.getEventById(eventId);
        // 获取到当前所有PR蓝区对应的结果信息
        List<CodeCheckResultSummaryVo> byUuid = new ArrayList<>();
        if (Objects.nonNull(eventModel)) {
            byUuid = incResultSummaryOperation.getSummaryByUuid(eventModel.getUuid());
        }
        // 未触发静态检查codecheckInfo  暂时为空
        String codeCheckStatus = "--";
        // 社区检查结果
        if (Objects.nonNull(eventModel)) {
            CodeCheckInfo codeCheckInfo;
            long dateCreate = 0;
            long dateCreates = 0;
            String innerCodeCheckResult = "";
            String innerCodeCheckStatus = "";
            String resultSetForYCheck = null;
            String filterForCommunityCheckResult = null;
            HashSet<String> resultSetForBCheck = new HashSet<>();
            CodeCheckDevCloud devCloudByUuid = codeCheckDevOperation.getDevCloudByUuid(eventModel.getUuid());
            // 蓝区结果集
            resultSetForBCheck = getAllBCodeCheckSet(eventModel.getUuid());
            codeCheckInfo =JsonUtils.fromJson(JsonUtils.toJson(eventModel.getCodeCheckInfo()), CodeCheckInfo.class);
            if (Objects.nonNull(codeCheckInfo)) {
                codeCheckStatus = codeCheckInfo.getCodeCheckStatus();
            }
            // 设置当前任务门禁结果
            dealWithInnerCodeCheckDataForPr(eventModel, dateNow, devCloudByUuid, reposForInner, resultSetForBCheck,
                    codeCheckInfo);
            if (Objects.nonNull(devCloudByUuid)) {
                innerCodeCheckResult = devCloudByUuid.getTotalResult();
                innerCodeCheckStatus = checkEffectiveInnerCodeCheckStatus(innerCodeCheckResult, devCloudByUuid);
                dateCreates = checkTimeExist(devCloudByUuid, dateCreate, eventModel);
                // 有效的黄区结果
                resultSetForYCheck =
                        scheduleDelegate.doFilterForInnerReposTotalResult(devCloudByUuid.getSubCodeCheckList(),
                        reposForInner, codeCheckInfo, byUuid, devCloudByUuid);
                filterForCommunityCheckResult = scheduleDelegate.doFilterForCommunityCheckResult(byUuid);
                eventModel.setCodeCheckDuration(devCloudByUuid.getBuildDuration());
            } else {
                // 如果当前任务并没有触发黄区codecheck检查
                eventModel.setFinalTotalResult(RUNNING);
                String timestampBaseDOnOutSide = eventModel.getTimestamp();
                long time = timeUtils.linkedStrToDate(timestampBaseDOnOutSide).getTime();
                if ((dateNow - time) > (TimeUtils.HOUR) / 2) {
                    eventModel.setFinalTotalResult(FAILED);
                }
            }

            // 总结果
            if (Objects.nonNull(devCloudByUuid)) {
                doJudgementForAllCheckResult(codeCheckInfo, codeCheckStatus, eventModel, filterForCommunityCheckResult
                        , dateNow, dateCreates, resultSetForYCheck, innerCodeCheckStatus, devCloudByUuid);
            } else {
                eventModel.setFinalTotalResult(codeCheckStatus);
            }
            // 设置对应的所有的PR的详情信息
            eventModel.setCodeCheckSummary(byUuid);
            eventModel.setDownloadUrl(DOWNLOAD_URL);
        }
        return Response.result(ResponseCode.SUCCESS,eventModel);
    }

    /**
     * 代码门禁详情-影响点（图标）
     *
     * @param eventId eventId
     * @return Response
     */
    @Override
    public Response getGraph(String eventId) {
        EventVo eventVo = eventCodeCheckOperation.getEventVoById(eventId);
        if (Objects.isNull(eventVo) || Objects.isNull(eventVo.getEffectTree())) {
            return Response.result(ResponseCode.SUCCESS, "data is not exist", FAILED);
        }
        HashMap<String, Object> result = new HashMap<>();
        result.put("productGraph", getProductGraph(eventVo));

        result.put("testGraph", getTestGraph(eventVo));
        return Response.result(ResponseCode.SUCCESS,result);
    }

    /**
     * 代码门禁详情-获取问题代码
     *
     * @param uuid                uuid
     * @param taskId              taskId
     * @param queryIncDetailModel queryIncDetailModel
     * @return Response
     */
    @Override
    public Response getCodeCheck(String uuid, String taskId, QueryIncDetailModel queryIncDetailModel) {
        validatorUtil.validUuid(uuid);
        CodeCheckResultDetailsVo resultListByUuid = incResultDetailsOperation.getResultListByUuid(uuid, taskId,
                queryIncDetailModel);
        return Response.result(ResponseCode.SUCCESS, resultListByUuid);
    }

    /**
     * 代码门禁问题片段扫描（获取FossCan问题屏蔽列表）
     *
     * @param taskId    当前任务id
     * @param uuid      当前任务uuid
     * @param date      时间
     * @param isConfirm 是否确认
     * @param pageSize  每页显示条数
     * @param pageNum   当前页码
     * @return Response
     */
    @Override
    public Response getFossScanFragment(String taskId, String uuid, String date, Boolean isConfirm, Integer pageSize,
        Integer pageNum) {
        List<FossscanFragment> fossscanFragmentsForAll = new ArrayList<>();
        long confirmedCountForChildren = 0;
        long noConfirmedCountForChildren = 0;
        // 存在子任务信息
        CodeCheckResultSummaryVo currentTaskInfo = resultSummaryOperation.getSummaryByTaskIdAndDate(date, taskId);
        if (Objects.nonNull(currentTaskInfo) && CollectionUtils.isNotEmpty(currentTaskInfo.getTaskOfInclude())) {
            for (ChildrenForCurrentTask childrenForCurrentTask : currentTaskInfo.getTaskOfInclude()) {
                // fossscan问题以及数量统计
                List<FossscanFragment> fossscanFragmentsforChild =
                        fossScanOperation.queryFossscan(childrenForCurrentTask.getTaskId(),
                                uuid, date, isConfirm, pageSize, pageNum);
                long confirmedCountForCurrentChild = fossScanOperation.count(childrenForCurrentTask.getTaskId(), uuid
                        , date, true);
                long noConfirmedCountForCurrentChild = fossScanOperation.count(childrenForCurrentTask.getTaskId(),
                        uuid, date, false);
                // 子任务聚合
                fossscanFragmentsforChild.stream().forEach(currentFragment -> fossscanFragmentsForAll.add(currentFragment));
                confirmedCountForChildren += confirmedCountForCurrentChild;
                noConfirmedCountForChildren += noConfirmedCountForCurrentChild;
            }
        }
        //任务字段扫描
        List<FossscanFragment> fossscanFragmentList = fossScanOperation.queryFossscan(taskId, uuid, date, isConfirm,
                pageSize, pageNum);
        long confirmedCount = fossScanOperation.count(taskId, uuid, date, true);
        long noConfirmedCount = fossScanOperation.count(taskId, uuid, date, false);
        if (CollectionUtils.isNotEmpty(fossscanFragmentList)) {
            fossscanFragmentList.stream().forEach(currentFragment -> fossscanFragmentsForAll.add(currentFragment));
        }
        HashMap<String, Object> result = new HashMap<>();
        result.put("fossList", fossscanFragmentsForAll);
        result.put("confirmedCount", confirmedCount + confirmedCountForChildren);
        result.put("noConfirmedCount", noConfirmedCount + noConfirmedCountForChildren);
        return Response.result(ResponseCode.SUCCESS, result);
    }

    @Override
    public Response queryOpensource(String defectId, String hash) {
        return Response.result(ResponseCode.SUCCESS,fossScanOperation.queryOpensource(defectId, hash));
    }

    @Override
    public Response fossScanOptionList(FossscanModel fossscanModel) {
        return Response.result(ResponseCode.SUCCESS,fossScanOperation.fossscanOptionList(fossscanModel));
    }

    @Override
    public Response updateFossScanFragment(String userId, String userName, FossscanModel fossscanModel) {
        fossScanOperation.updateFoss(fossscanModel, userId, userName);
        return Response.result(ResponseCode.SUCCESS,SUCCESS);
    }

    /**
     * 有效的黄区内部的执行状态
     */
    private String checkEffectiveInnerCodeCheckStatus(String innerCodeCheckResult, CodeCheckDevCloud codeCheckDevCloud) {
        String innerCodeCheckStatus;
        if (Objects.nonNull(innerCodeCheckResult)) {
            innerCodeCheckStatus = "end";
        } else {
            innerCodeCheckStatus = codeCheckDevCloud.getCurrentStatus();
        }
        return innerCodeCheckStatus;
    }

    /**
     * 保证时间有效
     */
    public Long checkTimeExist(CodeCheckDevCloud devCloudByUuid, long dateCreate, EventModel eventModel) {
        if (Objects.nonNull(devCloudByUuid.getRunning())) {
            if (Objects.nonNull(timeUtils.linkedStrToDate(devCloudByUuid.getRunning().getTimestamp()))) {
                dateCreate = timeUtils.linkedStrToDate(devCloudByUuid.getRunning().getTimestamp()).getTime();
            }
        } else {
            dateCreate = timeUtils.linkedStrToDate(eventModel.getTimestamp()).getTime();
        }
        return dateCreate;
    }

    /**
     * 校验当前增量任务的检查状态
     */
    private void checkCurrentStatusForEvents(CodeCheckDevCloud devCloudByUuid, long dateNow, String innerCodeCheckStatus, long dateCreates,
                                             String resultSetForYCheck, String filterForCommunityCheckResult,
                                             EventModel eventModel, String codeCheckStatus) {
        // 优先判断蓝区
        if (Objects.nonNull(eventModel)) {
            if (codeCheckStatus.equals(RUNNING)) {
                eventModel.setCodeCheckStatusWithYCodeCheck(RUNNING);
                if ((dateNow - dateCreates) > (TimeUtils.HOUR) / 2) {
                    eventModel.setCodeCheckStatusWithYCodeCheck(FAILED);
                }
            } else if (filterForCommunityCheckResult.equals(FAILED)) {
                eventModel.setCodeCheckStatusWithYCodeCheck(FAILED);
            } else if (codeCheckStatus.equals(FAILED)) {
                eventModel.setCodeCheckStatusWithYCodeCheck(FAILED);
            } else if (filterForCommunityCheckResult.equals(PASS)) {
                // 再判断黄区的结果
                if (resultSetForYCheck.equals(FAILED)) {
                    eventModel.setCodeCheckStatusWithYCodeCheck(FAILED);
                } else if (innerCodeCheckStatus.equals(RUNNING)) {
                    // 黄区数据出问题的情况下(状态为running ，但是总结果已经出来了)
                    if (Objects.isNull(devCloudByUuid.getTotalResult())) {
                        eventModel.setCodeCheckStatusWithYCodeCheck(RUNNING);
                        if ((dateNow - dateCreates) > (TimeUtils.HOUR) / 2) {
                            eventModel.setCodeCheckStatusWithYCodeCheck(FAILED);
                        }
                    } else {
                        eventModel.setCodeCheckStatusWithYCodeCheck(resultSetForYCheck.equals(PASS) ? SUCCESS : FAILED);
                    }
                } else if (resultSetForYCheck.equals(PASS)) {
                    eventModel.setCodeCheckStatusWithYCodeCheck(SUCCESS);
                } else {
                    eventModel.setCodeCheckStatusWithYCodeCheck(FAILED);
                }
            }
        }
    }

    /**
     * @param incResult 蓝区结果
     */
    private String checkFinalResultForSpecialProject(Set<String> incResult) {
        String result = null;
        if (CollectionUtils.isNotEmpty(incResult)) {
            if (incResult.contains(FAILED)) {
                return FAILED;
            } else if (incResult.contains(NO_PASS)) {
                // 也暂定失败
                return FAILED;
            } else if (incResult.size() == 1 && incResult.contains(PASS)) {
                return PASS;
            }
        }
        return result;
    }

    /**
     * 判断当前任务的执行状态
     */
    private String checkSpecialProjectStatus(String codeCheckStatus, String codecheckResultForSpecial) {
        if (Objects.nonNull(codeCheckStatus)) {
            if (codeCheckStatus.equals(RUNNING)) {
                return RUNNING;
            } else {
                return codecheckResultForSpecial;
            }
        }
        return null;
    }

    /**
     * 获取蓝区的结果集，用于判断状态
     *
     * @param uuid uuid
     */
    private HashSet<String> getAllBCodeCheckSet(String uuid) {
        List<CodeCheckResultSummaryVo> allSummaryVo = incResultSummaryOperation.getSummaryByUuid(uuid);
        HashSet<String> result = new HashSet<>();
        for (CodeCheckResultSummaryVo codeCheckResultSummaryVo : allSummaryVo) {
            result.add(codeCheckResultSummaryVo.getResult());
        }
        return result;
    }

    /**
     * 获取pr关联内部codecheck数据
     *
     * @param eventModel 入参
     */
    private void dealWithInnerCodeCheckDataForPr(EventModel eventModel, Long dateNow, CodeCheckDevCloud devCloudByUuid,
                                                 List<String> reposForInner, HashSet<String> resultSetForBCheck, CodeCheckInfo codeCheckInfo) {
        if (Objects.nonNull(eventModel)) {
            // ohpg项
            if (Objects.nonNull(devCloudByUuid)) {
                if (CollectionUtils.isNotEmpty(eventModel.getPrMsg())) {
                    List<PrMsg> prMsg = eventModel.getPrMsg();
                    if (CollectionUtils.isNotEmpty(prMsg)) {
                        for (PrMsg currentPr : prMsg) {
                            int i = currentPr.getUrl().indexOf("/pulls");
                            String url = currentPr.getUrl().substring(0, i);
                            String gitUrlForCurrentPR = currentPr.getUrl().substring(0, i) + ".git";
                            Boolean aBoolean = checkCurrentPRIfInnerRepo(gitUrlForCurrentPR, reposForInner);
                            // 如果 非内部仓库 或者是 ohpg仓库，或者是ark项目，则不显示黄区检查结果
                            // 如果当前pr的蓝区codecheckInfo 为空，说明当前pr未触发codecheck检查，黄区结果不做展示
                            if (!aBoolean || url.equals(OHPG_GIT_URL) || url.startsWith(ARK_PROJECT) || Objects.isNull(codeCheckInfo)
                                    //  蓝区执行了静态检查，但是增量任务详情不存在 ---> 蓝区结果异常
                                    || CollectionUtils.isEmpty(resultSetForBCheck)) {
                                currentPr.setSobCodeCheckResult(" -- ");
                            } else {
                                long timestampInnerCheck = 0;
                                String currentPRResult = devCloudByUuid.getTotalResult();
                                timestampInnerCheck = checkTimeExist(devCloudByUuid, timestampInnerCheck, eventModel);
                                if (Objects.isNull(currentPRResult)) {
                                    currentPr.setSobCodeCheckResult(RUNNING);
                                    // 检查是否超时
                                    if ((dateNow - timestampInnerCheck) > (TimeUtils.HOUR) / 2) {
                                        currentPr.setSobCodeCheckResult("Y-CodeCheck-Time-Out");
                                    }
                                } else {
                                    Map<String, List<CodeCheckDevVo>> subCodeCheckList = devCloudByUuid.getSubCodeCheckList();
                                    List<SubCodeCheckItemsForPrMsg> subCodeCheckItemsForPrMsgList = new ArrayList<>();
                                    if (Objects.nonNull(subCodeCheckList)) {
                                        if (subCodeCheckList.size() == 0) {
                                            currentPr.setSobCodeCheckResult("Y-CodeCheck-Error");
                                            subCodeCheckList.forEach((k, v) -> {
                                                if (Objects.isNull(k)) {
                                                    currentPr.setSobCodeCheckResult("Y-CodeCheck-Error");
                                                }
                                            });
                                        } else {
                                            // 处理所有的黄区检查项目详情
                                            dealWithInnerCodeCheckData(subCodeCheckList, currentPr, subCodeCheckItemsForPrMsgList);
                                        }
                                        // 如果同步过来的所有的PR为空
                                    } else {
                                        currentPr.setSobCodeCheckResult("Y-CodeCheck-Error");
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                List<PrMsg> prMsg = eventModel.getPrMsg();
                if (Objects.isNull(codeCheckInfo)) {
                    prMsg.forEach(currentPr -> currentPr.setSobCodeCheckResult(" -- "));
                } else {
                    checkCurrentPrYCodeCheckStatus(prMsg, reposForInner, resultSetForBCheck);
                    String timestampBaseDOnOutSide = eventModel.getTimestamp();
                    long time = timeUtils.linkedStrToDate(timestampBaseDOnOutSide).getTime();
                    if ((dateNow - time) > (TimeUtils.HOUR) / 2) {
                        prMsg.forEach(pr -> pr.setSobCodeCheckResult("Y-CodeCheck-Time-Out"));
                        // 区分pr是否内部git仓
                        checkCurrentPrYCodeCheckStatus(prMsg, reposForInner, resultSetForBCheck);
                    }
                }
            }
        }
    }

    /**
     * 判断当前pr是否是内部仓或者是ohpg项目
     */
    private Boolean checkCurrentPRIfInnerRepo(String gitUrlForCurrentPR, List<String> reposForInner) {
        return reposForInner.contains(gitUrlForCurrentPR);
    }

    /**
     * 处理黄区的检查数据项
     *
     * @param currentPr                     当前合入请求的某个pr
     * @param subCodeCheckList              pr对应的黄区codecheck检查项目map
     * @param subCodeCheckItemsForPrMsgList eventModel 当前pr对应的黄区codecheck检查结果
     */
    private void dealWithInnerCodeCheckData(Map<String, List<CodeCheckDevVo>> subCodeCheckList, PrMsg currentPr,
                                            List<SubCodeCheckItemsForPrMsg> subCodeCheckItemsForPrMsgList) {
        subCodeCheckList.forEach((k, v) -> {
            if (convertEncodingFormat(k).equals(currentPr.getUrl())) {
                if (CollectionUtils.isNotEmpty(v)) {
                    v.forEach(currentDev -> {
                        SubCodeCheckItemsForPrMsg subCodeCheckItemsForPrMsg = new SubCodeCheckItemsForPrMsg();
                        subCodeCheckItemsForPrMsg.setDetail(Objects.nonNull(currentDev.getDetail()) ? currentDev.getDetail() : "");
                        subCodeCheckItemsForPrMsg.setJsonReport(Objects.nonNull(currentDev.getJsonReport()) ? currentDev.getJsonReport() : "");
                        subCodeCheckItemsForPrMsg.setCodeCheckItems(StringUtils.isNotBlank(currentDev.getSubCodeCheckName()) ? currentDev.getSubCodeCheckName() : "");
                        subCodeCheckItemsForPrMsg.setReport(StringUtils.isNotBlank(currentDev.getReport()) ? convertEncodingFormat(currentDev.getReport()) : "");
                        subCodeCheckItemsForPrMsg.setResult(StringUtils.isNotBlank(currentDev.getResult()) ? currentDev.getResult() : "");
                        subCodeCheckItemsForPrMsgList.add(subCodeCheckItemsForPrMsg);
                    });
                    currentPr.setSobCodeCheckResult(SUCCESS);
                } else {
                    // 同步检查项为空
                    currentPr.setSobCodeCheckResult("Y-CodeCheck-Error");
                }
            }
            currentPr.setSubCodeCheckItemsForPrMsg(subCodeCheckItemsForPrMsgList);
        });
    }

    /**
     * @param str URL 其他编码入参
     * @return afterDecode
     * @Description URL编码格式还原UTF-8
     */
    public String convertEncodingFormat(String str) {
        String decode = "";
        String afterDecode = "";
        try {
            // 二次解码
            decode = URLDecoder.decode(str, "UTF-8");
            afterDecode = URLDecoder.decode(decode, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            LOGGER.error("Decoding URL Exceptions caused by :" + e);
        }
        return afterDecode;
    }

    /**
     * 检查当前当前合入请求所有PR
     */
    private void checkCurrentPrYCodeCheckStatus(List<PrMsg> prMsg, List<String> reposForInner, HashSet<String> resultSetForBCheck) {
        for (PrMsg currentPr : prMsg) {
            int i = currentPr.getUrl().indexOf("/pulls");
            String url = currentPr.getUrl().substring(0, i);
            String gitUrlForCurrentPR = url + ".git";
            Boolean aBoolean = checkCurrentPRIfInnerRepo(gitUrlForCurrentPR, reposForInner);
            // 如果当前pr的蓝区codecheckInfo 为空，说明当前pr未触发codecheck检查，黄区结果 ，不做展示
            if (!aBoolean || url.equals(OHPG_GIT_URL) || url.equals(ARK_PROJECT) || CollectionUtils.isEmpty(resultSetForBCheck)
            ) {
                currentPr.setSobCodeCheckResult(" -- ");
            }
        }
    }

    /**
     * 判断总结果状态
     */
    private void doJudgementForAllCheckResult(CodeCheckInfo codeCheckInfo, String codeCheckStatus,
        EventModel eventModel, String filterForCommunityCheckResult,long dateNow, long dateCreate,
        String resultSetForYCheck, String innerCodeCheckStatus, CodeCheckDevCloud devCloudByUuid
    ) {
        // 首先判断 是否执行了静态检查 以及蓝区的状态
        if (Objects.nonNull(codeCheckInfo)) {
            if (codeCheckStatus.equals("--")) {
                eventModel.setFinalTotalResult("--");
            }
            if (filterForCommunityCheckResult.equals(FAILED)) {
                eventModel.setFinalTotalResult(FAILED);
            } else if (codeCheckStatus.equals(FAILED)) {
                eventModel.setFinalTotalResult("failed_Failed");
            } else if (codeCheckStatus.equals(RUNNING)) {
                eventModel.setFinalTotalResult(RUNNING);
                if ((dateNow - dateCreate) > (TimeUtils.HOUR) / 2) {
                    eventModel.setFinalTotalResult(FAILED);
                }
            } else if (filterForCommunityCheckResult.equals(PASS)) {
                if (resultSetForYCheck.equals(FAILED)) {
                    eventModel.setFinalTotalResult(FAILED);
                } else if (innerCodeCheckStatus.equals(RUNNING)) {
                    if (Objects.isNull(devCloudByUuid.getTotalResult())) {
                        eventModel.setFinalTotalResult(RUNNING);
                        if ((dateNow - dateCreate) > (TimeUtils.HOUR) / 2) {
                            eventModel.setFinalTotalResult(FAILED);
                        }
                    } else {
                        eventModel.setFinalTotalResult(resultSetForYCheck.equals(PASS) ? SUCCESS : FAILED);
                    }
                } else if (resultSetForYCheck.equals(PASS)) {
                    eventModel.setFinalTotalResult(SUCCESS);
                } else {
                    eventModel.setFinalTotalResult(resultSetForYCheck.equals(PASS) ? SUCCESS : FAILED);
                }
            }
        } else {
            // 未触发静态检查，默认成功
            eventModel.setFinalTotalResult(SUCCESS);
        }
    }

    // 组装ProductGraph节点数据
    private GraphNode getProductGraph(EventVo eventVo) {
        GraphNode top = new GraphNode();
        top.setName(eventVo.getUuid());
        top.setType(GraphNodeType.EVENT);
        top.setChildren(new ArrayList<>());
        LinkedHashMap<String, LinkedHashMap<String, ArrayList<Object>>> effectTree = eventVo.getEffectTree();
        effectTree.entrySet().forEach(projectToProduct -> {
            GraphNode project = new GraphNode();
            project.setName(projectToProduct.getKey());
            project.setType(GraphNodeType.PROJECT);
            project.setChildren(new ArrayList<>());
            projectToProduct.getValue().entrySet().forEach(productToTest -> {
                GraphNode product = new GraphNode();
                product.setName(productToTest.getKey());
                product.setType(GraphNodeType.PRODUCT);
                project.getChildren().add(product);
            });
            top.getChildren().add(project);
        });
        return top;
    }

    // 组装TestGraph节点数据
    private GraphNode getTestGraph(EventVo eventVo) {
        GraphNode top = new GraphNode();
        top.setName(eventVo.getUuid());
        top.setType(GraphNodeType.EVENT);
        top.setChildren(new ArrayList<>());
        LinkedHashMap<String, LinkedHashMap<String, ArrayList<Object>>> effectTree = eventVo.getEffectTree();
        effectTree.entrySet().forEach(projectToProduct -> {
            GraphNode project = new GraphNode();
            project.setName(projectToProduct.getKey());
            project.setType(GraphNodeType.PROJECT);
            project.setChildren(new ArrayList<>());
            projectToProduct.getValue().entrySet().forEach(productToTest -> {
                GraphNode product = new GraphNode();
                product.setName(productToTest.getKey());
                product.setType(GraphNodeType.PRODUCT);
                product.setChildren(new ArrayList<>());
                ArrayList<Object> value = productToTest.getValue();
                if (CollectionUtils.isEmpty(value)) {
                    return;
                }
                productToTest.getValue().forEach(test -> {
                    GraphNode testNode = new GraphNode();
                    testNode.setType(GraphNodeType.TEST);
                    testNode.setName(test.toString());
                    product.getChildren().add(testNode);
                });
                project.getChildren().add(product);
            });
            top.getChildren().add(project);
        });
        return top;
    }

    public byte[] exportEvents(String projectName, String branch, String startTime, String endTime,
                               String triggerUser, String buildFailType) {
        // 先获取项目对应的builds
        List<String> titleHead = new ArrayList<>(Arrays.asList("触发人员", "调度时间", "耗时"));
        List<CiTitle> ciTitles = projectDelegate.getCiTitleList(USERNAME, ACCESS_TOKEN);
        if (CollectionUtils.isNotEmpty(ciTitles)) {
            Optional<CiTitle> titleObj = ciTitles.stream()
                    .filter(ciTitle -> projectName.equalsIgnoreCase(ciTitle.getProjectName())).findFirst();
            if (titleObj.isPresent() && CollectionUtils.isNotEmpty(titleObj.get().getBranchs())) {
                Optional<List<CiBuild>> ciBuilds = titleObj.get().getBranchs().stream()
                        .filter(branchInfo -> branch.equalsIgnoreCase(branchInfo.getBranch()))
                        .map(CiBranch::getBuilds).findFirst();
                if (ciBuilds.isPresent()) {
                    for (CiBuild build : ciBuilds.get()) {
                        for (String stepsAlias : build.getBuildStepsAlias()) {
                            titleHead.add(build.getBuildTargets() + WAVE + stepsAlias);
                        }
                    }
                }
            }
        }
        titleHead.addAll(new ArrayList<>(Arrays.asList("静态检查", "pr合入状态", "prCmmitter", "prUrl", "门禁详情页")));
        // 从数据库获取数据
        List<EventExportData> exportList = eventCodeCheckOperation.getExportData(projectName, branch, startTime,
                endTime, triggerUser, buildFailType);

        return ExcelUtil.workbookToBytes(exportEventsExcel(exportList, titleHead.toArray(new String[0])), "event");
    }

    private Workbook exportEventsExcel(List<EventExportData> exportList, String[] titles) {
        XSSFWorkbook workbook = new XSSFWorkbook();
        XSSFSheet sheet = workbook.createSheet("events");
        // 创建表头
        XSSFRow row1 = sheet.createRow(0);
        XSSFRow row2 = sheet.createRow(1);
        for (int i = 0; i < titles.length; i++) {
            XSSFCell row1Cell = row1.createCell(i);
            XSSFCell row2Cell = row2.createCell(i);
            if (i < 3 || i >= titles.length - 5) {
                row1Cell.setCellValue(titles[i]);
                sheet.addMergedRegion(new CellRangeAddress(0, 1, i, i));
            } else {
                String[] build = titles[i].split(WAVE);
                row2Cell.setCellValue(build[1]);
                if (build[0].equalsIgnoreCase(titles[i - 1].split(WAVE)[0])) {
                    sheet.addMergedRegion(new CellRangeAddress(0, 0, i - 1, i));
                    continue;
                }
                row1Cell.setCellValue(build[0]);
            }
        }
        // 表格样式
        XSSFCellStyle cellStyle = workbook.createCellStyle();
        cellStyle.setWrapText(true); // 自动换行
        // 写入门禁数据，从第三行开始
        int rowNum = 2;
        for (EventExportData exportData : exportList) {
            XSSFRow row = sheet.createRow(rowNum);

            row.createCell(0).setCellValue(exportData.getTriggerUser());
            row.createCell(1).setCellValue(exportData.getTimestamp());
            row.createCell(2).setCellValue(exportData.getDuration());
            int length = titles.length;
            Map<String, List<EventBuildTarget>> buildNameMap =
                    exportData.getBuilds().stream().collect(Collectors.groupingBy(EventBuildTarget::getTargetName));
            for (int i = 3; i < length - 5; i++) {
                List<EventBuildTarget> builds = buildNameMap.get(titles[i]);
                if (CollectionUtils.isNotEmpty(builds)) {
                    row.createCell(i).setCellValue(builds.get(0).getResult());
                } else {
                    row.createCell(i).setCellValue(ExcelUtil.DEFAULT_VALUE);
                }
            }
            row.createCell(length - 5).setCellValue(exportData.getCodeCheckTotalResult());
            row.createCell(length - 4).setCellValue(exportData.getPrStatus());
            XSSFCell cellPrCommitter = row.createCell(length - 3);
            cellPrCommitter.setCellValue(exportData.getPrCommitter());
            cellPrCommitter.setCellStyle(cellStyle);
            XSSFCell cellPrUrl = row.createCell(length - 2);
            cellPrUrl.setCellValue(exportData.getPrUrl());
            cellPrUrl.setCellStyle(cellStyle);
            row.createCell(length - 1).setCellValue(exportData.getEventDetailUrl());

            rowNum++;
        }
        // 设置列的宽度，excel中1约等于280个
        sheet.setColumnWidth(0, 4200);
        sheet.setColumnWidth(1, 4200);
        sheet.setColumnWidth(titles.length - 4, 3000);
        sheet.setColumnWidth(titles.length - 3, 4200);
        sheet.setColumnWidth(titles.length - 2, 25000);
        sheet.setColumnWidth(titles.length - 1, 15000);

        return workbook;
    }
}
