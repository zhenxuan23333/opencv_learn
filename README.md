# opencv_learn
-_-
启动一个节点
ros run <package_name> <executable_name>

ros2的命令行
CLI（命令行界面）
GUI（图形用户界面）

1.运行节点
ros2 run 

2.列出节点
ros2 node list

3.展示节点信息
ros2 node ifo <node_name>

4.节点名称重映射
ros2 run turtlesim turtlesim_node --ros-args --remap --node:my_turtle

ROS2工作空间介绍
一个工作空间下有多个功能包
一个功能包包含多个节点

功能包
ament_python
cmake
ament_cmake

功能包获取方式
安装获取
手动便以获取

与功能包有关的指令

创建功能包
ros pkg create <package-name> --build-type {cmake, ament_cmake, ament_python} --dependencies <依赖名字>

列出可执行文件
列出所有
ros2 pkg executables
列出某个功能包的
ros2 pkg executables turtlesim
列出所有的包的
ros2 pkg list
输出某个包所在路径的前缀
ros2 pkg prefix <package_name>

列出包的清单描述文件
每个包都有一个标配的mainifest.xml文件，用于记录这个包的信息
ros2 pkg xml turtlesim

ROS2构建工具
colcon

功能包构建工具（编译工具）
colcon build

只编译一个功能包
colcon test --packages-select YOUR_PKG_NAME

不编译测试单元
colcon build --packages-select YOUR_PKG_NAME  --cmake-args -DBUILD_TESTING=0

运行编译包的测试
colcon test

允许通过更改src下的部分文件（是src源代码与编译后的文件同步便于修改代码、热编译？）
colcon build --symlink-install//将代码从src链接到install

创建Python功能包和节点

创建工作空间，并在工作空间中创建功能包
1.创建工作空间
2.创建功能包
    ros2 pkg create village_li --build-typeament_python --dependencies rclpy
    
    pkg creat//创建功能包
    --build-type//制定该包的编译类型 有ament_python, ament_cmake, cmake, 若为空, 默认为cmake
    --dependencies//指这个功能包的依赖
