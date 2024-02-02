/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

package com.huawei.config;

import com.huawei.enums.CiConstants;
import com.huawei.utils.KmsUtil;

import com.google.common.collect.ImmutableMap;

import io.lettuce.core.ReadFrom;

import org.apache.commons.pool2.impl.GenericObjectPoolConfig;
import org.redisson.Redisson;
import org.redisson.api.RedissonClient;
import org.redisson.config.Config;
import org.redisson.config.MasterSlaveServersConfig;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.autoconfigure.condition.ConditionalOnClass;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Primary;
import org.springframework.data.redis.cache.RedisCacheConfiguration;
import org.springframework.data.redis.cache.RedisCacheManager;
import org.springframework.data.redis.connection.RedisConnectionFactory;
import org.springframework.data.redis.connection.RedisStandaloneConfiguration;
import org.springframework.data.redis.connection.RedisStaticMasterReplicaConfiguration;
import org.springframework.data.redis.connection.lettuce.LettuceConnectionFactory;
import org.springframework.data.redis.connection.lettuce.LettucePoolingClientConfiguration;
import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.data.redis.serializer.GenericJackson2JsonRedisSerializer;
import org.springframework.data.redis.serializer.GenericToStringSerializer;
import org.springframework.data.redis.serializer.RedisSerializationContext;
import org.springframework.data.redis.serializer.RedisSerializer;
import org.springframework.data.redis.serializer.StringRedisSerializer;

import java.time.Duration;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

import javax.annotation.Resource;

/**
 * redis配置类
 *
 * @since 2022-10-14
 */
@Configuration
public class RedisConfig {
    static {
        ENV_LIST = Arrays.asList("beta", "alpha");
    }

    private static final List<String> ENV_LIST;

    /**
     * 初始最小空闲工作池数
     */
    public static final Integer DEFAULT_MIN_IDLE_SIZE = 0;

    /**
     * 最大空闲数工作池
     */
    public static final Integer DEFAULT_MAX_IDLE_SIZE = 20;

    /**
     * 连接池最大连接数
     */
    public static final Integer DEFAULT_MAX_TOTAL_SIZE = 100;

    /**
     * 缓存时间 15分钟
     */
    public static final String CACHE_15_MIN = "cache:15m";

    /**
     * 缓存时间 30分钟
     */

    public static final String CACHE_30_MIN = "cache:30m";

    /**
     * 缓存时间 60分钟
     */
    public static final String CACHE_60_MIN = "cache:60m";

    /**
     * 缓存时间 180分钟
     */
    public static final String CACHE_180_MIN = "cache:180m";

    @Value("${spring.redis.master-url}")
    private String masterUrl;

    @Value("${spring.redis.slave-url-node}")
    private String slaveUrlNode;

    @Value("${server.environment}")
    private String environment;

    @Value("${spring.redis.password}")
    private String password;

    @Resource
    private KmsUtil kmsUtil;

    /**
     * 获取LettuceConnectionFactory的连接池
     *
     * @return {@link LettuceConnectionFactory}
     */
    @Primary
    @Bean("redisFactory")
    public LettuceConnectionFactory redisConnectionFactory() {
        String[] server = this.masterUrl.split(CiConstants.REDIS_URL_DIVIDE_COLON);
        String masterLink = server[CiConstants.NUM_ZERO];
        int masterPort = Integer.parseInt(server[CiConstants.NUM_ONE]);
        RedisStandaloneConfiguration redisStandaloneConfiguration = new RedisStandaloneConfiguration(masterLink,
            masterPort);
        redisStandaloneConfiguration.setPassword(kmsUtil.decrypt(password));
        if (ENV_LIST.contains(environment)) {
            return new LettuceConnectionFactory(redisStandaloneConfiguration);
        } else {
            RedisStaticMasterReplicaConfiguration redisMasterWithSlaveConfiguration
                = new RedisStaticMasterReplicaConfiguration(masterLink, masterPort);
            Arrays.stream(this.slaveUrlNode.split(CiConstants.REDIS_URL_DIVIDE_DOT))
                .map(node -> node.split(CiConstants.REDIS_URL_DIVIDE_COLON))
                .collect(Collectors.toList())
                .forEach(nodeServer -> {
                    String nodeServerUrl = nodeServer[CiConstants.NUM_ZERO];
                    int nodeServerPort = Integer.parseInt(nodeServer[CiConstants.NUM_ONE]);
                    redisMasterWithSlaveConfiguration.addNode(nodeServerUrl, nodeServerPort);
                });
            LettucePoolingClientConfiguration lettucePool = LettucePoolingClientConfiguration.builder()
                .commandTimeout(Duration.ofMinutes(CiConstants.NUM_ONE))
                .readFrom(ReadFrom.REPLICA_PREFERRED)
                .build();
            initPoolConfig(lettucePool);
            redisMasterWithSlaveConfiguration.setPassword(kmsUtil.decrypt(password));
            return new LettuceConnectionFactory(redisMasterWithSlaveConfiguration, lettucePool);
        }
    }

    private void initPoolConfig(LettucePoolingClientConfiguration lettucePooling) {
        GenericObjectPoolConfig poolConfig = lettucePooling.getPoolConfig();
        poolConfig.setMinIdle(DEFAULT_MIN_IDLE_SIZE);
        poolConfig.setMaxIdle(DEFAULT_MAX_IDLE_SIZE);
        poolConfig.setMaxTotal(DEFAULT_MAX_TOTAL_SIZE);
    }

    /**
     * redis操作使用工具
     *
     * @param redisConnectionFactory redis连接工厂
     * @return {@link RedisTemplate}
     */
    @Bean("redisTemplate")
    @ConditionalOnClass(LettuceConnectionFactory.class)
    public RedisTemplate<String, Object> redisTemplate(
        @Qualifier("redisFactory") LettuceConnectionFactory redisConnectionFactory) {
        RedisTemplate<String, Object> redisTemplate = new RedisTemplate<>();
        redisTemplate.setConnectionFactory(redisConnectionFactory);
        redisTemplate.setKeySerializer(RedisSerializer.string());
        redisTemplate.setHashKeySerializer(RedisSerializer.string());
        redisTemplate.setValueSerializer(new GenericToStringSerializer(Object.class));
        redisTemplate.setHashValueSerializer(new GenericToStringSerializer(Object.class));
        redisTemplate.afterPropertiesSet();
        return redisTemplate;
    }

    /**
     * spring的缓存管理器，配置各个缓存的时间组和序列化方式。主要是缓存注解使用@Cacheable,@CacheEvict,@CachePut...
     *
     * @param redisConnectionFactory redis连接工厂
     * @return {@link RedisCacheManager}
     */
    @Bean("redisCacheManager")
    public RedisCacheManager redisCacheManager(RedisConnectionFactory redisConnectionFactory) {
        Map<String, RedisCacheConfiguration> cacheConfig = ImmutableMap.<String, RedisCacheConfiguration>builder().put(
            CACHE_15_MIN, RedisCacheConfiguration.defaultCacheConfig()
                .entryTtl(Duration.ofMinutes(15))
                .serializeKeysWith(
                    RedisSerializationContext.SerializationPair.fromSerializer(new StringRedisSerializer()))
                .serializeValuesWith(RedisSerializationContext.SerializationPair.fromSerializer(
                    new GenericJackson2JsonRedisSerializer())))
            .put(CACHE_30_MIN, RedisCacheConfiguration.defaultCacheConfig()
                .entryTtl(Duration.ofMinutes(30))
                .serializeKeysWith(
                    RedisSerializationContext.SerializationPair.fromSerializer(new StringRedisSerializer()))
                .serializeValuesWith(RedisSerializationContext.SerializationPair.fromSerializer(
                    new GenericJackson2JsonRedisSerializer())))
            .put(CACHE_60_MIN, RedisCacheConfiguration.defaultCacheConfig()
                .entryTtl(Duration.ofMinutes(60))
                .serializeKeysWith(
                    RedisSerializationContext.SerializationPair.fromSerializer(new StringRedisSerializer()))
                .serializeValuesWith(RedisSerializationContext.SerializationPair.fromSerializer(
                    new GenericJackson2JsonRedisSerializer())))
            .put(CACHE_180_MIN, RedisCacheConfiguration.defaultCacheConfig()
                .entryTtl(Duration.ofMinutes(180))
                .serializeKeysWith(
                    RedisSerializationContext.SerializationPair.fromSerializer(new StringRedisSerializer()))
                .serializeValuesWith(RedisSerializationContext.SerializationPair.fromSerializer(
                    new GenericJackson2JsonRedisSerializer())))
            .build();
        return RedisCacheManager.RedisCacheManagerBuilder.fromConnectionFactory(redisConnectionFactory)
            .withInitialCacheConfigurations(cacheConfig)
            .build();
    }

    /**
     * redis序列化配置
     *
     * @return {@link RedisCacheConfiguration}
     */
    @Bean("redisCacheConfiguration")
    @Primary
    public RedisCacheConfiguration redisCacheConfiguration() {
        return RedisCacheConfiguration.defaultCacheConfig()
            .serializeKeysWith(RedisSerializationContext.SerializationPair.fromSerializer(new StringRedisSerializer()))
            .serializeValuesWith(
                RedisSerializationContext.SerializationPair.fromSerializer(new GenericJackson2JsonRedisSerializer()));
    }

    /**
     * redisson 客户端配置
     *
     * @return redissonClient {@linkplain RedissonClient}
     */
    @Bean("redissonClient")
    public RedissonClient redissonClient() {
        Config config = new Config();
        if (ENV_LIST.contains(environment)) {
            config.useSingleServer()
                .setAddress(CiConstants.REDIS_URL_PREFIX + this.masterUrl)
                .setPassword(kmsUtil.decrypt(password));
        } else {
            // RMapCache 实现不依赖写入。
            MasterSlaveServersConfig masterSlaveServersConfig = config.useMasterSlaveServers();
            masterSlaveServersConfig.setMasterAddress(CiConstants.REDIS_URL_PREFIX + this.masterUrl);
            Set<String> nodeSet = Arrays.stream(this.slaveUrlNode.split(CiConstants.REDIS_URL_DIVIDE_DOT))
                .map(nodeSlave -> CiConstants.REDIS_URL_PREFIX + nodeSlave)
                .collect(Collectors.toSet());
            masterSlaveServersConfig.setSlaveAddresses(nodeSet);
            masterSlaveServersConfig.setPassword(kmsUtil.decrypt(password));
        }
        return Redisson.create(config);
    }
}
