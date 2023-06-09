package com.huawei.utils;

import lombok.extern.java.Log;
import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ScheduledThreadPoolExecutor;

/**
 * @description juc init
 * @classname com.huawei.utils
 * @since 2023-06-08 10:01
 **/
@Log
@Component
public class ParallelTaskFactory {
    /**
     * core size
     */
    private static final int CORE_SIZE = Runtime.getRuntime().availableProcessors();

    @FunctionalInterface
    public interface IExecutorConsumerTask {
        void doRun();

    }
    @FunctionalInterface
    public interface IExecutorProviderTask {
        Object doRun();

    }

    /**
     * consumer task
     *
     * @param task  IExecutorVoidTask
     * @return  CompletableFuture<Void>
     */
    public CompletableFuture<Void> addConsumerTask(IExecutorConsumerTask task) {
        return CompletableFuture.runAsync(task::doRun, new ScheduledThreadPoolExecutor(CORE_SIZE));
    }

    /**
     * do consumer task
     */
    public void doConsumerTask(List<CompletableFuture<Object>> consumerTaskList) {
        if (CollectionUtils.isEmpty(consumerTaskList)) {
            CompletableFuture.allOf(consumerTaskList.toArray(new CompletableFuture[0])).join();
        }
    }

    /**
     * add provider task
     *
     * @param task IExecutorProviderTask
     * @return CompletableFuture<Void>
     */
    public CompletableFuture<Object> addProviderExecuteTask(IExecutorProviderTask task) {
        return CompletableFuture.supplyAsync(task::doRun, new ScheduledThreadPoolExecutor(CORE_SIZE));
    }

    /**
     * do provider task
     *
     * @param providerTaskList {@linkplain List<CompletableFuture<Object>>}
     */
    public List<Object> doProviderTasksCollect(List<CompletableFuture<Object>> providerTaskList) {
        List<Object> list = new ArrayList<>();
        if (!providerTaskList.isEmpty()) {
            CompletableFuture[] completableFutures = providerTaskList.toArray(new CompletableFuture[0]);
            CompletableFuture.allOf(completableFutures).join();
            Arrays.stream(completableFutures).forEach(future -> {
                Object obj = null;
                try {
                    obj = future.get();
                } catch (InterruptedException | ExecutionException exception) {
                    log.info(exception.getMessage());
                }
                list.add(obj);
            });
        }
        return list;
    }


}
