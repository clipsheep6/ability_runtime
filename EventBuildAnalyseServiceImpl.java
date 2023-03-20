package com.huawei.impl;

import com.huawei.ci.common.contant.ResponseCode;
import com.huawei.ci.common.pojo.vo.Response;
import com.huawei.entity.vo.ciinfo.event.QueryStageModel;
import com.huawei.entity.vo.eventbuild.BuildStageVo;
import com.huawei.operation.EventBuildAnalyseOperation;
import com.huawei.service.EventBuildAnalyseService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.math.BigDecimal;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * @Description: EventBuildAnalyseServiceImpl
 * @ClassName: com.huawei.impl
 * @since : 2022/11/11  11:31
 **/
@Component
public class EventBuildAnalyseServiceImpl implements EventBuildAnalyseService {
    @Autowired
    private EventBuildAnalyseOperation eventBuildAnalyseOperation;

    /**
     * 门禁构建分析（列表）
     *s
     * @param queryStageModel queryStageModel
     * @return Response
     */
    @Override
    public Response getEventBuildStage(QueryStageModel queryStageModel) {
        List<BuildStageVo> buildStageVos = eventBuildAnalyseOperation.countBuildStageAvg(queryStageModel);
        buildStageVos.forEach(buildStageVo -> {
            BigDecimal avgDuration =
                    buildStageVo.getInit().add(buildStageVo.getDownloadCode()).add(buildStageVo.getFetchPr())
                            .add(buildStageVo.getGitLfs()).add(buildStageVo.getPreCompile()).add(buildStageVo.getMainCompile())
                            .add(buildStageVo.getAfterCompile()).add(buildStageVo.getArchive()).add(buildStageVo.getUpload());
            buildStageVo.setAvgDuration(avgDuration);
        });
        List<BuildStageVo> countBuildStageSum = eventBuildAnalyseOperation.countBuildStageSum(queryStageModel);
        Map<String, Integer> sumDuration = new HashMap<>();
        if (!countBuildStageSum.isEmpty()) {
            sumDuration.put("init", countBuildStageSum.get(0).getInit().intValue());
            sumDuration.put("downloadCode", countBuildStageSum.get(0).getDownloadCode().intValue());
            sumDuration.put("fetchPr", countBuildStageSum.get(0).getFetchPr().intValue());
            sumDuration.put("gitLfs", countBuildStageSum.get(0).getGitLfs().intValue());
            sumDuration.put("preCompile", countBuildStageSum.get(0).getPreCompile().intValue());
            sumDuration.put("mainCompile", countBuildStageSum.get(0).getMainCompile().intValue());
            sumDuration.put("afterCompile", countBuildStageSum.get(0).getAfterCompile().intValue());
            sumDuration.put("archive", countBuildStageSum.get(0).getArchive().intValue());
            sumDuration.put("upload", countBuildStageSum.get(0).getUpload().intValue());
        }
        Map<String, Object> result = new HashMap<>();
        result.put("list", buildStageVos);
        result.put("sumDuration", sumDuration);
        return Response.result(ResponseCode.SUCCESS, result);
    }
}
