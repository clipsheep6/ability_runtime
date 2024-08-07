# 元能力子系统

## 简介

**元能力子系统**实现对Ability的运行及生命周期进行统一的调度和管理，应用进程能够支撑多个Ability，Ability具有跨应用进程间和同一进程内调用的能力。Ability管理服务统一调度和管理应用中各Ability，并对Ability的生命周期变更进行管理。

![](figures/ability.png)

**元能力子系统架构图说明：**

- **Ability Kit**为Ability的运行提供基础的运行环境支撑。Ability是系统调度应用的最小单元，是能够完成一个独立功能的组件，一个应用可以包含一个或多个Ability。

- **Ability管理服务（AbilityManagerService）**：用于协调各Ability运行关系、及对生命周期进行调度的系统服务。
  - 连接管理模块（AbilityConnectManager）是Ability管理服务对Service类型Ability连接管理的模块。
  - 数据管理模块（DataAbilityManager）是Ability管理服务对Data类型Ability管理的模块。
  - App管理服务调度模块（AppScheduler）提供Ability管理服务对用户程序管理服务进行调度管理的能力。
  - Ability调度模块（AbilityScheduler）提供对Ability进行调度管理的能力。
  - 生命周期调度模块（LifecycleDeal）是Ability管理服务对Ability的生命周期事件进行管理调度的模块。

- Ability框架模型结构具有两种框架形态：

  - 第一种形态为FA模型。API 8及其更早版本的应用程序只能使用FA模型进行开发。 FA模型将Ability分为FA（Feature Ability）和PA（Particle Ability）两种类型，其中FA支持Page Ability，PA支持Service Ability、Data Ability、以及FormAbility。
  - 第二种形态为Stage模型。从API 9开始，Ability框架引入了Stage模型作为第二种应用框架形态，Stage模型将Ability分为Ability和ExtensionAbility两大类，其中ExtensionAbility又被扩展为ServiceExtensionAbility、FormExtensionAbility、DataShareExtensionAbility等等一系列ExtensionAbility，以便满足更多的使用场景。

  ​Stage模型的设计，主要是为了方便开发者更加方便地开发出分布式环境下的复杂应用。下表给出了两种模型在设计上的差异：

  | 对比           | FA模型                                                       | Stage模型                                                |
  | -------------- | ------------------------------------------------------------ | -------------------------------------------------------- |
  | 开发方式       | 提供类Web的 api，UI开发与Stage模型一致。                     | 提供面向对象的开发方式，UI开发与FA模型一致。             |
  | 引擎实例       | 每个进程内的每个Ability独享一个JS VM引擎实例。               | 每个进程内的多个Ability实例共享一个JS VM引擎实例。       |
  | 进程内对象共享 | 不支持。                                                     | 支持。                                                   |
  | 包描述文件     | 使用config.json描述HAP包和组件信息，组件必须使用固定的文件名。 | 使用module.json描述HAP包和组件信息，可以指定入口文件名。 |
  | 组件           | 提供PageAbility(页面展示)，ServiceAbility(服务)，DataAbility(数据分享), FormAbility(卡片)。 | 提供Ability(页面展示)、Extension(基于场景的服务扩展)。   |

  ​        除了上述设计上的差异外，对于开发者而言，两种模型的主要区别在于：

  * Ability类型存在差异；

    ![favsstage](figures/favsstage.png)

  * Ability生命周期存在差异；

    ![lifecycle](figures/lifecycle.png)


## 目录

```
foundation/ability            #元能力子系统
├── ability_runtime           #ability_runtime元能力运行时部件
│   ├── frameworks
│   │   ├── js
│   │   │   └── napi          # ability_runtime的napi代码实现
│   │   └── native            # ability_runtime的核心代码实现
│   ├── interfaces
│   │   ├── inner_api         # ability_runtime的系统内部件间接口
│   │   └── kits
│   │       └── native        # ability_runtime的对外接口  
│   ├── services
│   │   ├── abilitymgr        # Ability管理服务框架代码
│   │   ├── appmgr            # App管理服务框架代码
│   │   ├── common            # 服务公共组件目录
│   │   ├── dataobsmgr        # DataAbilityObserver管理服务框架代码
│   │   └── uripermmgr        # UriPermission管理服务框架代码
│   ├── test                  # 测试目录
│   └── tools                 # aa命令工具代码目录
│
├── ability_base              # ability_base元能力基础部件
│
├── ability_lite              # ability_lite轻量化元能力部件
│
├── dmsfwk                    # dmsfwk分布式组件管理部件
│
├── dmsfwk_lite               # dmsfwk_lite轻量化分布式组件管理部件
│
├── form_fwk                  # form_fwk卡片运行时部件
│
├── idl_tool                  # idl工具部件

```

## 使用说明
### 启动Abiltiy
启动新的ability(callback形式)

* startAbility参数描述

| 名称      | 读写属性 | 类型                  | 必填 | 描述                |
| --------- | -------- | --------------------- | ---- | ------------------- |
| parameter | 读写     | StartAbilityParameter | 是   | 表示被启动的Ability |
| callback  | 只读     | AsyncCallback         | 是   | 被指定的回调方法    |

- StartAbilityParameter类型说明

| 名称                | 读写属性 | 类型   | 必填 | 描述                               |
| ------------------- | -------- | ------ | ---- | ---------------------------------- |
| want                | 读写     | want   | 是   | 表示需要包含有关目标启动能力的信息 |
| abilityStartSetting | 只读     | string | 否   | 指示启动能力中使用的特殊启动设置   |

- want类型说明

| 名称         | 读写属性 | 类型                 | 必填 | 描述                            |
| ------------ | -------- | -------------------- | ---- | ------------------------------- |
| deviceId     | 读写     | string               | 否   | 设备id                          |
| bundleName   | 读写     | string               | 否   | Bundle名                        |
| abilityName  | 读写     | string               | 否   | Ability 名                      |
| uri          | 读写     | string               | 否   | 请求中URI的描述                 |
| type         | 读写     | string               | 否   | Want中类型的说明                |
| flags        | 读写     | number               | 否   | Want中标志的选项，必填          |
| action       | 读写     | string               | 否   | Want中对操作的描述              |
| parameters   | 读写     | {[key: string]: any} | 否   | Want中WantParams对象的描述      |
| entities     | 读写     | string               | 否   | 对象中实体的描述                |
| moduleName9+ | 读写     | string               | 否   | Ability所属的模块（module）名称 |

* 返回值

  void

* 示例

  更多开发指导可参考[**示例文档**](https://gitee.com/openharmony/docs/tree/master/zh-cn/application-dev/application-models/Readme-CN.md)


## **aa命令**

**aa help**

| 命令    | 描述               |
| ------- | ------------------ |
| aa help | 显示aa命令帮助信息 |

**aa start**

| 命令                                                           | 描述                      |
| -------------------------------------------------------------- | ------------------------ |
| aa start [-d <device>] -a <ability-name> -b <bundle-name> [-D] | 启动ability，设备ID 可空  |

```
示例：
aa start -d 12345 -a com.ohos.app.MainAbility -b com.ohos.app -D
```

**aa dump**

| 命令       | 描述                  |
| ---------- | --------------------- |
| aa dump -a | 打印栈中的Ability信息 |

**aa force-stop**

| 命令                                                           | 描述                      |
| -------------------------------------------------------------- | ------------------------ |
| aa force-stop <bundle-name> [-p <pid>] [-r <kill-reason>] | 强制停止application，支持传递pid和进程退出原因 |

```
示例：
aa force-stop com.ohos.app
```

## 相关仓
元能力子系统

[ability_base](https://gitee.com/openharmony/ability_ability_base)

[ability_lite](https://gitee.com/openharmony/ability_ability_lite)

[**ability_runtime**](https://gitee.com/openharmony/ability_ability_runtime)

[dmsfwk](https://gitee.com/openharmony/ability_dmsfwk)

[dmsfwk_lite](https://gitee.com/openharmony/ability_dmsfwk_lite)

[form_fwk](https://gitee.com/openharmony/ability_form_fwk)

[idl_tool](https://gitee.com/openharmony/ability_idl_tool)