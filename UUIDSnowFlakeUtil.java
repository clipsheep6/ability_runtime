package com.huawei.utils;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;

/**
 * @Description: 使用雪花算法生成全局唯一id，注意问题: 多服务端必须保持时间同步。
 * @since 2022/11/24  14:22
 **/
@Component
@Data
@AllArgsConstructor
@NoArgsConstructor
public class UUIDSnowFlakeUtil {

    private static final Logger logger = LoggerFactory.getLogger(UUIDSnowFlakeUtil.class);

    /**
     * 工作id
     */
    private long workerId;

    /**
     * 数据id
     */
    private long datacenterId;

    /**
     * 12位序列号
     */
    private long sequence;

    public UUIDSnowFlakeUtil(long workerId, long datacenterId, long sequence) {
        // 检查入参合法性
        if (workerId > maxWorkerId || workerId < 0) {
            throw new IllegalArgumentException(String.format("worker Id can't be greater than %s or less than 0", maxWorkerId));
        }
        if (datacenterId > maxDatacenterId || datacenterId < 0) {
            throw new IllegalArgumentException(String.format("datacenter Id can't be greater than %s or less than 0", maxDatacenterId));
        }

        logger.info("worker starting. timestamp left shift :{}, datacenter id bits :{}, worker id bits :{}, sequence bits :{}, workerid :{}",
                timestampLeftShift, datacenterIdBits, workerIdBits, sequenceBits, workerId);

        this.workerId = workerId;

        this.datacenterId = datacenterId;

        this.sequence = sequence;
    }

    /**
     * 初始时间戳 使用当前时间戳: 2022/11/24  16:10
     */
    private long twePoch = 1669276750164L;

    /**
     * 长度为5位
     */
    private long workerIdBits = 5L;

    private long datacenterIdBits = 5L;
    /**
     * 最大值
     */
    private long maxWorkerId = ~(-1L << workerIdBits);

    private long maxDatacenterId = ~(-1L << datacenterIdBits);

    /**
     * 序列号长度
     */
    private long sequenceBits = 12L;

    //  序列号最大值
    private long sequenceMask = ~(-1L << sequenceBits);

    //  工作id需要左移的位数，12位
    private long workerIdShift = sequenceBits;

    //  数据id需要左移位数 12+5=17位
    private long datacenterIdShift = sequenceBits + workerIdBits;

    //  时间戳需要左移位数 12+5+5=22位
    private long timestampLeftShift = sequenceBits + workerIdBits + datacenterIdBits;

    //  上次时间戳，初始值为负数
    private long lastTimestamp = -1L;

    public long getTimestamp() {
        return System.currentTimeMillis();
    }

    // 获取时间戳
    private long tilNextMillis(long lastTimestamp) {
        long timestamp = timeGen();
        while (timestamp <= lastTimestamp) {
            timestamp = timeGen();
        }
        return timestamp;
    }

    // 获取系统时间戳
    private long timeGen() {
        return System.currentTimeMillis();
    }

    // 生成下一个id算法
    public synchronized long nextId() {
        long timestamp = timeGen();

        // 获取当前时间戳如果小于上次时间戳，则表示时间戳获取出现异常
        if (timestamp < lastTimestamp) {
            logger.info("clock is moving backwards.  Rejecting requests until: {} ", lastTimestamp);
            // 获取间隔时间的下一时间单位的时间戳
            throw new RuntimeException(String.format("Clock moved backwards.  Refusing to generate id for :{} milliseconds",
                    lastTimestamp - timestamp));
        }

        // 获取当前时间戳如果等于上次时间戳（同一毫秒内），则在序列号加一；否则序列号赋值为0，从0开始。
        if (lastTimestamp == timestamp) {
            sequence = (sequence + 1) & sequenceMask;
            if (sequence == 0) {
                timestamp = tilNextMillis(lastTimestamp);
            }
        } else {
            sequence = 0;
        }

        // 将上次时间戳值刷新
        lastTimestamp = timestamp;
        return ((timestamp - twePoch) << timestampLeftShift) |
                (datacenterId << datacenterIdShift) |
                (workerId << workerIdShift) |
                sequence;
    }
}
