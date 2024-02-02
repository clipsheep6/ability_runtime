package com.huawei.impl;

import com.huawei.ci.common.contant.ResponseCode;
import com.huawei.ci.common.entity.cicd.CiProjectQuery;
import com.huawei.ci.common.entity.cicd.CiProjectVo;
import com.huawei.ci.common.pojo.vo.Response;
import com.huawei.entity.vo.buildIntercept.SmokeTest;
import com.huawei.entity.vo.ciinfo.event.EventVo;
import com.huawei.entity.vo.ciinfo.event.PipelineVo;
import com.huawei.entity.vo.codecheck.eventModule.CustomParameterVo;
import com.huawei.entity.vo.codechecksummary.CodeCheckOverViewBuildStable;
import com.huawei.entity.vo.eventbuild.TargetBuildData;
import com.huawei.entity.vo.eventbuild.TargetComponentBuild;
import com.huawei.entity.vo.eventbuild.TargetDateAndBuildTrend;
import com.huawei.entity.vo.pipieline.DailyBuildVo;
import com.huawei.entity.vo.testboard.TestModuleCopyVo;
import com.huawei.enums.CiConstants;
import com.huawei.enums.CodeCheckAccessConstant;
import com.huawei.enums.ConsoleConstants;
import com.huawei.feign.IDailyCodeCheckFeign;
import com.huawei.module.codecheckoverview.CodeCheckOverViewModule;
import com.huawei.operation.*;
import com.huawei.service.codecheck.CodeCheckOverViewService;
import com.huawei.utils.BigDecimalUtil;
import com.huawei.utils.RedisOperateUtils;
import com.huawei.utils.TimeUtils;
import lombok.extern.slf4j.Slf4j;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;
import org.springframework.util.StringUtils;

import java.util.*;
import java.util.stream.Collectors;

import static com.huawei.enums.CiConstants.*;


/**
 * @description
 * @classname com.huawei.impl
 * @since 2023-03-21 15:45
 **/
@Slf4j
@Component
public class CodeCheckOverViewImpl implements CodeCheckOverViewService {

    @Value("${server.environment}")
    public String environment;

    @Autowired
    public EventOperation eventOperation;

    @Autowired
    public CiConfigOperation ciConfigOperation;

    @Autowired
    public CustomParameterOperation customParameterOperation;

    @Autowired
    public CodeCheckBuildOperation codeCheckBuildOperation;

    @Autowired
    public RedisOperateUtils redisOperateUtils;

    @Autowired
    public TimeUtils timeUtils;

    @Autowired
    public RedissonClient redissonClient;

    @Autowired
    public SmokeTestOperation smokeTestOperation;

    @Autowired
    public PipelineOperation pipelineOperation;

    @Autowired
    public EventBuildOperation eventBuildOperation;

    @Autowired
    public TestBoardOperation testBoardOperation;

    @Autowired
    public CacheScheduleServiceImpl scheduleControllerServiceImpl;

    @Autowired
    public IDailyCodeCheckFeign iDailyCodeCheckFeign;


    /**
     * 获取门禁最近的概览信息
     * 如果没时间，默认获取最近一个月天
     *
     * @param codeCheckOverViewModule {@linkplain CodeCheckOverViewModule}
     * @return Response  {@linkplain Response}
     */
    @Override
    public Response getOverViewSummary(CodeCheckOverViewModule codeCheckOverViewModule) {
        HashMap<String, Object> totalMap = new HashMap<>();
        if (org.apache.commons.lang.StringUtils.isBlank(codeCheckOverViewModule.getProjectName())
                || org.apache.commons.lang.StringUtils.isBlank(codeCheckOverViewModule.getBranch())) {
            return new Response(ResponseCode.SUCCESS.getCode(), ResponseCode.SUCCESS.getMessage(), totalMap);
        }
        // 门禁 top
        HashMap<String, Object> eventMap = eventOperation.getEventByCondition(codeCheckOverViewModule);
        // 测试
        HashMap<String, Object> testRateMap = getCodeCheckBuildDataInfoByProjectAndBranch(codeCheckOverViewModule);
        // 构建
        HashMap<String, Double> dailyBuildSuccessRate = getDailySuccessRate(codeCheckOverViewModule);
        // 类型
        HashMap<String, Object> totalMapByType = dealWithCodeCheckByType(codeCheckOverViewModule.getType(), eventMap, codeCheckOverViewModule);
        eventMap.entrySet().removeIf(next -> next.getKey().equals("codeCheckCountList") || next.getKey().equals("mappedResults"));
        totalMap.put("eventMap", eventMap);
        totalMap.put("testRateMap", testRateMap);
        totalMap.put("totalMapByType", totalMapByType);
        totalMap.put("dailyBuildSuccessRate", dailyBuildSuccessRate);
        return new Response(ResponseCode.SUCCESS.getCode(), ResponseCode.SUCCESS.getMessage(), totalMap);
    }

    /**
     * 获取每日构建成功率
     */
    private HashMap<String, Double> getDailySuccessRate(CodeCheckOverViewModule codeCheckOverViewModule) {
        HashMap<String, Double> dailyBuildSuccessRateMap = new HashMap<>();
        List<DailyBuildVo> dailyPipeLineBuildSuccessRate = codeCheckBuildOperation
                .getDailyPipeLineBuildSuccessRate(codeCheckOverViewModule);
        // 总数
        List<DailyBuildVo> collect = dailyPipeLineBuildSuccessRate.stream().filter(mappedResult ->
                Long.parseLong(codeCheckOverViewModule.getStartTime()) <= Long.parseLong(mappedResult.getBuildStartTime())
                        && Long.parseLong(codeCheckOverViewModule.getEndTime()) >= Long.parseLong(mappedResult.getBuildStartTime())
        ).collect(Collectors.toList());
        long aTrue = collect.stream().filter(collectOne -> !StringUtils.isEmpty(collectOne.getBuildSuccess())
                && collectOne.getBuildSuccess().equals("true")
        ).count();
        long count = collect.size() == NUM_ZERO ? CiConstants.NUM_ONE : collect.size();
        double v = BigDecimalUtil.divideHalfUp(aTrue * CiConstants.ONE_HUNDRED, count, CiConstants.NUM_TWO);
        dailyBuildSuccessRateMap.put("successRate", v);
        return dailyBuildSuccessRateMap;
    }

    private HashMap<String, Object> dealWithCodeCheckByType(Integer type, HashMap<String, Object> eventMap,
                                                            CodeCheckOverViewModule codeCheckOverViewModule) {
        // 类型 1: 代码门禁  2: 每日构建  3: 代码质量
        if (type == CiConstants.NUM_THREE) {
            return doJudgementByOverViewTypeCodeCheckQuality(codeCheckOverViewModule);
        } else if (type == CiConstants.NUM_TWO) {
            return doJudgementByOverViewTypeDailyBuild(codeCheckOverViewModule);
        } else {
            return doJudgementByOverViewTypeCodeCheck(eventMap, codeCheckOverViewModule);
        }
    }

    private HashMap<String, Object> doJudgementByOverViewTypeCodeCheck(HashMap<String, Object> eventMap,
                                                                       CodeCheckOverViewModule codeCheckOverViewModule) {
        List<EventVo> codeCheckCountList = new ArrayList<>();
        List<EventVo> mappedResults = new ArrayList<>();
        HashMap<String, Object> codeCheckAccess = new HashMap<>();
        if (eventMap.get("codeCheckCountList") instanceof List)
            codeCheckCountList = Collections.unmodifiableList((List<EventVo>) eventMap.get("codeCheckCountList"));
        if (eventMap.get("mappedResults") instanceof List)
            mappedResults = Collections.unmodifiableList((List<EventVo>) eventMap.get("mappedResults"));
        dealWithRepeat(codeCheckCountList);
        codeCheckAccess.put("buildTrend", dealWithCodeCheckBuildTrend(codeCheckCountList));
        codeCheckAccess.put("prTrend", dealWithCodeCheckPRTrend(codeCheckCountList));
        codeCheckAccess.put("successRateTrend", dealWithCodeCheckSuccessRateTrend(codeCheckOverViewModule));
        codeCheckAccess.put("efficacyTrend", dealWithCodeCheckEfficacyTrend(mappedResults));
        codeCheckAccess.put("stableTrend", dealWithCodeCheckStableTrend(codeCheckOverViewModule));
        return codeCheckAccess;
    }

    private void dealWithRepeat(List<EventVo> codeCheckCountList) {
        codeCheckCountList.forEach(eventVo -> {
            eventVo.setTimestamp(eventVo.getTimestamp().substring(NUM_ZERO, CiConstants.NUM_EIGHT));
            eventVo.setUuid(String.valueOf(eventVo.getPrMsg().size()));
        });
    }

    private HashMap<String, Object> dealWithCodeCheckStableTrend(CodeCheckOverViewModule codeCheckOverViewModule) {
        HashMap<String, Object> codecheckStableMap = new HashMap<>();
        HashMap<String, List<PipelineVo>> codecheckStable = pipelineOperation.getPipelineByOverViewCondition(codeCheckOverViewModule);
        List<PipelineVo> collectAll = codecheckStable.get("collectAll");
        List<PipelineVo> collectFailed = codecheckStable.get("collectFailed");
        // filter
        int allSize = (int) collectAll.stream()
                .filter(pipeVo -> org.apache.commons.lang3.StringUtils.isNotBlank(pipeVo.getTimestamp())
                        && timeUtils.linkedDateStrToLong(codeCheckOverViewModule.getStartTime())
                        <= timeUtils.linkedDateStrToLong(pipeVo.getTimestamp())
                        && timeUtils.linkedDateStrToLong(pipeVo.getTimestamp())
                        <= timeUtils.linkedDateStrToLong(codeCheckOverViewModule.getEndTime())).count();
        List<PipelineVo> collectFailedForCondition = collectFailed.stream()
                .filter(pipeVo -> org.apache.commons.lang3.StringUtils.isNotBlank(pipeVo.getTimestamp())
                        && timeUtils.linkedDateStrToLong(codeCheckOverViewModule.getStartTime())
                        <= timeUtils.linkedDateStrToLong(pipeVo.getTimestamp())
                        && timeUtils.linkedDateStrToLong(pipeVo.getTimestamp())
                        <= timeUtils.linkedDateStrToLong(codeCheckOverViewModule.getEndTime())).collect(Collectors.toList());
        long businessFailed = collectFailedForCondition.stream().filter(failed ->
                CodeCheckAccessConstant.businessFailedTypes.contains(failed.getBuildFailType())).count();
        long toolFailed = collectFailedForCondition.stream().filter(failed ->
                CodeCheckAccessConstant.toolFailedTypes.contains(failed.getBuildFailType())).count();
        long environmentFailed = collectFailedForCondition.stream().filter(failed ->
                CodeCheckAccessConstant.environmentFailedTypes.contains(failed.getBuildFailType())).count();
        // count
        int totalFailed = (int) (businessFailed + toolFailed + environmentFailed);
        int failed = totalFailed == NUM_ZERO ? CiConstants.NUM_ONE : totalFailed;
        int size = allSize == NUM_ZERO ? CiConstants.NUM_ONE : allSize;
        int totalSuccess = size - totalFailed;
        codecheckStableMap.put("totalFailed", totalFailed);
        codecheckStableMap.put("totalSuccess", totalSuccess);
        codecheckStableMap.put("successRate", BigDecimalUtil.divideHalfUp(totalSuccess
                * CiConstants.ONE_HUNDRED, size, CiConstants.NUM_TWO));
        codecheckStableMap.put("failedRate", BigDecimalUtil.divideHalfUp(totalFailed
                * CiConstants.ONE_HUNDRED, size, CiConstants.NUM_TWO));
        codecheckStableMap.put("businessFailedRate", BigDecimalUtil.divideHalfUp(businessFailed
                * CiConstants.ONE_HUNDRED, failed, CiConstants.NUM_TWO));
        codecheckStableMap.put("toolFailedRate", BigDecimalUtil.divideHalfUp(toolFailed
                * CiConstants.ONE_HUNDRED, failed, CiConstants.NUM_TWO));
        codecheckStableMap.put("environmentFailedRate", BigDecimalUtil.divideHalfUp(environmentFailed
                * CiConstants.ONE_HUNDRED, failed, CiConstants.NUM_TWO));
        return codecheckStableMap;
    }

    private HashMap<String, Object> dealWithCodeCheckPRTrend(List<EventVo> codeCheckCountList) {
        HashMap<String, Object> eventCodeCheckPRTrendCountMap = new HashMap<>();
        codeCheckCountList.stream()
                .collect(Collectors.groupingBy(EventVo::getTimestamp)).forEach((k, v) -> {
            LinkedList<Integer> prCount = new LinkedList<>();
            v.forEach(eventVo -> prCount.add(Integer.parseInt(eventVo.getUuid())));
            eventCodeCheckPRTrendCountMap.put(k, prCount.stream().reduce(NUM_ZERO, Integer::sum));
        });
        return eventCodeCheckPRTrendCountMap;
    }

    private HashMap<String, Object> dealWithCodeCheckBuildTrend(List<EventVo> codeCheckCountList) {
        HashMap<String, Object> eventCodeCheckBuildCountMap = new HashMap<>();
        codeCheckCountList.stream()
                .collect(Collectors.groupingBy(EventVo::getTimestamp))
                .forEach((k, v) -> eventCodeCheckBuildCountMap.put(k, v.size()));
        return eventCodeCheckBuildCountMap;
    }

    private List<CodeCheckOverViewBuildStable> dealWithCodeCheckSuccessRateTrend(CodeCheckOverViewModule codeCheckOverViewModule) {
        List<CodeCheckOverViewBuildStable> buildStableList = new ArrayList<>();
        List<SmokeTest> smokeTests = smokeTestOperation.countSmokeTestByDate(codeCheckOverViewModule);
        // filter
        List<SmokeTest> smokeTestList = smokeTests.stream()
                .filter(smokeTestVo -> org.apache.commons.lang3.StringUtils.isNotBlank(smokeTestVo.getCountDate())
                        && Long.parseLong(codeCheckOverViewModule.getStartTime().substring(NUM_ZERO,
                        CiConstants.NUM_EIGHT))
                        <= timeUtils.transDate2LinkedDate(smokeTestVo.getCountDate())
                        && timeUtils.transDate2LinkedDate(smokeTestVo.getCountDate())
                        <= Long.parseLong(codeCheckOverViewModule.getEndTime().substring(NUM_ZERO,
                        CiConstants.NUM_EIGHT)))
                .collect(Collectors.toList());
        // for i
        smokeTestList.forEach(smokeTest -> {
            CodeCheckOverViewBuildStable.CodeCheckOverViewBuildStableBuilder builder = CodeCheckOverViewBuildStable.builder();
            CodeCheckOverViewBuildStable build = builder.countDate(timeUtils.transDate2LinkedDate(smokeTest.getCountDate()))
                    .successRate(BigDecimalUtil.divideHalfUp(
                            Double.parseDouble(String.valueOf(smokeTest.getSuccess())) * CiConstants.ONE_HUNDRED,
                            Double.parseDouble(smokeTest.getUuid()),
                            CiConstants.NUM_TWO))
                    .countProblem(smokeTest.getCriticalProblem() + smokeTest.getFatalProblem())
                    .build();
            buildStableList.add(build);
        });
        return buildStableList.stream()
                .sorted(Comparator.comparing(CodeCheckOverViewBuildStable::getCountDate)).collect(Collectors.toList());
    }

    private HashMap<String, Object> dealWithCodeCheckEfficacyTrend(List<EventVo> mappedResults) {
        HashMap<String, Object> codeCheckEfficacyMap = new HashMap<>();
        int size = mappedResults.size();
        if (size > NUM_ZERO) {
            int countUnderFifteen = (int) mappedResults.stream().filter(eventVo ->
                    Objects.nonNull(eventVo.getDuration()) && eventVo.getDuration() < CodeCheckAccessConstant.TIME_LONG_UNDER_15)
                    .count();
            int countThirtyAbove = (int) mappedResults.stream().filter(eventVo ->
                    Objects.nonNull(eventVo.getDuration()) && eventVo.getDuration() > CodeCheckAccessConstant.TIME_LONG_ABOVE_30)
                    .count();
            int countBetweenTwentyAndThirty = (int) mappedResults.stream()
                    .filter(eventVo -> Objects.nonNull(eventVo.getDuration()) && eventVo.getDuration()
                            >= CodeCheckAccessConstant.TIME_LONG_UNDER_20 && eventVo.getDuration()
                            <= CodeCheckAccessConstant.TIME_LONG_ABOVE_30).count();
            int countBetweenFifteenAndTwenty = size - countUnderFifteen - countThirtyAbove - countBetweenTwentyAndThirty;
            codeCheckEfficacyMap.put("under15", BigDecimalUtil.divideHalfUp(countUnderFifteen
                    * CiConstants.ONE_HUNDRED, size, CiConstants.NUM_TWO));
            codeCheckEfficacyMap.put("between15And20", BigDecimalUtil.divideHalfUp(countBetweenFifteenAndTwenty
                    * CiConstants.ONE_HUNDRED, size, CiConstants.NUM_TWO));
            codeCheckEfficacyMap.put("between20And30", BigDecimalUtil.divideHalfUp(countBetweenTwentyAndThirty
                    * CiConstants.ONE_HUNDRED, size, CiConstants.NUM_TWO));
            codeCheckEfficacyMap.put("above30", BigDecimalUtil.divideHalfUp(countThirtyAbove
                    * CiConstants.ONE_HUNDRED, size, CiConstants.NUM_TWO));
        }
        return codeCheckEfficacyMap;
    }

    // daily build
    private HashMap<String, Object> doJudgementByOverViewTypeDailyBuild(CodeCheckOverViewModule codeCheckOverViewModule) {
        HashMap<String, Object> dailyBuildTrend = new HashMap<>();
        CustomParameterVo othersComponentConfiguration = customParameterOperation.getCustomParameterByConfiguration
                (ConsoleConstants.TEST_BOARD_COMPONENT);
        //  xts tdd fuzz
        HashMap<String, List<String>> tddAndXtsAndFuzz = scheduleControllerServiceImpl
                .getTddAndXtsAndFuzz(codeCheckOverViewModule, othersComponentConfiguration);
        List<String> tdd = tddAndXtsAndFuzz.get("tddComponent");
        List<String> xts = tddAndXtsAndFuzz.get("xtsComponent");
        List<String> fuzz = tddAndXtsAndFuzz.get("fuzzComponent");
        ArrayList<String> strings = new ArrayList<>();
        strings.addAll(tdd);
        strings.addAll(xts);
        strings.addAll(fuzz);
        // duration: abt 300ms
        HashMap<String, Object> dailyBuildDataAnalysis = getDailyBuildTrendInfo(codeCheckOverViewModule);
        // duration: abt 0.8 S
        HashMap<String, Object> dailyEquipmentTestingAnalysis = getCodeCheckBuildDataInfoByProjectAndBranch(codeCheckOverViewModule,
                tdd, xts, fuzz);
        // duration: abt 200ms
        HashMap<String, Object> dailyBuildProblemSummary = getDailyBuildPipeLineInfo(codeCheckOverViewModule, strings);
        dailyBuildTrend.put("dailyBuildDataAnalysis", dailyBuildDataAnalysis);
        dailyBuildTrend.put("dailyEquipmentTestingAnalysis", dailyEquipmentTestingAnalysis);
        dailyBuildTrend.put("dailyBuildProblemSummary", dailyBuildProblemSummary);
        return dailyBuildTrend;
    }

    /**
     * 构建问题汇总
     *
     * @return HashMap<String, Object>
     */
    private HashMap<String, Object> getDailyBuildPipeLineInfo(CodeCheckOverViewModule codeCheckOverViewModule,
                                                              ArrayList<String> strings) {
        HashMap<String, Double> rateMap = new HashMap<>();
        HashMap<String, Object> failedMap = new HashMap<>();
        HashMap<String, Object> dailyBuildProblemsMap = new HashMap<>();
        HashMap<String, Object> groupedByDailyFailedMap = new HashMap<>();
        // filter
        List<DailyBuildVo> pipelinesList = codeCheckBuildOperation.getPipelinesByBuildItem(codeCheckOverViewModule, strings).stream()
                .filter(dailyBuildVo -> org.apache.commons.lang3.StringUtils.isNotBlank(dailyBuildVo.getBuildStartTime())
                        && Long.parseLong(codeCheckOverViewModule.getStartTime())
                        <= Long.parseLong(dailyBuildVo.getBuildStartTime())
                        && Long.parseLong(dailyBuildVo.getBuildStartTime())
                        <= Long.parseLong(codeCheckOverViewModule.getEndTime())).collect(Collectors.toList());
        int size = pipelinesList.size();
        // sum
        pipelinesList.stream().collect(Collectors.groupingBy(DailyBuildVo::getComponent))
                .forEach((kc, vc) -> {
                    HashMap<String, Long> failedType = new HashMap<>();
                    dealWithBuildFailedTypeByItem(vc, CodeCheckAccessConstant.businessFailedTypes);
                    long businessFailed = dealWithBuildFailedTypeByItem(vc, CodeCheckAccessConstant.businessFailedTypes);
                    long toolFailed = dealWithBuildFailedTypeByItem(vc, CodeCheckAccessConstant.toolFailedTypes);
                    long environmentFailed = dealWithBuildFailedTypeByItem(vc, CodeCheckAccessConstant.environmentFailedTypes);
                    failedType.put("businessFailed", businessFailed);
                    failedType.put("toolFailed", toolFailed);
                    failedType.put("environmentFailed", environmentFailed);
                    failedMap.put(kc, failedType);
                    rateMap.put(kc, BigDecimalUtil.divideHalfUp(vc.size() * CiConstants.ONE_HUNDRED, size, CiConstants.NUM_TWO));
                });
        pipelinesList.forEach(currentBuildOne -> {
            currentBuildOne.setBuildStartTime(currentBuildOne.getBuildStartTime().substring(NUM_ZERO, CiConstants.NUM_EIGHT));
        });
        pipelinesList.stream().collect(Collectors.groupingBy(DailyBuildVo::getBuildStartTime))
                .forEach((kd, vc) -> {
                    HashMap<String, Long> failedType = new HashMap<>();
                    long businessFailed = dealWithBuildFailedTypeByItem(vc, CodeCheckAccessConstant.businessFailedTypes);
                    long toolFailed = dealWithBuildFailedTypeByItem(vc, CodeCheckAccessConstant.toolFailedTypes);
                    long environmentFailed = dealWithBuildFailedTypeByItem(vc, CodeCheckAccessConstant.environmentFailedTypes);
                    failedType.put("businessFailed", businessFailed);
                    failedType.put("toolFailed", toolFailed);
                    failedType.put("environmentFailed", environmentFailed);
                    groupedByDailyFailedMap.put(kd, failedType);
                });
        dailyBuildProblemsMap.put("groupedByDailyFailedMap", groupedByDailyFailedMap);
        dailyBuildProblemsMap.put("rateMap", rateMap);
        dailyBuildProblemsMap.put("failedMap", failedMap);
        return dailyBuildProblemsMap;
    }

    private long dealWithBuildFailedTypeByItem(List<DailyBuildVo> vc, List<String> businessFailedTypes) {
        return CollectionUtils.isEmpty(vc) ? CiConstants.NUM_ZERO :
                vc.stream().filter(failed -> businessFailedTypes.contains(failed.getBuildFailType())).count();
    }

    private HashMap<String, Object> getDailyBuildTrendInfo(CodeCheckOverViewModule codeCheckOverViewModule) {
        HashMap<String, Object> resultMap = new HashMap<>();
        LinkedList<TargetBuildData> eventLinkedList = eventBuildOperation.getBuildDataByProjectAndBranchWithTime
                (codeCheckOverViewModule);
        List<TargetDateAndBuildTrend> targetDateAndBuildTrendList = new LinkedList<>();
        // group
        eventLinkedList.stream().collect(Collectors.groupingBy(TargetBuildData::getDate,
                Collectors.groupingBy(TargetBuildData::getComponent))).forEach((buildDataKey, buildDataValue) -> {
            for (Map.Entry<String, List<TargetBuildData>> buildMap : buildDataValue.entrySet()) {
                targetDateAndBuildTrendList.add(TargetDateAndBuildTrend.builder()
                        .component(buildMap.getKey())
                        .date(buildDataKey)
                        .buildCount(buildMap.getValue().stream().mapToInt(TargetBuildData::getAllTask).sum())
                        .build());
            }
        });
        //  grouped by date and component for Top 10
        Map<String, List<TargetDateAndBuildTrend>> targetDateAndBuildTrendMapTemp = new HashMap<>();
        targetDateAndBuildTrendList.stream().collect(Collectors.groupingBy(TargetDateAndBuildTrend::getDate))
                .forEach((k, v) -> targetDateAndBuildTrendMapTemp.put(k, v
                        .stream().sorted(Comparator.comparing(TargetDateAndBuildTrend::getBuildCount).reversed())
                        .limit(CiConstants.SORT_SIZE)
                        .collect(Collectors.toList())));
        // size success successRate  ave time
        LinkedList<TargetComponentBuild> componentBuildLinkedList = new LinkedList<>();
        eventLinkedList.stream().collect(Collectors.groupingBy(TargetBuildData::getComponent)).forEach((k, v) -> {
            int allTask = v.stream().mapToInt(TargetBuildData::getAllTask).sum();
            int successTask = v.stream().mapToInt(TargetBuildData::getAllSuccessTask).sum();
            int successTaskPercent = successTask * CiConstants.ONE_HUNDRED;
            componentBuildLinkedList.add(TargetComponentBuild.builder().componentName(k)
                    .allTask(allTask)
                    .successTask(successTask)
                    .successRate(BigDecimalUtil.divideHalfUp(successTaskPercent, allTask, CiConstants.NUM_TWO))
                    .averageDuration(BigDecimalUtil.divideHalfUp(v.stream().mapToDouble(TargetBuildData::getAverageDuration).sum(),
                            v.size(), CiConstants.NUM_TWO)).build());
        });
        List<TargetComponentBuild> targetComponentBuilds = componentBuildLinkedList.stream()
                .sorted(Comparator.comparing(TargetComponentBuild::getAllTask).reversed())
                .limit(CiConstants.SORT_SIZE)
                .collect(Collectors.toList());
        int sumForAllTask = targetComponentBuilds.stream().mapToInt(TargetComponentBuild::getAllTask).sum();
        targetComponentBuilds.forEach(targetComponentBuild -> targetComponentBuild.setRate(BigDecimalUtil
                .divideHalfUp(targetComponentBuild.getAllTask() * CiConstants.ONE_HUNDRED, sumForAllTask, CiConstants.NUM_TWO)));
        // sort
        LinkedHashMap<String, List<TargetDateAndBuildTrend>> targetDateAndBuildTrendMap =
                targetDateAndBuildTrendMapTemp.entrySet()
                        .stream()
                        .sorted(Comparator.comparingLong((CodeCheckOverViewImpl::getTargetBuildKey)))
                        .collect(LinkedHashMap::new, (m, e) -> m.put(e.getKey(), e.getValue()), LinkedHashMap::putAll);
        resultMap.put("targetDateAndBuildTrendMap", targetDateAndBuildTrendMap);
        resultMap.put("targetComponentBuilds", targetComponentBuilds);
        return resultMap;
    }

    private static long getTargetBuildKey(Object vo) {
        return Long.parseLong(((Map.Entry<String, List<TargetDateAndBuildTrend>>) vo)
                .getKey().replaceAll("-", ""));
    }

    private HashMap<String, Object> getCodeCheckBuildDataInfoByProjectAndBranch(CodeCheckOverViewModule codeCheckOverViewModule,
                                                                                List<String> tdd, List<String> xts, List<String> fuzz) {
        HashMap<String, Object> dailyBuildMap = new HashMap<>();
        String type = codeCheckOverViewModule.getDailyBuildType();
        String Component = doJudgementByOverViewType(type);
        long start = Long.parseLong(codeCheckOverViewModule.getStartTime().substring(NUM_ZERO, NUM_EIGHT));
        long end = Long.parseLong(codeCheckOverViewModule.getEndTime().substring(NUM_ZERO, NUM_EIGHT));
        // module list
        List<TestModuleCopyVo> allModuleList = testBoardOperation.getTestComponentByComponentAll
                (codeCheckOverViewModule, Component, tdd, xts, fuzz).stream().
                filter(module -> start <= module.getBuildStartTime()
                        && module.getBuildStartTime() <= end).collect(Collectors.toList());
        // module
        LinkedHashMap<String, List<TestModuleCopyVo>> totalMapByItem = allModuleList.stream()
                .collect(Collectors.groupingBy(TestModuleCopyVo::getItems))
                .entrySet()
                .stream()
                .sorted(Comparator.comparing(CodeCheckOverViewImpl::buildSize).reversed())
                .limit(CiConstants.SORT_SIZE)
                .collect(LinkedHashMap::new, (m, e) -> m.put(e.getKey(), e.getValue()), LinkedHashMap::putAll);
        // date
        Map<Long, List<TestModuleCopyVo>> totalMapByDate = allModuleList.stream()
                .collect(Collectors.groupingBy(TestModuleCopyVo::getBuildStartTime));
        HashMap<String, Object> itemMaps = new HashMap<>();
        totalMapByItem.forEach((k, v) -> {
            HashMap<String, Object> itemMap = new HashMap<>();
            int size = v.size() == NUM_ZERO ? CiConstants.NUM_ONE : v.size();
            long passed = v.stream().filter(moduleVo -> moduleVo.getResult().equals("passed")).count();
            Double duration = v.stream().collect(Collectors.averagingDouble(TestModuleCopyVo::getTime));
            itemMap.put("passRate", BigDecimalUtil.divideHalfUp(passed * CiConstants.ONE_HUNDRED, size, CiConstants.NUM_TWO));
            itemMap.put("duration", Double.parseDouble(String.format(DOUBLE_KEEP_LONG, duration * size)));
            itemMap.put("totalSize", v.size() == NUM_ZERO ? NUM_ZERO : v.size());
            itemMaps.put(k, itemMap);
        });
        HashMap<Long, LinkedHashMap<String, Double>> dateAndItem = new HashMap<>();
        Set<String> strings = itemMaps.keySet();
        // sum duration by build size
        totalMapByDate.forEach((k, v) -> {
            LinkedHashMap<String, Double> collectByItem = v.stream()
                    .filter(module -> strings.contains(module.getItems()))
                    .collect(Collectors.toList()).stream()
                    .collect(Collectors.groupingBy(TestModuleCopyVo::getItems))
                    .entrySet()
                    .stream()
                    .sorted(Comparator.comparing(CodeCheckOverViewImpl::buildSize).reversed())
                    .limit(CiConstants.SORT_SIZE)
                    .collect(LinkedHashMap::new, (m, e) -> m.put(e.getKey(), Double.parseDouble(String.format(DOUBLE_KEEP_LONG,
                            e.getValue().stream().mapToDouble(TestModuleCopyVo::getTime).sum()))),
                            LinkedHashMap::putAll);
            dateAndItem.put(k, collectByItem);
        });
        LinkedHashMap<Long, LinkedHashMap<String, Double>> buildTrend = dateAndItem.entrySet().stream()
                .sorted(Comparator.comparingDouble(CodeCheckOverViewImpl::dateTime))
                .collect(LinkedHashMap::new, (m, e) -> m.put(e.getKey(), e.getValue()), LinkedHashMap::putAll);
        dailyBuildMap.put("buildTrend", buildTrend);
        dailyBuildMap.put("itemMaps", itemMaps);
        return dailyBuildMap;
    }

    private static <T> double dateTime(T t) {
        return ((Map.Entry<Long, LinkedHashMap<String, Double>>) t).getKey();
    }

    private static double buildSize(Map.Entry<String, List<TestModuleCopyVo>> stringListEntry) {
        return stringListEntry.getValue().size();
    }


    private String doJudgementByOverViewType(String dailyBuildType) {
        if (dailyBuildType.equals(CiConstants.TYPE_THREE)) {
            return "xts";
        } else if (dailyBuildType.equals(CiConstants.TYPE_TWO)) {
            return "fuzz";
        } else {
            return "tdd";
        }
    }

    // codecheck quality
    private HashMap<String, Object> doJudgementByOverViewTypeCodeCheckQuality(CodeCheckOverViewModule codeCheckOverViewModule) {
        HashMap<String, Object> dailyCodeCheckMap = new HashMap<>();
        // static check summary
        LinkedHashMap<String, Object> staticCheck = getCodeCheckDailyCheck(codeCheckOverViewModule);
        // bad smell summary

        // test cover summary

        // safety check summary

        // in-rule check summary

        // stuff design summary
        dailyCodeCheckMap.put("staticCheck", staticCheck);
        return dailyCodeCheckMap;
    }

    private LinkedHashMap<String, Object> getCodeCheckDailyCheck(CodeCheckOverViewModule codeCheckOverViewModule) {
        Response lastBugs = iDailyCodeCheckFeign.getLastestBugs(codeCheckOverViewModule.getProjectName(),
                codeCheckOverViewModule.getBranch());
        Object data = lastBugs.getData();
        if (data instanceof LinkedHashMap) {
            return (LinkedHashMap<String, Object>) data;
        }
        return new LinkedHashMap<>();
    }

    public HashMap<String, Object> getCodeCheckBuildDataInfoByProjectAndBranch(CodeCheckOverViewModule codeCheckOverViewModule) {
        HashMap<String, Object> successRateMap = new HashMap<>();
        List<String> tddComponent = new ArrayList<>();
        // component
        CiProjectQuery ciProjectQuery = new CiProjectQuery();
        ciProjectQuery.setNameSpace(codeCheckOverViewModule.getProjectName());
        ciProjectQuery.setManifestBranch(codeCheckOverViewModule.getBranch());
        List<CiProjectVo> ciProjectList = new ArrayList<>(ciConfigOperation.getProjectVo(ciProjectQuery));
        // TDD
        ciProjectList.forEach(ciProjectVo -> {
            List<Map<String, Object>> dailyPipelineList = ciProjectVo.getDailyPipelineList();
            if (!CollectionUtils.isEmpty(dailyPipelineList)) {
                tddComponent.addAll(getTddComponent(dailyPipelineList));
            }
        });
        CustomParameterVo othersComponentConfiguration = customParameterOperation
                .getCustomParameterByConfiguration(ConsoleConstants.TEST_BOARD_COMPONENT);
        // XTS
        Object xtsObject = othersComponentConfiguration.getParameters().get(ConsoleConstants.XTS);
        List<String> xts = new ArrayList<>();
        if (Objects.nonNull(xtsObject) && xtsObject instanceof String) {
            tddComponent.add(xtsObject.toString());
            xts.add(xtsObject.toString());
        } else if (Objects.nonNull(xtsObject) && xtsObject instanceof List) {
            tddComponent.addAll((List) xtsObject);
            xts.addAll((List) xtsObject);
        }
        // build info
        List<DailyBuildVo> dailyBuildLists = codeCheckBuildOperation.getBuildsByComponent(tddComponent, ciProjectQuery);
        // filter
        List<DailyBuildVo> dailyBuildList = dailyBuildLists.stream().filter(result ->
                timeUtils.linkedDateStrToLong(codeCheckOverViewModule.getStartTime())
                        <= timeUtils.linkedDateStrToLong(result.getBuildStartTime())
                        && timeUtils.linkedDateStrToLong(org.apache.commons.lang3.StringUtils.isBlank(result.getBuildEndTime())
                        ? result.getBuildStartTime() : result.getBuildEndTime())
                        <= timeUtils.linkedDateStrToLong(codeCheckOverViewModule.getEndTime())).collect(Collectors.toList());
        // collectXTS
        List<DailyBuildVo> collectXTS = dailyBuildList.stream()
                .filter(dailyBuildVo -> xts.contains(dailyBuildVo.getComponent())).collect(Collectors.toList());
        int collectXTSSuccess = (int) collectXTS.stream()
                .filter(buildVo -> org.apache.commons.lang3.StringUtils.isNotBlank(buildVo.getBuildSuccess())
                        && buildVo.getBuildSuccess().equals("true")).count();
        // collectTDD
        List<DailyBuildVo> collectTDD = dailyBuildList.stream()
                .filter(dailyBuildVo -> !xts.contains(dailyBuildVo.getComponent())).collect(Collectors.toList());
        int collectTDDSuccess = (int) collectTDD.stream()
                .filter(buildVo -> org.apache.commons.lang3.StringUtils.isNotBlank(buildVo.getBuildSuccess())
                        && buildVo.getBuildSuccess().equals("true")).count();
        // calculator
        double XTSSuccessRate = BigDecimalUtil.divideHalfUp(Double.parseDouble(String.valueOf(collectXTSSuccess))
                        * CiConstants.ONE_HUNDRED
                , Double.parseDouble(String.valueOf(collectXTS.size() == NUM_ZERO ? CiConstants.NUM_ONE : collectXTS.size())),
                CiConstants.NUM_TWO);

        double TDDSuccessRate = BigDecimalUtil.divideHalfUp(Double.parseDouble(String.valueOf(collectTDDSuccess))
                        * CiConstants.ONE_HUNDRED
                , Double.parseDouble(String.valueOf(collectTDD.size() == NUM_ZERO ? CiConstants.NUM_ONE : collectTDD.size())),
                CiConstants.NUM_TWO);
        successRateMap.put("XTSSuccessRate", XTSSuccessRate);
        successRateMap.put("TDDSuccessRate", TDDSuccessRate);
        return successRateMap;
    }

    /**
     * daily build pipeline info
     *
     * @param dailyPipelineList pipeline info
     * @return List<String>
     */
    public List<String> getTddComponent(List<Map<String, Object>> dailyPipelineList) {
        List<String> tddComponent = new ArrayList<>();
        dailyPipelineList.forEach(dailyPipeline -> {
            List<Map<String, Object>> testList = (List<Map<String, Object>>) dailyPipeline.get("testList");
            if (!CollectionUtils.isEmpty(testList)) {
                for (Map<String, Object> testModel : testList) {
                    if (testModel.containsKey("TDDPath") && !StringUtils.isEmpty(testModel.get("TDDPath"))) {
                        tddComponent.add(dailyPipeline.get("name").toString());
                        break;
                    }
                }
            }
        });
        return tddComponent;
    }
}
