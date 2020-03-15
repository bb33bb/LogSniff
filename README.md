# LogView-Windows系统本地调试日志捕获工具


```
Windows平台日志嗅探工具，可以方便的嗅探本本地的调试信息，包括像DbgView一样捕获系统函数OutputDebugString输出的调试信息以及输出到日志文件的调试信息。
主要特色功能:
1.小巧绿色，只有一个可执行文件，没有额外的依赖，运行速度快.
2.日志数据全部保存到内存里，提供简洁易用的日志检索语句随时进行检索（DbgView过滤后不合规则的数据就扔掉了）。
3.实时探测指定文件夹下所有的日志文件变化，并实时展示到日志页面。
4.漂亮的语法高亮展示，不同的高亮颜色区分检索语句中不同的关键字。
```


![输入图片说明](https://images.gitee.com/uploads/images/2020/0315/161413_00aee409_498054.png "222png.png")

#### 软件架构

```
1.OutputDebugString调试信息的捕获和DbgView原理类似，通过系统创建的共享内存块获取OutputDebugString输出的调试内容。
2.文件日志探测是通过ReadDirectoryChangesW接口配合完成端口实现的高效文件变化探测。
3.日志展示界面用的notepad++同款scintilla控件，用于日志内容和高亮关键字的展示。
```



#### 使用说明
无需安装，只有一个绿色的可执行文件，打开就能使用，如果使用日志监控需要在配置选项输入日志文件所在的路径。
配置选项页面：设置日志文件所在目录，设置完成后就可以监控该目录下的所有日志文件，并实时在文件日志界面中展示出来。
调试输出页面：类似DbgView展示程序通过OutputDebugString输出的调试信息。
文件日志页面：展示文件日志输出的信息。
文件检索页面：从配置页面配置的目录的所有日志文件中检索内容。
日志信息过滤规则：


eg:
```
keyword1                            // 保留单条日志中包含keyword的日志
keyword1 && keyword2                // 保留单条日志中同时包含keyword1和keyword2的日志
keyword1 || keyword2                // 保留单条日志中包含keyword1或者keyword2的日志
!keyword                            // 保留单条日志不包含keyword的日志
keyword1 && (keyword2 || keyword3)  // 保留单条日志包含keyword1并且包含keyword2或者keyword3的日志
```

![输入图片说明](https://images.gitee.com/uploads/images/2020/0315/181327_3ed5e877_498054.png "4444.png")
![输入图片说明](https://images.gitee.com/uploads/images/2020/0315/181339_e5bfbf03_498054.png "5555.png")

#### 码云特技

1. 使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2. 码云官方博客 [blog.gitee.com](https://blog.gitee.com)
3. 你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解码云上的优秀开源项目
4. [GVP](https://gitee.com/gvp) 全称是码云最有价值开源项目，是码云综合评定出的优秀开源项目
5. 码云官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6. 码云封面人物是一档用来展示码云会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)