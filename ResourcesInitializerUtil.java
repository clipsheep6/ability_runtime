package com.huawei.utils;

import com.huawei.enums.ResourceConstant;
import lombok.extern.slf4j.Slf4j;
import org.dom4j.Attribute;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;

import java.io.IOException;
import java.io.InputStreamReader;
import java.util.*;

/**
 * @Description 资源初始化操作类
 * @ClassName com.huawei.utils
 * @since 2022/11/3  11:02
 **/
@Slf4j
@Component
public class ResourcesInitializerUtil {

    private static final Logger logger = LoggerFactory.getLogger(ResourcesInitializerUtil.class);

    private static String encoding = ResourceConstant.ENCODING;

    private static String ohosInnerRepo = ResourceConstant.OHOS_REPO;


    private static String fileInnerRepos = String.valueOf(ResourceConstant.FILE_PATH_INNER_REPOS);


    /**
     * 读取本地配置文件获取内部codecheck仓
     * <p>
     * 数据来源 resources\InnerRepos\ohos_gitee_mirror.xml
     */
    public LinkedList<String> getInnerReposFromNative() {
        LinkedList<String> linkedListRepos = new LinkedList<>();
        try {
            SAXReader saxReader = new SAXReader();
            InputStreamReader inputStreamReader = new InputStreamReader(Objects.requireNonNull(ResourcesInitializerUtil.class.getClassLoader()
                    .getResourceAsStream(fileInnerRepos)), encoding);
            Document read = saxReader.read(inputStreamReader);
            Element rootElement = read.getRootElement();
            List<Element> elements = rootElement.elements();
            elements.stream().forEach(currentEle -> {
                List<Attribute> attributes = currentEle.attributes();
                attributes.forEach(attribute -> {
                    if (!attribute.getValue().contains("/") && !attribute.getValue().contains(",")
                            && !attribute.getValue().contains(":") && !attribute.getValue().contains(".")
                            && !attribute.getValue().contains("origin") && !attribute.getValue().contains("master")
                            && !attribute.getValue().contains("4") && !attribute.getValue().contains("1")
                            && !attribute.getValue().contains("Gitee_Mirror")
                    ) {
                        linkedListRepos.add(attribute.getValue());
                    }
                });
            });
        } catch (IOException | DocumentException e) {
            logger.error("init ohos_gitee_mirror.xml repos Info failed, caused by : {}", e.toString());
            return new LinkedList<>();
        }
        return linkedListRepos;
    }


    /**
     * 获取黄区需要进行codecheck内部检查的所有的仓库信息
     * <p>
     * 数据来源: https://gitee.com/openharmony/manifest/blob/master/ohos/ohos.xml 仓库均需要过滤
     */
    public LinkedList<String> getInnerRepoFromGitee() {
        // 创建dom4j解析器
        SAXReader reader = new SAXReader();
        LinkedList<String> linkedList = new LinkedList<>();
        try {
            Document read = reader.read(ohosInnerRepo);
            Element rootElement = read.getRootElement();
            List<Element> elements = rootElement.elements();
            elements.stream().forEach(currentEle -> {
                List<Attribute> attributes = currentEle.attributes();
                for (Attribute attribute : attributes) {
                    if (!attribute.getValue().contains("/") && !attribute.getValue().contains(",")
                            && !attribute.getValue().contains(":") && !attribute.getValue().contains(".")
                            && !attribute.getValue().contains("origin") && !attribute.getValue().contains("master")
                            && !attribute.getValue().contains("4") && !attribute.getValue().contains("1")
                            && !attribute.getValue().contains("docs")
                            && !attribute.getValue().contains("kernel_linux_5.10") && !attribute.getValue().contains("kernel_linux_4.19")
                    ) {
                        linkedList.add(attribute.getValue());
                    }
                }
            });
        } catch (DocumentException e) {
            logger.error("Error : init ohos.xml Info failed, caused by :" + e);
            // 兼容码云服务器出现异常 无法响应
            return new LinkedList<>();
        }
        return linkedList;
    }

}
