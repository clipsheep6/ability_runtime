package com.huawei.operation;

import com.huawei.ci.common.gitee.GiteeUtils;
import com.huawei.entity.vo.codecheck.GiteeRepoPermission.GiteeRepoPermissionModel;
import com.huawei.enums.CodeCheckCollectionName;
import com.netflix.config.DynamicPropertyFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.springframework.stereotype.Component;
import org.springframework.util.StringUtils;

import java.util.List;

@Component
public class GiteePermissionOperation {

    private final static String USER_URL = "https://gitee.com/api/v5/users/";
    private final String accessToken =
            DynamicPropertyFactory.getInstance().getStringProperty("gitee.access_token", "").get();
    @Autowired
    @Qualifier("codeCheckMongoTemplate")
    private MongoTemplate mongoTemplate;

    @Autowired
    GiteeUtils giteeUtils;

    public boolean checkUser(String userId, String projectId, String repoName, boolean isManage) {
        if (StringUtils.isEmpty(userId)) {
            return false;
        } else {
            //允许其他角色可以改所有仓状态
            Query query = Query.query(Criteria.where("userId").is(userId));
            List<GiteeRepoPermissionModel> giteeReposList = mongoTemplate.find(query,
                    GiteeRepoPermissionModel.class,
                    CodeCheckCollectionName.GITEE_REPOS);
            if (giteeReposList.size() > 0) {
                for (GiteeRepoPermissionModel gitr : giteeReposList) {
                    if (gitr.getType().equals("administrator") || gitr.getType().equals("codeIssueManager")) {
                        return true;
                    }
                }
            }
        }
        if (repoName != null) {
            String fullName;
            if (isManage) {
                fullName = repoName;
            } else {
                fullName = projectId + "/" + repoName;
            }
            Query query = Query.query(Criteria.where("userId").is(userId).and("repoFullName").is(fullName));
            List<GiteeRepoPermissionModel> giteeReposList = mongoTemplate.find(query,
                    GiteeRepoPermissionModel.class,
                    CodeCheckCollectionName.GITEE_REPOS);
            if (giteeReposList.size() > 0) {
                for (GiteeRepoPermissionModel gitr : giteeReposList) {
                    if (gitr.getType().equals("committer")) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
}
