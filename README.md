## RT-Thread W60x VSCode Template

用vscode开发RT-Thread的模板（W60x）

* 能开发无数个项目，而不需要每个都复制一份RT-Thread内核

# 为首次运行做准备
1. 系统依赖：
	```
	dnf install -y python3 scons
	```
1. 安装python依赖
	```
	pip install -r requirements.txt --user
	```
1. 安装并配置GCC
	* 在[这里](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)下载，然后解压缩
1. 下载RT-Thread内核
	* 从[GitHub](https://github.com/RT-Thread/rt-thread)或[码云](https://gitee.com/rtthread/rt-thread)，git clone或下载zip包都可以
1. 下载Env工具
	* Linux: 命令打开当前项目目录，行运行`scons --menuconfig`（如果失败，不是网络问题就不用管）
	* Windows: 从[官网](https://www.rt-thread.org/page/download.html)下载
1. 安装VSCode扩展
   * C/C++

# 开始项目
复制这个项目模板，打开其中的`.vscode/settings.json`，修改以下配置：
* `RTT_EXEC_PATH`: 指向GCC文件夹的bin目录
* `RTT_ROOT`: 指向下载的RT-Thread代码根目录
* `ENV_ROOT`: 指向下载的Env工具根目录，不可以用HOME、~之类的环境变量，必须写全 *（比如/home/xxx/.env）*

# 使用

*常用工具在Ctrl+R菜单里*

* `python serialport.py term [port number]`: 启动串口终端，端口号只写数字，比如COM11就是“11”
* `python serialport.py flash [port number]`: 刷写
* `scons`: 编译
* `scons --menuconfig`: 修改编译配置
* `pkgs --update`: 更新依赖包 *（linux需要先运行`source ~/.env/env.sh`）*
