# 元能力基础部件

## 简介

**ability_base**部件作为元能力的基础定义部件，提供组件启动参数（Want），系统环境参数（Configration），URI参数的定义，用于启动应用，获取环境参数等功能。

## 部件内子模块职责

| 子模块名称       | 职责                                                         |
| ---------------- | ------------------------------------------------------------|
| Want模块         | 组件启动参数模块；                                            |
| Configration模块 | 系统环境参数模块；                                            |
| URI模块          | URI参数定义模块；                                             |
| base模块         | 基础数据类型模块；                                            |

## 目录

```
foundation/ability/ability_base
├── frameworks
│   └── js
│       └── napi					# ability_base的napi代码实现
│   └── native 					    # ability_base的核心代码实现
├── interfaces
│   └── inner_api 				    # ability_base的系统内部件间接口 
└── test							# 测试目录
```

## 使用说明
功能模块开发指导可参考[**示例文档**](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/ability/Readme-CN.md)


## 相关仓
元能力子系统

[**ability_base**]

ability_runtime

form_runtime

idl
