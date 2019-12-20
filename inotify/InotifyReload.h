#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <functional>

#include <sys/select.h>

using reloadFn =  std::function<void(const std::string)>;

class InotifyReload{
    struct ReloadData {
        std::string file_name;
        reloadFn fn;
    };
public:
    static InotifyReload* instance() {
        static InotifyReload ins;
        return &ins;
    }

    int Open();
    int Close();

    int Add(const std::string& file, const reloadFn& fn = nullptr);
    std::string GetContent(const std::string& file_name);

protected:
    InotifyReload():inotify_fd(-1), m_isrunning(false){}
    ~InotifyReload(){}

private:
    void RunWatch();
    int readEvents(std::vector<struct inotify_event>& events);
    int handleEvents(const std::vector<struct inotify_event>& events);
    std::string loadFile(const std::string& file_name);

private:
    int inotify_fd;
    std::mutex m_mutex, con_mutex; 
    std::unordered_map<int, ReloadData> m_map;
    std::unordered_map<std::string, std::string> content_map;

    fd_set fds;
    bool m_isrunning;
};
