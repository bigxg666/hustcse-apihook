# HUSECSE-APIHOOK

#### 介绍
信安2104

#### 软件架构
Detours-main: Detours库的源码  
Detours: 编译好的Detours库  
Dll1: 测试用Dll  
FCInject: 注入程序  
FileDll: 文件操作行为监测  
heapDll: 堆操作行为监测  
RegDll: 注册表相关行为监测  
hookDll: 总的行为监测Dll,  **能够向共享内存中写入日志**   
ShareMemoryTest: 共享内存测试  
&emsp; **ReadLog: 从共享内存中读取日志并显示**   
test_exe: 测试程序（包含源码）  
&emsp;filetest: 文件行为测试程序  
&emsp;heaptest: 文件行为测试程序  
&emsp;regtest: 注册表等行为测试程序  

#### 安装教程

1.  xxxx
2.  xxxx
3.  xxxx

#### 使用说明

1. 由于Dll向共享内存写入数据需要时间，因此当测试程序在短时间内进行大量操作时，ReadLog模块将无法全部将操作显示出来。在设计测试程序时需要注意在不同操作之间**增加停顿**或是**sleep**。
2.  xxxx
3.  xxxx

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
