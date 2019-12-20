## Inotify

inotify是linux提供的一个监控文件事件的机制，当文件被访问，被修改或者被关闭时，它都可以通知我们。而且它可以同时监控多个文件。

### 数据结构

当inotify fd上有可读的事件，调用read函数会得到如下结构体：
```
struct inotify_event {
    int wd; //watch_id
    int mask;
    int cookie; //
    int len; //len of name
    char name[]; //optional
};
```
理论上name是监控的文件名，但是这个字段是可选字段，而一般情况下并不会为这个域赋值，对应的len的值通常也为零。所以从read的返回值中获取文件名不靠谱，最好是自己保存一个watch_id 到file_name的映射关系。

mask取值如下：
```
IN_ACCESS (+)
        File was accessed (e.g., read(2), execve(2)).
IN_ATTRIB (*)
       Metadata changed—for example, permissions (e.g.,
       chmod(2)), timestamps (e.g., utimensat(2)), extended
       attributes (setxattr(2)), link count (since Linux 2.6.25;
       e.g., for the target of link(2) and for unlink(2)), and
       user/group ID (e.g., chown(2)).
IN_CLOSE_WRITE (+)
       File opened for writing was closed.
IN_CLOSE_NOWRITE (*)
       File or directory not opened for writing was closed.
IN_CREATE (+)
       File/directory created in watched directory (e.g., open(2)
       O_CREAT, mkdir(2), link(2), symlink(2), bind(2) on a UNIX
       domain socket).
IN_DELETE (+)
       File/directory deleted from watched directory.
IN_DELETE_SELF
       Watched file/directory was itself deleted.  (This event
       also occurs if an object is moved to another filesystem,
       since mv(1) in effect copies the file to the other
       filesystem and then deletes it from the original filesys‐
       tem.)  In addition, an IN_IGNORED event will subsequently
       be generated for the watch descriptor.
IN_MODIFY (+)
       File was modified (e.g., write(2), truncate(2)).
IN_MOVE_SELF
       Watched file/directory was itself moved.
IN_MOVED_FROM (+)
       Generated for the directory containing the old filename
       when a file is renamed.
IN_MOVED_TO (+)
       Generated for the directory containing the new filename
       when a file is renamed.
IN_OPEN (*)
       File or directory was opened.
```

这些mask可以通过`|`操作符来批量添加，个人感觉 IN_MODIFY 比较实用

### 核心函数

```cpp
int inotify_init() //返回一个指向inotify实例的fd，后续可通过监听该fd的可读写事件来知道文件的变化
int inotify_init1(int flags) //和inotify_init类似，不过可以为创建的fd传递一些 flag，当它为0时，效果和 inotify_init 一致。可以设置的flag有：IN_NONBLOCK和IN_CLOEXEC
int inotify_add_watch(int fd, const char *pathname, uint32_t mask) //向inotify_fd添加要监控的文件以及事件，返回一个watch_id。
int inotify_rm_watch(int fd, int wd) //取消监控
```
### 使用方式

1. 使用inotify_init()初始化一个inotify实例，并保存返回的fd
2. 调用inotify_add_watch()函数为inotify实例添加要监控的文件
3. 使用select/pool/epoll等监听 inotify fd
4. 当有对应事件可读时，调用read函数，获取这些就绪的inotify_event 
5. 根据mask来判断发生了哪些事件，并进行对应的处理

### 参考链接
[用 inotify 监控 Linux 文件系统事件](https://www.ibm.com/developerworks/cn/linux/l-inotify/index.html)

[http://man7.org/linux/man-pages/man7/inotify.7.html](http://man7.org/linux/man-pages/man7/inotify.7.html)
