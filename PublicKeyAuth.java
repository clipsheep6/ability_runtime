package com.huawei.ci.gateway.common;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.huawei.ci.gateway.model.UserContext;
import org.apache.commons.lang.StringUtils;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClientBuilder;
import org.apache.http.impl.client.HttpClients;
import org.springframework.http.client.HttpComponentsClientHttpRequestFactory;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;


/**
 * 普通的登录校验
 */
public class PublicKeyAuth implements Auth {
    private static final int FIVE_MINUTES = 300000;
    private static final int TIME_OUT = 10000;
    private static final String[] SIGHATURE_FIELDS = {"user", "nonce", "timestamp"};
    private static final PublicKeyAuth AUTH = new PublicKeyAuth();
    private static final String ACCOUNT_SERVICE_URL = "cse://account";
    public static PublicKeyAuth getInstance(){
        return AUTH;
    }

    /**
     * 通过签名拿用户登录信息
     */
    @Override
    public UserContext auth(String au) throws IOException {
        ObjectMapper objectMapper = new ObjectMapper();
        Map<String, Object> codeInfo = objectMapper.readValue(au, HashMap.class);
        if (codeInfo == null) {
            throw new SecurityException("403 auth failed");
        }
        Map<String, Object> params = (Map<String, Object>) codeInfo.get("authorization");
        String userName = params.get("user").toString();
        String pk = params.get("pk").toString();

        //解密签名，获得签名json对象
        String signature = codeInfo.get("signature").toString();
        String publicKey = getMatchedPk(userName, pk);
        if (StringUtils.isEmpty(publicKey)) {
            throw new SecurityException("user's matched pk dose not find.");
        }
        Map<String, Object> signatureParams = decoder(signature, publicKey);
        if (signatureParams == null) {
            throw new SecurityException("403 auth failed.");
        }
        //签名中信息必须一致
        for (String field : SIGHATURE_FIELDS) {
            if (!params.get(field).equals(signatureParams.get(field))) {
                throw new SecurityException("403 auth failed.");
            }
        }

        Long timestamp = (Long) params.get("timestamp");
        //签名不能超时
        if (System.currentTimeMillis() - timestamp > FIVE_MINUTES) {
            throw new SecurityException("403 auth failed.");
        }
        return getUserContext(userName);
    }

    @Override
    public String getLoginUrl() {
        return null;
    }

    //获取签名对象
    private String getMatchedPk(String user, String pk) {
        HttpClientBuilder clientBuilder = HttpClients.custom();
        CloseableHttpClient httpClient = clientBuilder.build();
        HttpComponentsClientHttpRequestFactory requestFactory = new HttpComponentsClientHttpRequestFactory();
        requestFactory.setConnectionRequestTimeout(TIME_OUT);
        requestFactory.setConnectTimeout(TIME_OUT);
        requestFactory.setReadTimeout(TIME_OUT);
        requestFactory.setHttpClient(httpClient);
//        RestTemplate restTemplate = RestTemplateBuilder.create();
//        restTemplate.setRequestFactory(requestFactory);
//        String accountPKRestUrl = ACCOUNT_SERVICE_URL + "/users/" + user + "/pks";
//        ResponseEntity<RestResponse> response = restTemplate.getForEntity(accountPKRestUrl, RestResponse.class);
//        RestResponse restResponse = response.getBody();
//        if (restResponse == null) {
//            return "";
//        }
//        List<Map<String, String>> userPkMapList = (List<Map<String, String>>) restResponse.getData();
//        if (userPkMapList.stream().map(userPk -> userPk.get("pk").trim())
//                .anyMatch(value -> value.equals(pk.trim()))) {
//            return pk;
//        }
        return "";
    }

    private UserContext getUserContext(String user) {
        HttpClientBuilder clientBuilder = HttpClients.custom();
        CloseableHttpClient httpClient = clientBuilder.build();
        HttpComponentsClientHttpRequestFactory requestFactory = new HttpComponentsClientHttpRequestFactory();
        requestFactory.setConnectionRequestTimeout(TIME_OUT);
        requestFactory.setConnectTimeout(TIME_OUT);
        requestFactory.setReadTimeout(TIME_OUT);
        requestFactory.setHttpClient(httpClient);
//        RestTemplate restTemplate = RestTemplateBuilder.create();
//        restTemplate.setRequestFactory(requestFactory);
//        String accountPKRestUrl = ACCOUNT_SERVICE_URL + "/users/" + user;
//        ResponseEntity<RestResponse> response = restTemplate.getForEntity(accountPKRestUrl, RestResponse.class);
//        RestResponse restResponse = response.getBody();
//        if (restResponse == null) {
//            throw new SecurityException();
//        }
//        Map<String, String> userMap = (Map<String, String>) restResponse.getData();
        UserContext userContext = new UserContext();
//        userContext.setId(userMap.get("id"));
//        userContext.setName(userMap.get("name"));
//        userContext.getExtendInfo().put("account", userMap.get("account"));
//        userContext.setType(UserContext.AuthType.PK);
//        userContext.setRandomCsrfToken();
        return userContext;
    }

    // todo 转码
    private Map<String, Object> decoder(String sigature, String publicKey) {
        Map<String, Object> signatureParams = null;
        return signatureParams;
    }
}
