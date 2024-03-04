# HUSTCSE-APIHOOK

#### 介绍
信安2104

#### 软件架构
软件架构说明
本GUI项目基于Qt 6.6.0 (x86_64-little_endian-llp64 shared (dynamic) release build; by MSVC 2019) on "windows" 实现


#### 安装教程

1.  项目代码调用文件使用绝对路径，在GUI.pro文件中需要修改detours库的include文件和lib文件地址
2.  在mainwindow.h文件中需要修改dllPath，以匹配注入的dll的地址
3.  在logwindow.h文件中需要修改logFilePath，以完成log.txt文件的创建和识别

#### 使用说明

1.  对需要的可执行文件进行注入时，需要先选择文件，再点击运行按钮进行注入，如果选择的文件不是可执行文件，将不会运行。
2.  在mainwindow窗口中，如果截获api函数，截获的时间、函数名、参数名和值都会显示在左下角的组件。
3.  警告和错误信息会显示在右下角的Info窗口，警告信息颜色为黄色，错误信息为红色。
4.  点击log按钮可以弹出log界面，显示log.txt文件中的文本，信息文本为绿色，截获文本为灰色，警告文本为黄色，错误文本为红色。

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request

