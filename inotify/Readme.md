## Inotify

inotify是linux提供的一个监控文件事件的机制，当文件被访问，被修改或者被关闭时，它都可以通知我们。而且它可以同时监控多个文件。

核心函数如下：

```cpp
int inotify_init() //返回一个指向inotify实例的fd，后续可通过监听该fd的可读写事件来知道文件的变化
int inotify_init1(int flags) //和inotify_init类似，不过可以为创建的fd传递一些 flag，当它为0时，效果和 inotify_init 一致。可以设置的flag有：IN_NONBLOCK和IN_CLOEXEC
int inotify_add_watch(int fd, const char *pathname, uint32_t mask) //向inotify_fd添加要监控的文件以及事件，返回一个watch_id
int inotify_rm_watch(int fd, int wd) //取消监控
```
