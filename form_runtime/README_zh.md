# 卡片管理

## 简介

卡片是一种界面展示形式，可以将应用的重要信息或操作前置到卡片，以达到服务直达，减少体验层级的目的。

卡片常用于嵌入到其他应用（当前只支持系统应用）中作为其界面的一部分显示，并支持拉起页面，发送消息等基础的交互功能。卡片使用方负责显示卡片。

卡片的基本概念：

- 卡片提供方
  提供卡片显示内容原子化服务，控制卡片的显示内容、控件布局以及控件点击事件。
- 卡片使用方
  显示卡片内容的宿主应用，控制卡片在宿主中展示的位置。
- 卡片管理服务
  用于管理系统中所添加卡片的常驻代理服务，包括卡片对象的管理与使用，以及卡片周期性刷新等。
  
> ![icon-note.gif](figures/icon-note.gif) **说明：**
> 卡片使用方和提供方不要求常驻运行，在需要添加/删除/请求更新卡片时，卡片管理服务会拉起卡片提供方获取卡片信息。


开发者仅需作为卡片提供方进行卡片内容的开发，卡片使用方和卡片管理服务由系统自动处理。

卡片提供方控制卡片实际显示的内容、控件布局以及点击事件。

## 部件内子模块职责

| 子模块名称       | 职责                                                         |
| ---------------- | ------------------------------------------------------------|
| 卡片JS_Napikit模块| 提供外部接口，与卡片管理服务交互，负责事件通知的调度，通过ArkUI更新卡片视图。           |
| 卡片管理服务模块   | 管理系统中所添加卡片的常驻代理服务，管理卡片的生命周期，并维护卡片信息以及卡片事件的调度。 |

## 目录

```
foundation/ability/form_runtime
├── frameworks
│   └── js
│       └── napi					# form_runtime的napi代码实现
├── interfaces
│   ├── inner_api 				    # form_runtime的系统内部件间接口 
│   └── kits
│       └── native				    # form_runtime的对外接口  
├── services                        # form管理服务框架代码
└── test							# 测试目录
```

## 使用说明
Stage卡片开发，即基于[Stage模型](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/ability/stage-brief.md)的卡片提供方开发，主要涉及如下功能逻辑：

- 卡片生命周期回调函数FormExtension开发。
- 创建卡片数据FormBindingData对象。
- 通过FormProvider更新卡片。
- 卡片页面开发。

## 接口说明

FormExtension功能如下：FormExtension类，拥有context属性，具体的API详见[接口文档](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis/js-apis-formextension.md)。

**表1** FormExtension API接口功能介绍

| 接口名                                                       | 描述                                         |
| :----------------------------------------------------------- | :------------------------------------------- |
| onCreate(want: Want): formBindingData.FormBindingData        | 卡片提供方接收创建卡片的通知接口。           |
| onCastToNormal(formId: string): void                         | 卡片提供方接收临时卡片转常态卡片的通知接口。 |
| onUpdate(formId: string): void                               | 卡片提供方接收更新卡片的通知接口。           |
| onVisibilityChange(newStatus: { [key: string]: number }): void | 卡片提供方接收修改可见性的通知接口。         |
| onEvent(formId: string, message: string): void               | 卡片提供方接收处理卡片事件的通知接口。       |
| onDestroy(formId: string): void                              | 卡片提供方接收销毁卡片的通知接口。           |
| onConfigurationUpdated(config: Configuration): void;         | 当系统配置更新时调用。                       |

FormExtension类拥有context属性，context属性为FormExtensionContext类，具体的API详见[接口文档](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis/js-apis-formextensioncontext.md)。

**表2** FormExtensionContext API接口功能介绍

| 接口名                                                       | 描述                      |
| :----------------------------------------------------------- | :------------------------ |
| updateForm(formId: string, formBindingData: formBindingData.FormBindingData, callback: AsyncCallback\<void>): void | 回调形式主动更新卡片。    |
| updateForm(formId: string, formBindingData: formBindingData.FormBindingData): Promise\<void> | Promise形式主动更新卡片。 |

FormProvider类具体的API详见[接口文档](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis/js-apis-formprovider.md)。

**表3** FormProvider API接口功能介绍

| 接口名                                                       | 描述                                              |
| :----------------------------------------------------------- | :------------------------------------------------ |
| setFormNextRefreshTime(formId: string, minute: number, callback: AsyncCallback&lt;void&gt;): void; | 设置指定卡片的下一次更新时间。                    |
| setFormNextRefreshTime(formId: string, minute: number): Promise&lt;void&gt;; | 设置指定卡片的下一次更新时间，以promise方式返回。 |
| updateForm(formId: string, formBindingData: FormBindingData, callback: AsyncCallback&lt;void&gt;): void; | 更新指定的卡片。                                  |
| updateForm(formId: string, formBindingData: FormBindingData): Promise&lt;void&gt;; | 更新指定的卡片，以promise方式返回。               |

## 开发步骤
开发步骤可参考[开发指导](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/ability/stage-formextension.md#%E5%BC%80%E5%8F%91%E6%AD%A5%E9%AA%A4)

## 相关仓
元能力子系统

ability_base

ability_runtime

[**form_runtime**]

idl
