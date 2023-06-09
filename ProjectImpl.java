package com.huawei.impl;

import com.huawei.ci.common.entity.cicd.CiComponent;
import com.huawei.ci.common.entity.cicd.CiProjectQuery;
import com.huawei.ci.common.entity.cicd.CiProjectVo;
import com.huawei.ci.common.gitee.GiteeUtils;
import com.huawei.ci.common.pojo.vo.XmlPathVo;
import com.huawei.entity.vo.cicd.CiBranch;
import com.huawei.entity.vo.cicd.CiBuild;
import com.huawei.entity.vo.cicd.CiTitle;
import com.huawei.entity.vo.codecheck.project.ProjectInfoVo;
import com.huawei.enums.CodeCheckConstants;
import com.huawei.operation.CiConfigOperation;
import com.huawei.operation.ProjectInfoOperation;
import com.huawei.service.ProjectService;
import org.apache.commons.collections4.CollectionUtils;
import org.apache.commons.lang.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Collectors;

@Component
public class ProjectImpl implements ProjectService {
    @Autowired
    private ProjectInfoOperation projectInfoOperation;

    @Autowired
    private CiConfigOperation ciConfigOperation;

    @Autowired
    private GiteeUtils giteeUtils;

    public ArrayList<CiTitle> getCiTitleList(String loginName, String giteeToken) {
        boolean isLogout = StringUtils.isBlank(loginName) || StringUtils.isBlank(giteeToken);

        List<ProjectInfoVo> projectInfoVos = projectInfoOperation.getAll();
        List<CiProjectVo> publicProject = new ArrayList<>();
        List<CiProjectVo> privateProject = new ArrayList<>();
        projectInfoVos.forEach(projectInfoVo -> {
            if (isLogout && !projectInfoVo.isPublic()) {
                return;
            }
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
        // 2.2 没登陆 只返回开源组织
        // 2.获取组织列表 做权限校验  结果放入resultMap中
        if (isLogout) {
            return ciProjectVoToCiTitle(publicProject);
        }
        // 2.3 登录后 过滤没权限的项目
        List<String> orgList = getOrgList(loginName, giteeToken);
        if (CollectionUtils.isNotEmpty(orgList)) {
            privateProject.forEach(ciProjectVo -> {
                if (orgList.contains(ciProjectVo.getNameSpace())) {
                    publicProject.add(ciProjectVo);
                }
            });
        }

        return ciProjectVoToCiTitle(publicProject);
    }

    private ArrayList<CiTitle> ciProjectVoToCiTitle(List<CiProjectVo> publicProject) {
        Map<String, List<CiProjectVo>> nameSpaceToCiProject =
                publicProject.stream().collect(Collectors.groupingBy(CiProjectVo::getNameSpace));
        List<String> nameSpaces = publicProject.stream().map(CiProjectVo::getNameSpace).distinct().collect(Collectors.toList());
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
                if (CollectionUtils.isNotEmpty(componentList)){
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
                        Map<String, String> pipelineConfig = ciComponent.getPipelineConfig();
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

    private List<String> getOrgList(String loginName, String giteeToken) {
        int page = 1;
        int limit = 100;
        int size = 0;
        List<String> orgList = new ArrayList<>();
        do {
            List<Map<String, String>> userOrgs = giteeUtils.getUserOrgs(loginName, giteeToken, page, limit);
            if (CollectionUtils.isEmpty(userOrgs)) {
                break;
            }
            List<String> temp = userOrgs.stream()
                    .map(project -> project.get("login")).collect(Collectors.toList());

            orgList.addAll(temp);
            size = temp.size();
            page++;
        } while (size == limit);
        return orgList;
    }
}
