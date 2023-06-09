package com.huawei.impl;

import com.huawei.entity.vo.ciinfo.event.EventVo;
import com.huawei.operation.EventPrMsgOperation;
import com.huawei.service.EventPrMsgSyncService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * @Description: 同步合入请求关联的pr信息
 * @ClassName: com.huawei.impl
 * @since : 2022/10/31  10:05
 **/
@Component
public class EventPrMsgSyncDelegateImpl implements EventPrMsgSyncService {
    @Autowired
    private EventPrMsgOperation eventPrMsgOperation;

    /**
     * 获取某个条件下关联的所有的合入请求的相关信息
     *
     * @param
     * @return EventVo {@linkplain EventVo}
     */
    @Override
    public EventVo doSyncForPrMsgInfo() {
        String uuid = "05050e522dfa00f301ae41538b5302d3";
        EventVo eventVoByUUid = eventPrMsgOperation.getEventVoByUUid(uuid);
        return null;
    }

    /**
     * 获取某个条件下码云关联的所有的合入请求的相关信息
     *
     * @param
     * @return List<EventVo> {@linkplain EventVo}
     */
    @Override
    public List<EventVo> doGetAllEventList() {
        List<EventVo> allEventSummary = eventPrMsgOperation.getAllEventSummary();
        return allEventSummary;
    }

}
