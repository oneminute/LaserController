#Release Notes

#### 2021-10-06 0.3.2.2
特性
1. 实现了圆角矩形图元的导入、绘制和导出；
2. 在Json文档中加入了Absolute属性；

#### 2021-08-29 0.2.0.3
特性
1. 重构了分组与路径优化相关的源码结构；
2. 实现了用户寄存器的读写；
3. 实现了系统寄存器的读操作；
4. 修复了若干bug。

Features
1. Refactored source codes about grouping and path optimization;
2. Implemented reading and writing user registers;
3. Implemented reading system registers;
4. Fixed some other bugs.

#### 2021-08-21 0.2.0.2
特性:
1. 添加了调试参数；
2. 添加了分组中图元数量设置的功能；
3. 添加了横向分组与纵向分组的功能；
4. 修复了路径优化中的多处bug；
5. 修复了建立拓扑时，分组值修改造成图元丢失的bug；
6. 可以手动更新文档拓扑树。

Features:
1. Add debug parameters;
2. Add a parameter to set primitive count of each group;
3. Add a parameter to specify grouping by vertical or horizontal;
4. Fixed many bugs of path optimization;
5. Fixed a bug causing some primitives missing when generate node tre;
6. Add a tool button allowing user update node tree manually.

#### 2021-08-19 0.2.0.1
特性:
1. 在选项中添加了小圆限速选项；
2. 项目中添加了实现小圆限速相关的类；
3. 在ConfigItem类中提供了更多的钩子函数，方便自定义选项的实现；
4. 修改了图层实现；

#### 2021-08-12 0.2.0.0
特性:
1. 检查必须的文件夹是已经存在，不存在则创建；

#### 2021-08-12 0.1.2.8
特性：
1. 将ReleaseNotes.txt改为ReleaseNotes.md；
2. 在没有config.json的情况下自动生成config.json；
3. 在项目中移除config.ini；
4. 修改了项目名称为CNELaser；
5. 填充了在windows操作系统下，应用程序的属性细节；
6. 在windows下添加了图标。

Features:
1. Rename ReleaseNotes.txt to ReleaseNotes.md;
2. Auto generate config.json if there's no one;
3. Remove config.ini from current project;
4. Rename project name to CNELaser;
5. Fill in details of .exe at Windows OS;
6. Add icon at Windows OS.

#### 2021-08-11 0.1.2.7
1. 实现了从CAD中导入Nurbs曲线；
2. 实现了从CAD中导入图片；
3. 初步实现了撤消功能。

#### 2021-07-30 0.1.2.6
1. 修复了在导出JSON时图元变换错误的问题；
2. 路径优化按钮，点击后打开配置窗口直接显示路径优化相关的选项卡；
3. 添加了Dxf文件导入功能，正在进一步实现更多图元的导入；
4. 导入文件不再根据不同按钮区分导入文件的格式，而是根据导入文件的后缀自动区分。

#### 2021-07-16 0.1.2.5
1. 实现了选项窗口中选项的中英文切换；
2. 实现了主界面窗口中滑块控件与寄存器间的绑定，还需等支持库进一步处理寄存器后再进行修改；
3. 修改了一系列的bug；
4. 重新排版了所有的浮动面板。

#### 2021-07-15 0.1.2.4
1. 修复了多处图元操作bug；
2. 添加了图元按线选取的功能；
3. 创建了双滑块控件；
4. 创建了双可编辑滑块控件。

#### 2021-07-14 0.1.2.3
1. 实现双控制柄的DualSlider控件；
2. 修正了多处图元操作bug；

#### 2020-11-17 0.1.1.58
1. 实现了标尺显示功能；
2. 实现了左下角的缩放控件功能；
3. 处理了其它bug。

#### 2020-11-17 0.1.1.57
1. 当CDR未安装时，正常提示错误信息;
2. 添加了移除图层按钮。

#### 2020-11-17 0.1.1.56
1. 去掉了从CDR导入时的导入对话框，仅保留CDR的导出对话框；
2. 当用户从CDR导入时，如果用户关闭了CDR的导出对话框，则中断导入过程；
3. 处理了其它BUG。

#### 2020-11-17 0.1.1.55
1. 添加了普通参数配置文件及相应的窗口；
2. 修改了导入CDR时的部分显示问题；
3. 修改了文档关闭时图层列表没有关闭的问题。

#### 2020-11-17 0.1.1.54
1. 实现升降;
2. 修改加工原点控件的取值范围;
3. 连接/断开按钮的状态不正确;
4. 添加了。

#### 2020-11-17 0.1.1.53
1. 添加了左侧创建图元的部分功能；
2. 实现了“移动面板”；
3. 实现了“操作面板”;
4. 实现了FinishRun的功能。

#### 2020-11-17 0.1.1.52
1. 错误密码隐藏厂家参数面板；
2. 加了部分图标。隐藏了工具面板。合并了部分面板。

#### 2020-11-17 0.1.1.51
1. 初步处理了加工、暂停和停止按钮的状态。

#### 2020-10-28 0.1.1.50
1. 添加了新的寄存器参数修改窗口；
2. 寄存器参数分类显示；
3. 三家寄存器参数默认隐藏，提供密码正确后显示；
4. 添加了修改密码对话框。

#### 2020-10-28 0.1.1.48
1. 处理了圆弧不够圆的问题，添加了点间距离阈值；
2. 反转了二值图中的0和1。

#### 2020-10-28 0.1.1.47
1. 添加了MSVCP140.dll、VCRUNTIME140.dll、VCRUNTIME140_1.dll和CONCRT140.dll；
2. 在安装包中放入了extra文件夹，该文件夹下有vc分发包；
3. 解决了图层面板中最大值最小值不正确的问题。

#### 2020-10-28 0.1.1.46
1. 修复了鼠标框选时选框在画布后的问题；
2. 修改了挂网算法。

#### 2020-10-23 0.1.1.44
1. 修正导出JSON时崩溃的BUG。但win7依然有问题。
2. 输出的位图部分数据，使用新的格式约定，需要进一步测试。

#### 2020-10-21 0.1.1.43
1. 修正了cdr无法导出svg的bug。

#### 2020-10-16 0.1.1.42
1. 将QT默认的big-endian改为delphi可用的little-endian。
2. 将字节输出改为二值位输出。

#### 2020-10-14 0.1.1.41
1. 修正导出JSON时崩溃的BUG。

#### 2020-10-13	0.1.1.40
1. 位图改为来回输出。

#### 2020-10-13  0.1.1.39
1. 使用Ctrl+滚轮进行缩放；
2. 在雕刻图层中输出的位图改为了使用新的按行格式输出。