# IDL工具

## 简介

在OpenHarmony中，当客户端和服务器进行IPC通信时，需要定义双方都认可的接口，以保障双方可以成功通信，OpenHarmony IDL（OpenHarmony Interface Definition Language）则是一种定义此类接口的工具。OpenHarmony IDL先把需要传递的对象分解成操作系统能够理解的基本类型，并根据开发者的需要封装跨边界的对象。

  **图1** IDL接口描述
  ![IDL-interface-description](./figures/IDL-interface-description.png)

OpenHarmony  IDL接口描述语言主要用于：

- 声明系统服务对外提供的服务接口，根据接口声明在编译时生成跨进程调用（IPC）或跨设备调用（RPC）的代理（Proxy）和桩（Stub）的C/C++代码或JS/TS代码。

- 声明Ability对外提供的服务接口，根据接口声明在编译时生成跨进程调用（IPC）或跨设备调用（RPC）的代理（Proxy）和桩（Stub）的C/C++代码或JS/TS代码。

**图2** IPC/RPC通信模型

![IPC-RPC-communication-model](./figures/IPC-RPC-communication-model.png)

使用OpenHarmony IDL接口描述语言声明接口具有以下优点：

- OpenHarmony IDL中是以接口的形式定义服务，可以专注于定义而隐藏实现细节。

- OpenHarmony IDL中定义的接口可以支持跨进程调用或跨设备调用。根据OpenHarmony IDL中的定义生成的信息或代码可以简化跨进程或跨设备调用接口的实现。

## 部件内子模块职责

| 子模块名称       | 职责                                                         |
| ---------------- | ------------------------------------------------------------|
| 接口文件解析模块         | 解析校验接口定义文件。                                    |
| stub/proxy自动生成模块   | 根据IPC/RPC规格自动生成Stub服务端和Proxy客户端代码         |

## 目录

```
foundation/ability/idl_tool
├── ast                         # idl语法解析定义代码
├── codegen                     # 跨进程通信模板生成模块代码
├── metadata                    # matedata自定义数据解析模块代码
├── parser                      # idl解析模块代码
├── test                        # 测试目录
└── util					    # 公共方法代码
```

## 开发步骤
开发步骤可参考[开发指导](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/IDL/idl-guidelines.md#31-c%E5%BC%80%E5%8F%91%E6%AD%A5%E9%AA%A4)


## 相关仓
元能力子系统

ability_base

ability_runtime

form_fwk

[**idl_tool**]
