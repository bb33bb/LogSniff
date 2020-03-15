# LogView-Windows系统本地调试日志捕获工具

Windows平台日志嗅探工具，可以方便的嗅探本本地的调试信息，包括像DbgView一样捕获系统函数OutputDebugString输出的调试信息以及输出到日志文件的调试信息。
主要特色功能:
1.小巧绿色，只有一个可执行文件，没有额外的依赖，运行速度快.
2.日志数据全部保存到内存里，提供简洁易用的日志检索语句随时进行检索。
3.实时探测指定文件夹下所有的日志文件变化，并实时展示到日志页面。
4.漂亮的语法高亮展示，不同的高亮颜色区分检索语句中不同的关键字。

![输入图片说明](https://images.gitee.com/uploads/images/2020/0315/161413_00aee409_498054.png "222png.png")

#### 软件架构
1.OutputDebugString调试信息的捕获和DbgView原理类似，通过系统创建的共享内存块获取OutputDebugString输出的调试内容。
2.文件日志探测是通过ReadDirectoryChangesW接口配合完成端口实现的高效文件变化探测。
3.日志展示界面是


#### 安装教程

1. xxxx
2. xxxx
3. xxxx

#### 使用说明

1. xxxx
2. xxxx
3. xxxx

#### 参与贡献

1. Fork 本仓库
2. 新建 Feat_xxx 分支
3. 提交代码
4. 新建 Pull Request


#### 码云特技

1. 使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2. 码云官方博客 [blog.gitee.com](https://blog.gitee.com)
3. 你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解码云上的优秀开源项目
4. [GVP](https://gitee.com/gvp) 全称是码云最有价值开源项目，是码云综合评定出的优秀开源项目
5. 码云官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6. 码云封面人物是一档用来展示码云会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)