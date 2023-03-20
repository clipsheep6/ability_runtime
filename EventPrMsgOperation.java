package com.huawei.operation;

import com.huawei.entity.vo.ciinfo.event.EventVo;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * @Description: 获取ci_info 数据库中 码云eventvo关联的pr信息
 * @ClassName: com.huawei.operation
 * @since : 2022/10/31  10:09
 **/
@Component
public class EventPrMsgOperation {

    @Autowired
    @Qualifier("ciWriteMongoTemplate")
    private MongoTemplate mongoTemplate;


    /**
     * 根据uuid来获取对应的合入请求vo  测试接口，后续删除
     *
     * @param uuid uuid
     * @return event {@linkplain EventVo}
     */
    public EventVo getEventVoByUUid(String uuid) {
        Criteria criteria = Criteria.where("uuid").is(uuid);
        Query query = Query.query(criteria);
        EventVo event = mongoTemplate.findOne(query, EventVo.class, "event");
        return event;
    }

    /**
     * 获取所有的合入请求实体vo
     *
     * @return List<EventVo> {@linkplain EventVo}
     */
    public List<EventVo> getAllEventSummary() {
        List<EventVo> eventVoList = mongoTemplate.findAll(EventVo.class, "event");
        return eventVoList;
    }
}
