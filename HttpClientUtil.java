package com.huawei.ci.common.utils;

import com.huawei.ci.common.entity.HttpResponse;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPatch;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.ContentType;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;
import org.apache.servicecomb.foundation.common.utils.JsonUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.net.URI;
import java.util.Map;
import java.util.Objects;

public class HttpClientUtil {
    private static final Logger logger = LoggerFactory.getLogger(HttpClientUtil.class);

    /**
     * 执行GET请求
     *
     * @param url 调用地址
     * @return    响应信息
     */
    public static HttpResponse getRequest(String url) {
        try (CloseableHttpClient httpClient = HttpClients.createDefault()) {
            HttpGet httpGet = new HttpGet(URI.create(url));
            CloseableHttpResponse response = httpClient.execute(httpGet);
            if (response.getStatusLine().getStatusCode() == 401){
                return null;
            }

            return getResponseInfo(response);
        } catch (IOException e) {
            logger.error("----- get request error, message is: {}", e.getMessage());
            return null;
        }
    }

    /**
     * 执行POST请求
     *
     * @param url     调用地址
     * @param headers 请求头
     * @param body    请求体
     * @return        响应信息，目前包括body字符串，headers响应头数组header[]
     */
    public static HttpResponse postRequest(String url, Map<String, String> headers, Object body) {
        try (CloseableHttpClient httpClient = HttpClients.createDefault()) {
            String reqBodyJson = JsonUtils.writeValueAsString(body);
            StringEntity stringEntity = new StringEntity(reqBodyJson, ContentType.APPLICATION_JSON);
            HttpPost httpPost = new HttpPost(URI.create(url));
            if (Objects.nonNull(headers)) {
                for (Map.Entry<String, String> entry : headers.entrySet()) {
                    httpPost.setHeader(entry.getKey(), entry.getValue());
                }
            }
            httpPost.setEntity(stringEntity);
            CloseableHttpResponse response = httpClient.execute(httpPost);

            return getResponseInfo(response);
        } catch (IOException e) {
            logger.error("----- post request error, message is: {}", e.getMessage());
            return null;
        }
    }

    /**
     * 执行PATCH请求
     *
     * @param url  调用地址
     * @param body 请求体
     * @return     响应信息，目前包括body字符串，headers响应头数组header[]
     */
    public static HttpResponse patchRequest(String url, Object body) {
        try (CloseableHttpClient httpClient = HttpClients.createDefault()) {
            String reqBodyJson = JsonUtils.writeValueAsString(body);
            StringEntity stringEntity = new StringEntity(reqBodyJson, ContentType.APPLICATION_JSON);
            HttpPatch httpPatch = new HttpPatch(URI.create(url));
            httpPatch.setEntity(stringEntity);
            CloseableHttpResponse response = httpClient.execute(httpPatch);

            return getResponseInfo(response);
        } catch (IOException e) {
            logger.error("----- patch request error, message is: {}", e.getMessage());
            return null;
        }
    }

    private static HttpResponse getResponseInfo(CloseableHttpResponse response) throws IOException {
        HttpResponse httpResponse = new HttpResponse();
        httpResponse.setBodyJson(EntityUtils.toString(response.getEntity()));
        httpResponse.setHeaders(response.getAllHeaders());
        return httpResponse;
    }
}
