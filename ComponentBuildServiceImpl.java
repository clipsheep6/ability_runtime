package com.huawei.impl;

import com.huawei.ci.common.pojo.vo.Response;
import com.huawei.ci.common.utils.BigDecimalUtil;
import com.huawei.ci.common.utils.ExcelUtil;
import com.huawei.ci.common.utils.JsonUtils;
import com.huawei.entity.pojo.BundleCodeResultSummary;
import com.huawei.entity.pojo.CodeCheckResultSummaryVo;
import com.huawei.entity.vo.ciinfo.codecheck.CodeCheckInfo;
import com.huawei.entity.vo.ciinfo.codecheck.EventModel;
import com.huawei.entity.vo.ciinfo.event.CodeCheckDevCloud;
import com.huawei.entity.vo.ciinfo.event.EventVo;
import com.huawei.entity.vo.ciinfo.event.PipelineVo;
import com.huawei.entity.vo.codecheck.eventModule.CustomParameterVo;
import com.huawei.entity.vo.componentbuild.CacheCount;
import com.huawei.entity.vo.componentbuild.CacheHitRate;
import com.huawei.entity.vo.componentbuild.ComponentBuild;
import com.huawei.entity.vo.componentbuild.ComponentBuildExcelDto;
import com.huawei.entity.vo.componentbuild.ComponentBuildQo;
import com.huawei.entity.vo.componentbuild.TimeConsume;
import com.huawei.entity.vo.componentbuild.TimeConsumeContrastDto;
import com.huawei.entity.vo.componentbuild.TimeConsumeDto;
import com.huawei.enums.DateEnums;
import com.huawei.operation.*;
import com.huawei.service.cicd.ComponentBuildService;
import lombok.extern.slf4j.Slf4j;
import org.apache.commons.lang.StringUtils;
import org.apache.poi.ss.usermodel.Workbook;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.http.HttpStatus;
import org.springframework.scheduling.concurrent.ThreadPoolTaskExecutor;
import org.springframework.stereotype.Service;
import org.springframework.util.CollectionUtils;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.stream.Collectors;

import static com.huawei.enums.CodeCheckConstants.*;

@Slf4j
@Service
public class ComponentBuildServiceImpl implements ComponentBuildService {
    private static final String regex = "^(?:[1-9]\\d*|0)?(?:\\.\\d+)?$";

    private static final String[] Excel_title = {"触发", "触发", "触发", "触发", "触发", "触发", "触发", "触发", "触发", "触发", "触发"};

    @Autowired
    @Qualifier(value = "syncPipelineThreadPoolExecutor")
    private ThreadPoolTaskExecutor syncPipelineThreadPool;

    @Autowired
    private PipelineOperation pipelineOperation;

    @Autowired
    private ComponentBuildOperation componentBuildOperation;

    @Autowired
    private EventOperation eventOperation;

    @Autowired
    private EventCodeCheckOperation eventCodeCheckOperation;

    @Autowired
    private CustomParameterOperation customParameterOperation;

    @Autowired
    private IncResultSummaryOperation incResultSummaryOperation;

    @Autowired
    private CodeCheckDevOperation codeCheckDevOperation;

    @Autowired
    private ScheduleDelegateImpl scheduleDelegate;

    @Autowired
    private BundleCodeResultSummaryOperation bundleCodeResultSummaryOperation;

    @Override
    public Response saveComponentBuild() {
        // 1. 查询昨天所有的构建信息
        Calendar calendar = Calendar.getInstance();
        String today = DateEnums.DAY_NUM.getFormatTime(calendar.getTime());
        calendar.add(Calendar.DAY_OF_MONTH, -1);
        String yesterday = DateEnums.DAY_NUM.getFormatTime(calendar.getTime());
        // 内部仓信息
        CustomParameterVo customParameterByConfiguration =
                customParameterOperation.getCustomParameterByConfiguration(OHOS_REPO_CONFIG_KEY);
        List<String> reposForInner = (List<String>) customParameterByConfiguration.getParameters().get("gitUrls");
        List<PipelineVo> pipelineVos = pipelineOperation.listPipelineByTime(yesterday, today);
        pipelineVos.stream().forEach(pipelineVo -> {
            syncPipelineThreadPool.execute(() -> {
                try {
                    long l = componentBuildOperation.countByUuidAndComponent(pipelineVo.getUuid(), pipelineVo.getComponent());
                    if (l > 0) {
                        return;
                    }
                    ComponentBuild.ComponentBuildBuilder builder = ComponentBuild.builder();
                    builder.buildDate(yesterday);
                    buildByPipeline(builder, pipelineVo);
                    buildByEvent(builder, pipelineVo.getUuid(), reposForInner);
                    componentBuildOperation.insert(builder.build());
                } catch (Exception e) {
                    log.error("sync pipeline error  pipeline: {}", JsonUtils.toJson(pipelineVo));
                    e.printStackTrace();
                }
            });
        });
        return Response.result(HttpStatus.OK);
    }

    @Override
    public Response listComponentBuild(ComponentBuildQo componentBuildQo) {
        checkPage(componentBuildQo);
        List<ComponentBuild> componentBuilds = componentBuildOperation.listPageComponentBuildByQo(componentBuildQo);
        long total = componentBuildOperation.countByQo(componentBuildQo);
        HashMap<String, Object> result = new HashMap<>();
        result.put("total", total);
        result.put("data", componentBuilds);
        return Response.result(HttpStatus.OK, result);
    }

    @Override
    public Response<TimeConsumeDto> getTimeConsume(ComponentBuildQo componentBuildQo) {
        checkPage(componentBuildQo);
        List<TimeConsume> timeConsumes = componentBuildOperation.listTimeConsumeByQo(componentBuildQo);
        TimeConsume timeConsume = new TimeConsume();
        if (CollectionUtils.isEmpty(timeConsumes)) {
            return Response.result(HttpStatus.OK, new TimeConsumeDto());
        }
        Integer count = 0;
        for (TimeConsume consume : timeConsumes) {
            timeConsume.setEventDuration(consume.getEventDuration() + timeConsume.getEventDuration());
            timeConsume.setTestDuration(consume.getTestDuration() + timeConsume.getTestDuration());
            timeConsume.setBuildDuration(consume.getBuildDuration() + timeConsume.getBuildDuration());
            timeConsume.setInitDuration(consume.getInitDuration() + timeConsume.getInitDuration());
            timeConsume.setDownloadDuration(consume.getDownloadDuration() + timeConsume.getDownloadDuration());
            timeConsume.setFetchDuration(consume.getFetchDuration() + timeConsume.getFetchDuration());
            timeConsume.setGitLfsDuration(consume.getGitLfsDuration() + timeConsume.getGitLfsDuration());
            timeConsume.setPreCompileDuration(consume.getPreCompileDuration() + timeConsume.getPreCompileDuration());
            timeConsume.setMainCompileDuration(consume.getMainCompileDuration() + timeConsume.getMainCompileDuration());
            timeConsume.setAfterCompileDuration(consume.getAfterCompileDuration() +
                    timeConsume.getAfterCompileDuration());
            timeConsume.setArchiveDuration(consume.getArchiveDuration() + timeConsume.getArchiveDuration());
            timeConsume.setUploadDuration(consume.getUploadDuration() + timeConsume.getUploadDuration());
            double cacheHitRateNum = consume.getCacheHitRateNum();
            if (cacheHitRateNum > 0) {
                count++;
                timeConsume.setCacheHitRateNum(BigDecimalUtil.add(consume.getCacheHitRateNum(),
                        timeConsume.getCacheHitRateNum()));
            }
        }
        if (count != 0) {
            timeConsume.setCacheHitRateNum(BigDecimalUtil.divideHalfUp(timeConsume.getCacheHitRateNum(), count, 2));
        }
        setAverage(timeConsume, timeConsumes.size());
        return Response.result(HttpStatus.OK, new TimeConsumeDto(timeConsume));
    }

    private void setAverage(TimeConsume timeConsume, int size) {
        timeConsume.setEventDuration(timeConsume.getEventDuration() / size);
        timeConsume.setTestDuration(timeConsume.getTestDuration() / size);
        timeConsume.setBuildDuration(timeConsume.getBuildDuration() / size);
        timeConsume.setInitDuration(timeConsume.getInitDuration() / size);
        timeConsume.setDownloadDuration(timeConsume.getDownloadDuration() / size);
        timeConsume.setFetchDuration(timeConsume.getFetchDuration() / size);
        timeConsume.setGitLfsDuration(timeConsume.getGitLfsDuration() / size);
        timeConsume.setPreCompileDuration(timeConsume.getPreCompileDuration() / size);
        timeConsume.setMainCompileDuration(timeConsume.getMainCompileDuration() / size);
        timeConsume.setAfterCompileDuration(timeConsume.getAfterCompileDuration() / size);
        timeConsume.setArchiveDuration(timeConsume.getArchiveDuration() / size);
        timeConsume.setUploadDuration(timeConsume.getUploadDuration() / size);
    }

    @Override
    public Response getCacheHitRate(ComponentBuildQo componentBuildQo) {
        checkPage(componentBuildQo);
        List<CacheHitRate> cacheHitRateList = componentBuildOperation.listCacheHitRateByQo(componentBuildQo);
        cacheHitRateList.forEach(rate -> {
            double rateNum = rate.getCacheHitRateNum();
            if (rateNum >= 0 && rate.getCacheHitRateNum() <= 85) {
                rate.setHitPart(1);
            } else if (rateNum <= 90) {
                rate.setHitPart(2);
            } else if (rateNum <= 95) {
                rate.setHitPart(3);
            } else {
                rate.setHitPart(4);
            }
        });
        Map<Integer, CacheCount> result = new HashMap<>();
        CacheCount total = new CacheCount();
        total.setTotal(cacheHitRateList.size());
        Map<Integer, List<CacheHitRate>> groupByPart = cacheHitRateList.stream()
                .collect(Collectors.groupingBy(CacheHitRate::getHitPart));
        groupByPart.entrySet().stream().forEach(entry -> {
            CacheCount cacheCount = new CacheCount();
            cacheCount.setTotal(entry.getValue().size());
            Integer count = Math.toIntExact(entry.getValue().stream()
                    .filter(cacheHitRate -> cacheHitRate.getBuildDuration() >= 900).count());
            cacheCount.setFifteen(count);
            total.setFifteen(total.getFifteen() + count);
            result.put(entry.getKey(), cacheCount);
        });
        // 返回结果 key = 0 时为总结果
        result.put(0, total);
        for (int i = 0; i < 5; i++) {
            CacheCount cacheCount = result.get(i);
            if (cacheCount == null) {
                result.put(i, new CacheCount());
            }
        }
        return Response.result(HttpStatus.OK, result);
    }

    @Override
    public Response getContrast(ComponentBuildQo componentBuildQo) {
        TimeConsumeDto targetDay = getTimeConsume(componentBuildQo).getData();
        String timeOne = componentBuildQo.getTimeOne();
        SimpleDateFormat format = DateEnums.DAY_NUM.getFormat();
        try {
            Date parse = format.parse(timeOne);
            Calendar calendar = Calendar.getInstance();
            calendar.setTime(parse);
            calendar.add(Calendar.DAY_OF_MONTH, -1);
            componentBuildQo.setTimeOne(format.format(calendar.getTime()));
        } catch (ParseException e) {
            return Response.result(String.valueOf(HttpStatus.BAD_REQUEST.value()), "time format not incorrect");
        }
        TimeConsumeDto targetYesterday = getTimeConsume(componentBuildQo).getData();
        return Response.result(HttpStatus.OK, new TimeConsumeContrastDto(targetDay, targetYesterday));
    }

    @Override
    public byte[] exportComponentBuild(ComponentBuildQo componentBuildQo) {
        checkCondition(componentBuildQo);
        List<ComponentBuild> componentBuilds = componentBuildOperation.listComponentBuildData(componentBuildQo);
        List<ComponentBuildExcelDto> excelDtoList = componentBuilds.stream()
                .map(ComponentBuildExcelDto::new).collect(Collectors.toList());
        Workbook export;
        export = ExcelUtil.exportNew(excelDtoList);
        return ExcelUtil.workbookToBytes(export, "component builds");
    }

    private void checkPage(ComponentBuildQo componentBuildQo) {
        int pageNum = componentBuildQo.getPageNum();
        if (pageNum < 0) {
            componentBuildQo.setPageNum(0);
        }
        int pageSize = componentBuildQo.getPageSize();
        if (pageSize > 500) {
            componentBuildQo.setPageSize(500);
        }
        if (pageSize < 0) {
            componentBuildQo.setPageSize(0);
        }
        checkCondition(componentBuildQo);
    }

    private void checkCondition(ComponentBuildQo componentBuildQo) {
        String projectName = componentBuildQo.getProjectName();
        if (StringUtils.isBlank(projectName)) {
            componentBuildQo.setProjectName("openharmony");
        }
        String branch = componentBuildQo.getBranch();
        if (StringUtils.isBlank(branch)) {
            componentBuildQo.setBranch("master");
        }
        String timeOne = componentBuildQo.getTimeOne();
        if (StringUtils.isBlank(timeOne)) {
            Calendar instance = Calendar.getInstance();
            instance.add(Calendar.DAY_OF_MONTH, -1);
            componentBuildQo.setTimeOne(DateEnums.DAY_NUM.getFormatTime(instance.getTime()));
        }
    }

    private void buildByEvent(ComponentBuild.ComponentBuildBuilder builder, String uuid, List<String> reposForInner) {
        EventVo event = eventOperation.getEventByUuid(uuid);
        String codeCheckResult = dealWithFinalCodeCheckResult(event.getUuid(), reposForInner);
        builder.triggerUser(event.getTriggerUser())
                .timestamp(event.getTimestamp())
                .eventStatus(event.getCurrentStatus())
                .committer(getCommitters(event))
                .uuid(uuid)
                .codecheckResult(codeCheckResult);
        if (event.getDuration() != null) {
            builder.eventDuration(Math.toIntExact(Math.round(event.getDuration())));
        }
        EventModel ciEvent = eventCodeCheckOperation.getEventByUuid(uuid);
        if (!Objects.isNull(ciEvent)) {
            builder.ciUrl(ciEvent.getId());
        }
    }

    private String dealWithFinalCodeCheckResult(String uuid, List<String> reposForInner) {
        EventModel eventModel = eventCodeCheckOperation.getEventByUuid(uuid);
        if (Objects.isNull(eventModel)) {
            return PASS;
        }
        List<BundleCodeResultSummary> byUuid = new ArrayList<>();
        if (Objects.nonNull(eventModel)) {
            byUuid = bundleCodeResultSummaryOperation.getSummaryByUuid(uuid);
        }
        CodeCheckInfo codeCheckInfo = null;
        String resultSetForYCheck;
        String totalResultForBCheck;
        CodeCheckDevCloud devCloudByUuid = codeCheckDevOperation.getDevCloudByUuid(uuid);

        // 蓝
        HashSet<String> resultSetForBCheck = getAllBCodeCheckSet(uuid);
        if (Objects.nonNull(eventModel.getCodeCheckInfo())) {
            codeCheckInfo = JsonUtils.fromJson(JsonUtils.toJson(eventModel.getCodeCheckInfo()), CodeCheckInfo.class);
        }else {
            return PASS;
        }
        totalResultForBCheck = scheduleDelegate.doFilterForCommunityCheckResult(byUuid);
        // 黄
        if (Objects.nonNull(devCloudByUuid) && Objects.nonNull(codeCheckInfo)) {
            resultSetForYCheck =
                    scheduleDelegate.doFilterForInnerReposTotalResult(devCloudByUuid.getSubCodeCheckList(),
                            reposForInner, codeCheckInfo, byUuid, devCloudByUuid);
        } else {
            resultSetForYCheck = PASS;
        }
        // 未检查/未配置 默认通过
        if (Objects.nonNull(codeCheckInfo)) {
            return doJudgmentForCurrentIncEventAllPR(totalResultForBCheck, resultSetForYCheck);
        } else {
            return PASS;
        }
    }

    private String doJudgmentForCurrentIncEventAllPR(String totalResultForBCheck, String totalResultForYCheck) {
        return ((totalResultForBCheck.equals(PASS) && totalResultForYCheck.equals(PASS)) ? PASS : FAILED);
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

    private String getCommitters(EventVo event) {
        StringBuilder builder = new StringBuilder();
        event.getPrMsg().forEach(prMsgVo -> {
            if (StringUtils.isNotBlank(prMsgVo.getCommitter())) {
                builder.append(prMsgVo.getCommitter());
                builder.append(",");
            }
        });
        builder.deleteCharAt(builder.length() - 1);
        return builder.toString();
    }

    private void buildByPipeline(ComponentBuild.ComponentBuildBuilder builder, PipelineVo pipelineVo) {
        Map<String, Map<String, String>> stages = pipelineVo.getStages();
        String testDuration = pipelineVo.getTestDuration();
        String buildDuration = pipelineVo.getBuildDuration();
        builder.projectName(pipelineVo.getNamespace())
                .branch(pipelineVo.getManifestBranch())
                .component(pipelineVo.getComponent())
                .buildResult(pipelineVo.getBuildResult())
                .testResult(pipelineVo.getTestResult())
                .testDuration(getDurationByString(testDuration))
                .buildDuration(getDurationByString(buildDuration))
                .prUrl(pipelineVo.getPr())
                .compileCmd(pipelineVo.getCompileCmd())
                .preCompile(pipelineVo.getPreCompile())
                .afterCompile(pipelineVo.getAfterCompile());
        if (stages != null) {
            setStage(builder, stages);
            setCacheByStage(builder, stages);
        }
    }

    private int getDurationByString(String duration) {
        if (duration == null) {
            return 0;
        }
        if (duration.contains(".")) {
            String replace = duration.replace(".", "");
            return Integer.valueOf(replace) / 10;
        }
        return Integer.valueOf(duration);
    }

    private void setStage(ComponentBuild.ComponentBuildBuilder builder, Map<String, Map<String, String>> stages) {
        if (stages.get("init") != null) {
            builder.initStatus(stages.get("init").get("status"))
                    .initDuration(getDuration(stages.get("init").get("end"),
                            stages.get("init").get("start")));
        }
        if (stages.get("download_code") != null) {
            builder.downloadStatus(stages.get("download_code").get("status"))
                    .downloadDuration(getDuration(stages.get("download_code").get("end"),
                            stages.get("download_code").get("start")));
        }
        if (stages.get("fetch_pr") != null) {
            builder.fetchStatus(stages.get("fetch_pr").get("status"))
                    .fetchDuration(getDuration(stages.get("fetch_pr").get("end"),
                            stages.get("fetch_pr").get("start")));
        }
        if (stages.get("git_lfs") != null) {
            builder.gitLfsStatus(stages.get("git_lfs").get("status"))
                    .gitLfsDuration(getDuration(stages.get("git_lfs").get("end"),
                            stages.get("git_lfs").get("start")));
        }
        if (stages.get("pre_compile") != null) {
            builder.preCompileStatus(stages.get("pre_compile").get("status"))
                    .preCompileDuration(getDuration(stages.get("pre_compile").get("end"),
                            stages.get("pre_compile").get("start")));
        }
        if (stages.get("main_compile") != null) {
            builder.mainCompileStatus(stages.get("main_compile").get("status"))
                    .mainCompileDuration(getDuration(stages.get("main_compile").get("end"),
                            stages.get("main_compile").get("start")));
        }
        if (stages.get("after_compile") != null) {
            builder.afterCompileStatus(stages.get("after_compile").get("status"))
                    .afterCompileDuration(getDuration(stages.get("after_compile").get("end"),
                            stages.get("after_compile").get("start")));
        }
        if (stages.get("archive") != null) {
            builder.archiveStatus(stages.get("archive").get("status"))
                    .archiveDuration(getDuration(stages.get("archive").get("end"),
                            stages.get("archive").get("start")));
        }
        if (stages.get("upload") != null) {
            builder.uploadStatus(stages.get("upload").get("status"))
                    .uploadDuration(getDuration(stages.get("upload").get("end"),
                            stages.get("upload").get("start")));
        }
    }

    private void setCacheByStage(ComponentBuild.ComponentBuildBuilder builder,
                                 Map<String, Map<String, String>> stages) {
        Map<String, String> summary = stages.get("ccache_summary");
        if (Objects.isNull(summary)) {
            return;
        }
        Map<String, String> cacheMap = JsonUtils.fromJson(summary.get("status"), Map.class);
        String cacheHitRate = getFirst(cacheMap.getOrDefault("ccache hit rate", "0"));
        builder.cacheHitDirect(getFirst(cacheMap.getOrDefault("ccache hit (direct)", "0")))
                .cacheHitPreprocessed(getFirst(cacheMap.getOrDefault("ccache hit (preprocessed)", "0")))
                .cacheMiss(getFirst(cacheMap.getOrDefault("ccache miss", "0")))
                .cacheHitRate(cacheHitRate)
                .cacheMissRate(getFirst(cacheMap.getOrDefault("ccache mis rate", "0")));
        builder.cacheHitRateNum(getNumByRate(cacheHitRate));
    }

    private double getNumByRate(String cacheMisRate) {
        String replace = cacheMisRate.replace("%", "");
        if (!replace.matches(regex)) {
            return -1;
        }
        return Double.parseDouble(replace);
    }

    private String getFirst(String str) {
        if (!str.contains(" ")) {
            return str;
        }
        String[] s = str.split(" ");
        return s[0];
    }


    private Integer getDuration(String end, String start) {
        SimpleDateFormat format = new SimpleDateFormat(DateEnums.SECOND_ONE.getPattern());
        if (!end.matches(regex) || !start.matches(regex)) {
            return -1;
        }
        long big;
        try {
            Date parse = format.parse(end);
            big = parse.getTime() / 1000;
        } catch (ParseException e) {
            log.error("parse pipeline stage time error, timestamp:{}", end);
            return 0;
        }
        long small;
        try {
            Date parse = format.parse(start);
            small = parse.getTime() / 1000;
        } catch (ParseException e) {
            log.error("parse pipeline stage time error, timestamp:{}", start);
            return 0;
        }
        return Math.toIntExact(big - small);
    }

    public static void main(String[] args) {
        SimpleDateFormat format = DateEnums.SECOND_ONE.getFormat();
        long big;
        try {
            Date parse = format.parse("20230227155752");
            big = parse.getTime() / 1000;
        } catch (ParseException e) {
            log.error("parse pipeline stage time error, timestamp:{}", "end");
            return;
        }
        long small;
        try {
            Date parse = format.parse("20230227154414");
            small = parse.getTime() / 1000;
        } catch (ParseException e) {
            log.error("parse pipeline stage time error, timestamp:{}", "start");
            return;
        }
        int i = 0;
        try {
            i = Math.toIntExact(big - small);
        } catch (Exception e) {
            log.error("integer overflow : {} {}", "end", "start");
        }
        log.info("i {} ", i);
    }
}
