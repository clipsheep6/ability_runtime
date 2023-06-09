package com.huawei.utils;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.dao.InvalidDataAccessApiUsageException;
import org.springframework.dao.QueryTimeoutException;
import org.springframework.data.redis.RedisConnectionFailureException;
import org.springframework.data.redis.core.Cursor;
import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.data.redis.core.ScanOptions;
import org.springframework.stereotype.Component;

import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.TimeUnit;

/**
 * Redis工具类
 *
 * @since 2022/11/8
 */
@Component
public class RedisOperateUtils {
    private static final Logger LOGGER = LoggerFactory.getLogger(RedisOperateUtils.class);

    @Autowired
    private RedisTemplate redisTemplate;

    /**
     * 获取redis实例中的所有的匹配到的key,Scan模式获取，禁用keys(*)
     *
     * @return {@linkplain Set<String>}
     * @since 2023/3/7
     */
    public Set<String> getAllKeys(String regex, int size) {
        Set<String> res = new HashSet<>();
        StringBuilder sb = new StringBuilder("*" + regex + "*");
        redisTemplate.execute(redisConnectionInstance -> {
            Set<String> binaryKeySet = new HashSet<>();
            ScanOptions.ScanOptionsBuilder scanOptionsBuilder = ScanOptions.scanOptions();
            Cursor<byte[]> scanCursor = redisConnectionInstance.scan(scanOptionsBuilder
                    .match(String.valueOf(sb))
                    .count(size)
                    .build()
            );
            while (scanCursor.hasNext() && binaryKeySet.size() < size) {
                binaryKeySet.add(new String(scanCursor.next()));
            }
            res.addAll(binaryKeySet);
            return binaryKeySet;
        }, true);

        return res;
    }

    /**
     * 批量删除redis库中的key,模糊匹配某个字段
     *
     * @since 2023/3/7
     */
    public void deleteKeys(String regex) {
        StringBuilder sb = new StringBuilder("*" + regex + "*");
        redisTemplate.execute(redisConnectionInstance -> {
            ScanOptions.ScanOptionsBuilder scanOptionsBuilder = ScanOptions.scanOptions();
            Cursor<byte[]> scan = redisConnectionInstance.scan(scanOptionsBuilder
                    .match(String.valueOf(regex))
                    .build());
            while (scan.hasNext()) {
                redisConnectionInstance.del(scan.next());
            }
            return true;
        }, true);
    }


    /**
     * 将当前 reids 中的某个key删除
     *
     * @param key redis中的key
     * @return boolean {@linkplain RedisTemplate}
     * @since 2023/3/7
     */
    public boolean delete(String key) {
        boolean result = false;
        if (redisTemplate.hasKey(key)) {
            try {
                result = redisTemplate.delete(key);
            } catch (RedisConnectionFailureException | QueryTimeoutException exception) {
                LOGGER.error(exception.getMessage(), exception);
            }
        }
        return result;
    }

    /**
     * 检查当前key是否已存在redis中
     *
     * @param key redis中的key
     * @return boolean
     * @since 2022/11/8
     */
    public boolean hasKey(String key) {
        boolean result = false;
        try {
            result = redisTemplate.hasKey(key);
        } catch (RedisConnectionFailureException | QueryTimeoutException exception) {
            LOGGER.error(exception.getMessage(), exception);
        }
        return result;
    }

    /**
     * 检查当前key是否已存在redis中,存在则返回value 否则返回null
     *
     * @param key redis中的key
     * @return Object
     * @since 2022/11/8
     */
    public Object get(Object key) {
        Object object = null;
        try {
            object = redisTemplate.opsForValue().get(key);
        } catch (RedisConnectionFailureException | QueryTimeoutException exception) {
            LOGGER.error(exception.getMessage(), exception);
        }
        return object;
    }

    /**
     * 将redis中的key值对应的value 设置为入参value。
     * 第一次成功返回null 否则返回 原始value
     *
     * @param key   redis中的key
     * @param value 将入参value 设置为当前key对应的value
     * @return boolean
     * @since 2022/11/8
     */
    public Object getAndSet(String key, Object value) {
        Object object = null;
        try {
            object = redisTemplate.opsForValue().getAndSet(key, value);
        } catch (RedisConnectionFailureException | QueryTimeoutException exception) {
            LOGGER.error(exception.getMessage(), exception);
        }
        return object;
    }

    /**
     * 将当前key-value 入redis中
     *
     * @param key   redis中的key
     * @param value 将入参value 设置为当前key对应的value
     * @since 2022/11/8
     */
    public void set(String key, Object value) {
        try {
            redisTemplate.opsForValue().set(key, value);
        } catch (InvalidDataAccessApiUsageException e) {
            LOGGER.error(e.getMessage(), e);
        }
    }

    /**
     * 将当前key-value 入redis中, 并且设置超时时间
     *
     * @param key      redis中的key
     * @param value    将入参value 设置为当前key对应的value
     * @param time     时间数
     * @param timeUnit 时间的单位
     * @since 2022/11/8
     */
    public void set(String key, Object value, long time, TimeUnit timeUnit) {
        try {
            redisTemplate.opsForValue().set(key, value, time, timeUnit);
        } catch (InvalidDataAccessApiUsageException e) {
            LOGGER.error(e.getMessage(), e);
        }
    }

    /**
     * 如果当前key不存在于redis中 则存入key-value,否则失败放弃操作。
     *
     * @param key      redis中的key
     * @param value    将入参value 设置为当前key对应的value
     * @param time     时间数
     * @param timeUnit 时间的单位
     * @return boolean
     * @since 2022/11/8
     */
    public boolean setIfAbsent(String key, Object value, long time, TimeUnit timeUnit) {
        boolean result = false;
        try {
            result = redisTemplate.opsForValue().setIfAbsent(key, value, time, timeUnit);
        } catch (RedisConnectionFailureException exception) {
            LOGGER.error(exception.getMessage(), exception);
        }
        return result;
    }
}
