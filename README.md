# unix网络编程 卷2: 进程间通信
[书里源码下载链接](http://www.ituring.com.cn/book/download/65a4aa23-a459-41a6-bc56-e0f787a7d83f)
## 配置运行(本人环境ubuntu16.04)
1. 进入根目录(即configure所在目录)
2. 赋予权限: `chmod a+x config.sub configure`
3. 编译: `./configure ubuntu`
4. 编译库: `cd lib && make`
5. 编译程序: `cd ../pipe && make` 生成多个可执行文件,对应其源文件. 其它文件夹代码也是直接进入文件夹直接`make`就好
