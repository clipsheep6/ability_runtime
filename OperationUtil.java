package com.huawei.utils;

import org.apache.commons.collections4.CollectionUtils;
import org.springframework.data.domain.Sort;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.aggregation.Aggregation;
import org.springframework.data.mongodb.core.aggregation.AggregationOperation;
import org.springframework.data.mongodb.core.query.Criteria;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * 数据库操作工具类
 *
 * @since 2023/2/13
 * @version 1.0
 */
public class OperationUtil {
    /**
     * 获取某个字段的值的字符串集合
     *
     * @param field          字段
     * @param criteria       查询条件
     * @param mongoTemplate  数据库配置类
     * @param collectionName 数据库表名
     * @return               值的集合
     */
    public static List<String> getFieldList(String field, Criteria criteria, MongoTemplate mongoTemplate, String collectionName) {
        List<AggregationOperation> operations = new ArrayList<>();
        operations.add(Aggregation.match(criteria));
        operations.add(Aggregation.group(field));
        operations.add(Aggregation.sort(Sort.by(Sort.Order.desc("_id"))));
        List<Map> mappedResults = mongoTemplate.aggregate(Aggregation.newAggregation(operations),
                collectionName, Map.class).getMappedResults();
        if (CollectionUtils.isNotEmpty(mappedResults)) {
            return mappedResults.stream().map(map -> map.get("_id").toString()).collect(Collectors.toList());
        }

        return new ArrayList<>();
    }
}
