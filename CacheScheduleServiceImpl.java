package com.huawei.impl;

import com.huawei.ci.common.entity.cicd.CiComponent;
import com.huawei.ci.common.entity.cicd.CiProjectQuery;
import com.huawei.ci.common.entity.cicd.CiProjectVo;
import com.huawei.ci.common.pojo.vo.XmlPathVo;
import com.huawei.entity.vo.cicd.CiBranch;
import com.huawei.entity.vo.cicd.CiBuild;
import com.huawei.entity.vo.cicd.CiTitle;
import com.huawei.entity.vo.codecheck.eventModule.CustomParameterVo;
import com.huawei.entity.vo.codecheck.project.ProjectInfoVo;
import com.huawei.enums.CiConstants;
import com.huawei.enums.CodeCheckConstants;
import com.huawei.enums.ConsoleConstants;
import com.huawei.enums.RedisConstant;
import com.huawei.module.codecheckoverview.CodeCheckOverViewModule;
import com.huawei.operation.*;
import com.huawei.service.schedule.CacheScheduleServices;
import com.huawei.utils.RedisOperateUtils;
import lombok.extern.slf4j.Slf4j;
import org.apache.commons.collections4.CollectionUtils;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.scheduling.concurrent.ThreadPoolTaskExecutor;
import org.springframework.stereotype.Component;

import java.util.*;
import java.util.stream.Collectors;

/**
 * @description 初始化
 * @classname com.huawei.impl
 * @since 2023-03-28 17:18
 **/
@Slf4j
@Component
public class CacheScheduleServiceImpl implements CacheScheduleServices {

    @Value("${server.environment}")
    private String environment;

    @Autowired
    private ProjectInfoOperation projectInfoOperation;

    @Autowired
    private CiConfigOperation ciConfigOperation;

    @Autowired
    private EventOperation eventOperation;

    @Autowired
    private CodeCheckOverViewImpl codeCheckOverViewImpl;

    @Autowired
    private RedisOperateUtils redisOperateUtils;

    @Autowired
    private CodeCheckBuildOperation codeCheckBuildOperation;

    @Autowired
    private CustomParameterOperation customParameterOperation;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private PipelineOperation pipelineOperation;

    @Autowired
    private TestBoardOperation testBoardOperation;

    @Autowired
    @Qualifier(value = "initRedisCacheThreadPools")
    private ThreadPoolTaskExecutor initRedisCacheThreadPool;


    @Override
    public void initCodeCheckOverViewCache() {
        CustomParameterVo othersComponentConfiguration = customParameterOperation.getCustomParameterByConfiguration
                (ConsoleConstants.TEST_BOARD_COMPONENT);
        ArrayList<CiTitle> allProjectAndBranch = getCiPlatformAllProjectAndBranch();
        for (CiTitle projectAndBranch : allProjectAndBranch) {
            String projectName = projectAndBranch.getProjectName();
            for (CiBranch branches : projectAndBranch.getBranchs()) {
                String branch = branches.getBranch();
                // environment-project-branch-key
                CodeCheckOverViewModule codeCheckOverViewModule = new CodeCheckOverViewModule();
                codeCheckOverViewModule.setProjectName(projectName);
                codeCheckOverViewModule.setBranch(branch);
                CiProjectQuery ciProjectQuery = new CiProjectQuery();
                ciProjectQuery.setNameSpace(codeCheckOverViewModule.getProjectName());
                ciProjectQuery.setManifestBranch(codeCheckOverViewModule.getBranch());
                List<String> tddAndXts = getTddAndXts(codeCheckOverViewModule, othersComponentConfiguration);
                boolean deleteCodeCheckMark = redisOperateUtils.delete(environment + "-" + projectName + "-" + branch
                        + "-" + RedisConstant.OVER_VIEW_KEY_CODE_CHECK_COUNT);
                boolean deleteDailyBuildMark = redisOperateUtils.delete(environment + "-" + projectName + "-" + branch
                        + "-" + RedisConstant.OVER_VIEW_DAILY_BUILD_COUNT);
                if (deleteCodeCheckMark || deleteDailyBuildMark) {
                    redisOperateUtils.delete(environment + "-" + projectName + "-" + branch
                            + "-" + RedisConstant.OVER_VIEW_KEY_CODE_CHECK_COUNT);
                    redisOperateUtils.delete(environment + "-" + projectName + "-" + branch
                            + "-" + RedisConstant.OVER_VIEW_DAILY_BUILD_COUNT);
                    codeCheckBuildOperation.getBuildsByComponent(tddAndXts, ciProjectQuery);
                    eventOperation.getCriteriaByCondition(codeCheckOverViewModule);
                }
            }
        }
    }

    /**
     * 初始化门禁稳定性数据 以及 问题数据
     */
    @Override
    public void initCodeCheckOverViewStable() {
        ArrayList<CiTitle> allProjectAndBranch = getCiPlatformAllProjectAndBranch();
        for (CiTitle projectAndBranch : allProjectAndBranch) {
            String projectName = projectAndBranch.getProjectName();
            for (CiBranch branches : projectAndBranch.getBranchs()) {
                String branch = branches.getBranch();
                CodeCheckOverViewModule codeCheckOverViewModule = new CodeCheckOverViewModule();
                codeCheckOverViewModule.setProjectName(projectName);
                codeCheckOverViewModule.setBranch(branch);
                // environment-project-branch-key
                boolean deleteCacheMark = redisOperateUtils.delete(environment + "-" + projectName + "-" + branch
                        + "-" + RedisConstant.OVER_VIEW_KEY_CODE_CHECK_STABLE);
                if (deleteCacheMark) {
                    pipelineOperation.getPipelineByOverViewCondition(codeCheckOverViewModule);
                }
            }
        }
    }

    /**
     * 初始化项目分支的XTS FUZZ TDD相关西信息
     */
    @Override
    public void initCodeCheckOverViewDailyBuild() {
        CustomParameterVo othersComponentConfiguration = customParameterOperation.getCustomParameterByConfiguration
                (ConsoleConstants.TEST_BOARD_COMPONENT);
        ArrayList<CiTitle> allProjectAndBranch = getCiPlatformAllProjectAndBranch();
        for (CiTitle projectAndBranch : allProjectAndBranch) {
            String projectName = projectAndBranch.getProjectName();
            for (CiBranch branches : projectAndBranch.getBranchs()) {
                String branch = branches.getBranch();
                CodeCheckOverViewModule codeCheckOverViewModule = new CodeCheckOverViewModule();
                codeCheckOverViewModule.setProjectName(projectName);
                codeCheckOverViewModule.setBranch(branch);
                // 当前项目-分支 下的 TDD XTS FUZZ 的配置信息
                HashMap<String, List<String>> tddAndXtsAndFuzz = getTddAndXtsAndFuzz(codeCheckOverViewModule, othersComponentConfiguration);
                StringBuilder tdd = new StringBuilder(this.environment + "-" + projectName + "-" + branch + "-"
                        + CiConstants.COMPONENT_TYPE_TDD + "-");
                tdd.append(RedisConstant.OVER_VIEW_DAILY_BUILD_TDD_XTS_FUZZ);
                StringBuilder xts = new StringBuilder(this.environment + "-" + projectName + "-" + branch + "-"
                        + CiConstants.COMPONENT_TYPE_XTS + "-");
                xts.append(RedisConstant.OVER_VIEW_DAILY_BUILD_TDD_XTS_FUZZ);
                StringBuilder fuzz = new StringBuilder(this.environment + "-" + projectName + "-" + branch + "-"
                        + CiConstants.COMPONENT_TYPE_FUZZ + "-");
                fuzz.append(RedisConstant.OVER_VIEW_DAILY_BUILD_TDD_XTS_FUZZ);
                String redisKeyTdd = String.valueOf(tdd);
                String redisKeyXts = String.valueOf(xts);
                String redisKeyFuzz = String.valueOf(fuzz);
                redisOperateUtils.delete(redisKeyTdd);
                redisOperateUtils.delete(redisKeyXts);
                redisOperateUtils.delete(redisKeyFuzz);
                // item and success rate
                redisOperateUtils.delete(environment + "-" + projectName + "-" + branch + "-" +
                        RedisConstant.OVER_VIEW_DAILY_BUILD_SUCCESS_RATE);
                redisOperateUtils.delete(environment + "-" + projectName + "-" + branch + "-" +
                        RedisConstant.OVER_VIEW_DAILY_BUILD_ITEM);
                ArrayList<String> strings = new ArrayList<>();
                List<String> tddList = tddAndXtsAndFuzz.get("tddComponent");
                List<String> xtsList = tddAndXtsAndFuzz.get("xtsComponent");
                List<String> fuzzList = tddAndXtsAndFuzz.get("fuzzComponent");
                strings.addAll(tddList);
                strings.addAll(xtsList);
                strings.addAll(fuzzList);
                testBoardOperation.getTestComponentByComponentAll(codeCheckOverViewModule,
                        CiConstants.COMPONENT_TYPE_TDD, tddList, xtsList, fuzzList);
                testBoardOperation.getTestComponentByComponentAll(codeCheckOverViewModule,
                        CiConstants.COMPONENT_TYPE_XTS, tddList, xtsList, fuzzList);
                testBoardOperation.getTestComponentByComponentAll(codeCheckOverViewModule,
                        CiConstants.COMPONENT_TYPE_FUZZ, tddList, xtsList, fuzzList);
                // item and success rate
                codeCheckBuildOperation.getPipelinesByBuildItem(codeCheckOverViewModule, strings);
                codeCheckBuildOperation.getDailyPipeLineBuildSuccessRate(codeCheckOverViewModule);
            }
        }
    }

    /**
     * 获取所有的XTS TDD FUZZ 组件信息
     *
     * @param codeCheckOverViewModule      项目入参信息
     * @param othersComponentConfiguration 其他的配置信息
     * @return HashMap<String, List < String>>
     */
    public HashMap<String, List<String>> getTddAndXtsAndFuzz(CodeCheckOverViewModule codeCheckOverViewModule,
                                                             CustomParameterVo othersComponentConfiguration) {
        List<CiProjectVo> ciProjectList = new ArrayList<>();
        HashMap<String, List<String>> componentMap = new HashMap<>();
        List<String> tddComponent = new ArrayList<>();
        List<String> xtsComponent = new ArrayList<>();
        List<String> fuzzComponent = new ArrayList<>();
        // component
        CiProjectQuery ciProjectQuery = new CiProjectQuery();
        ciProjectQuery.setNameSpace(codeCheckOverViewModule.getProjectName());
        ciProjectQuery.setManifestBranch(codeCheckOverViewModule.getBranch());
        ciProjectList.addAll(ciConfigOperation.getProjectVo(ciProjectQuery));
        // tdd
        ciProjectList.forEach(ciProjectVo -> {
            List<Map<String, Object>> dailyPipelineList = ciProjectVo.getDailyPipelineList();
            if (!org.springframework.util.CollectionUtils.isEmpty(dailyPipelineList)) {
                tddComponent.addAll(codeCheckOverViewImpl.getTddComponent(dailyPipelineList));
            }
        });
        // fuzz
        Object fuzzObject = othersComponentConfiguration.getParameters().get(ConsoleConstants.FUZZ);
        if (Objects.nonNull(fuzzObject) && fuzzObject instanceof String) {
            fuzzComponent.add(fuzzObject.toString());
        } else if (Objects.nonNull(fuzzObject) && fuzzObject instanceof List) {
            fuzzComponent.addAll((List) fuzzObject);
        }

        // xts
        Object xtsObject = othersComponentConfiguration.getParameters().get(ConsoleConstants.XTS);
        if (Objects.nonNull(xtsObject) && xtsObject instanceof String) {
            xtsComponent.add(xtsObject.toString());
        } else if (Objects.nonNull(xtsObject) && xtsObject instanceof List) {
            xtsComponent.addAll((List) xtsObject);
        }
        componentMap.put("tddComponent", tddComponent);
        componentMap.put("xtsComponent", xtsComponent);
        componentMap.put("fuzzComponent", fuzzComponent);
        return componentMap;
    }

    private List<String> getTddAndXts(CodeCheckOverViewModule codeCheckOverViewModule,
                                      CustomParameterVo othersComponentConfiguration) {
        List<CiProjectVo> ciProjectList = new ArrayList<>();
        List<String> tddComponent = new ArrayList<>();
        CiProjectQuery ciProjectQuery = new CiProjectQuery();
        ciProjectQuery.setNameSpace(codeCheckOverViewModule.getProjectName());
        ciProjectQuery.setManifestBranch(codeCheckOverViewModule.getBranch());
        ciProjectList.addAll(ciConfigOperation.getProjectVo(ciProjectQuery));
        ciProjectList.forEach(ciProjectVo -> {
            List<Map<String, Object>> dailyPipelineList = ciProjectVo.getDailyPipelineList();
            if (!org.springframework.util.CollectionUtils.isEmpty(dailyPipelineList)) {
                tddComponent.addAll(codeCheckOverViewImpl.getTddComponent(dailyPipelineList));
            }
        });
        Object xtsObject = othersComponentConfiguration.getParameters().get(ConsoleConstants.XTS);
        List<String> xts = new ArrayList<>();
        if (Objects.nonNull(xtsObject) && xtsObject instanceof String) {
            tddComponent.add(xtsObject.toString());
            xts.add(xtsObject.toString());
        } else if (Objects.nonNull(xtsObject) && xtsObject instanceof List) {
            tddComponent.addAll((List) xtsObject);
            xts.addAll((List) xtsObject);
        }
        return tddComponent;
    }

    private ArrayList<CiTitle> getCiPlatformAllProjectAndBranch() {
        List<ProjectInfoVo> projectInfoVos = projectInfoOperation.getAll();
        List<CiProjectVo> publicProject = new ArrayList<>();
        List<CiProjectVo> privateProject = new ArrayList<>();
        projectInfoVos.forEach(projectInfoVo -> {
            List<XmlPathVo> xmlList = projectInfoVo.getXmlList();
            List<String> branches = xmlList.stream().map(XmlPathVo::getBranch).distinct().collect(Collectors.toList());
            CiProjectQuery ciProjectQuery = new CiProjectQuery();
            ciProjectQuery.setTopic(CodeCheckConstants.CI_CONF);
            ciProjectQuery.setNameSpace(projectInfoVo.getProjectName());
            List<CiProjectVo> projectVos = ciConfigOperation.getVoWithOutPipeline(ciProjectQuery);
            if (CollectionUtils.isEmpty(projectVos)) {
                return;
            }
            List<CiProjectVo> temp = projectVos.stream().filter(ciProjectVo ->
                    branches.contains(ciProjectVo.getManifestBranch())
            ).collect(Collectors.toList());
            boolean isPublic = projectInfoVo.isPublic();
            if (isPublic) {
                publicProject.addAll(temp);
            } else {
                privateProject.addAll(temp);
            }
        });
        return ciProjectVoToCiTitle(publicProject);
    }

    //  project and branch
    private ArrayList<CiTitle> ciProjectVoToCiTitle(List<CiProjectVo> publicProject) {
        Map<String, List<CiProjectVo>> nameSpaceToCiProject =
                publicProject.stream().collect(Collectors.groupingBy(CiProjectVo::getNameSpace));
        List<String> nameSpaces = publicProject.stream().map(CiProjectVo::getNameSpace).distinct()
                .collect(Collectors.toList());
        ArrayList<CiTitle> ciTitles = new ArrayList<>();
        nameSpaces.forEach(nameSpace -> {
            CiTitle ciTitle = new CiTitle();
            ciTitle.setProjectName(nameSpace);
            ciTitle.setBranchs(new ArrayList<>());
            ciTitles.add(ciTitle);
        });
        ciTitles.forEach(ciTitle -> {
            List<CiProjectVo> ciProjectVos = nameSpaceToCiProject.get(ciTitle.getProjectName());
            ciProjectVos.forEach(ciProjectVo -> {
                CiBranch ciBranch = new CiBranch();
                ciBranch.setBranch(ciProjectVo.getManifestBranch());
                ciBranch.setCodeBranch(ciProjectVo.getCodeBranchList());
                ciBranch.setBuilds(new ArrayList<>());
                List<CiComponent> componentList = ciProjectVo.getComponentList();
                if (CollectionUtils.isNotEmpty(componentList)) {
                    for (CiComponent ciComponent : componentList) {
                        CiBuild ciBuild = new CiBuild();
                        String name = ciComponent.getName();
                        if ("codecheck".equalsIgnoreCase(name)) {
                            continue;
                        }
                        ciBuild.setDeviceType(name);
                        ciBuild.setBuildTargets(name);
                        List<String> steps = new ArrayList<>();
                        steps.add("build");
                        List<String> stepsAlias = new ArrayList<>();
                        stepsAlias.add("debug");
                        Map<String, Object> pipelineConfig = ciComponent.getPipelineConfig();
                        if (Objects.nonNull(pipelineConfig)
                                && Objects.nonNull(pipelineConfig.get("hasTest"))
                                && "true".equals(pipelineConfig.get("hasTest"))) {
                            steps.add("test");
                            stepsAlias.add("test");
                        }
                        ciBuild.setBuildSteps(steps);
                        ciBuild.setBuildStepsAlias(stepsAlias);
                        ciBranch.getBuilds().add(ciBuild);
                    }
                }
                ciTitle.getBranchs().add(ciBranch);
            });
        });
        return ciTitles;
    }
}
