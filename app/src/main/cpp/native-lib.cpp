#include <jni.h>
#include <string>
#include <unistd.h>
#include "native-lib.h"

const char *PATH = "data/data/com.qbase.guardserver/my.sock";
int m_child;
const char *userId;


//做守护进程动作
void child_do_work() {
    //开启socket  服务端
    //第一步  等待连接
    if (child_create_channel()) {
        child_listen_msg();
    }


    //第二步 读取消息

}

/**创建服务端的socket*/
void child_listen_msg() {
    fd_set rfds;
    struct timeval timeout = {3, 0};
    while (1) {
        FD_ZERO(&rfds);
        FD_SET(m_child, &rfds);
        //监视最大文件数+1   客户端+1
        int r = select(m_child + 1, &rfds, NULL, NULL, &timeout);
        LOGE("读取消息前  %d", r);
        if (r > 0) {
            //缓冲区
            char pkg[256] = {0};
            //保证所读取到的消息是指定客户端
            if (FD_ISSET(m_child, &rfds)) {
                //阻塞式函数  什么都不读  此在子进程中
                int result = read(m_child, pkg, sizeof(pkg));
                LOGE("--userId %d",userId);
                //开启服务
                execlp("am", "am", "startservice", "--user", userId,
                       "com.qbase.guardserver/com.qbase.guardserver.ProcessService", (char *) NULL);
                break;
            }
        }
    }

}

/**
 * 服务端读取信息
 * 客户端：宿主app
 * */
int child_create_channel() {
    //协议、类型、
    int listenfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    unlink(PATH);
    // addr ---> 内存区域不为 0 的
    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(sockaddr_un));
    addr.sun_family = AF_LOCAL;

    int connfd = 0;

    strcpy(addr.sun_path, PATH);
    if (bind(listenfd, (const sockaddr *) &addr, sizeof(sockaddr_un)) < 0) {
        LOGE("绑定错误");
        return 0;
    }

    //监听app个数
    listen(listenfd, 5);

    //保证宿主进程链接成功,失败一直链接
    while (1) {
        //返回值为客户端的地址,阻塞式函数
        if (connfd = accept(listenfd, NULL, NULL) < 0) {
            if (errno == EINTR) {
                //链接成功
            } else {
                LOGE("读取错误");
                return 0;
            }
        }
        m_child = connfd;

        LOGE("apk 父进程连接上了   %d " ,m_child);
        break;
    }

    return 1;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_qbase_guardserver_Watcher_createWatcher(JNIEnv *env, jobject instance, jstring userId_) {
    userId = env->GetStringUTFChars(userId_, 0);

    //如何开启双进程，Linux有开启双进程的方法
    pid_t pid = fork();
    if (pid < 0) {
        //失败
    } else if (pid == 0) {
        //子进程
        child_do_work();

    } else if (pid > 0) {
        //父进程
    }


    env->ReleaseStringUTFChars(userId_, userId);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_qbase_guardserver_Watcher_connectMonitor(JNIEnv *env, jobject instance) {

    // 子进程 1 apk 2
    int socked;
    while (1){
        LOGE("客户端 父进程开始链接");
        socked = socket(AF_LOCAL,SOCK_STREAM,0);
        if(socked<0){
            LOGE("链接失败");
            return;
        }

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(sockaddr));
        addr.sun_family = AF_LOCAL;
        strcpy(addr.sun_path, PATH);

        if(connect(socked, (const sockaddr *)(&addr), sizeof(sockaddr_un)) < 0){
            LOGE("链接失败");
            close(socked);
            sleep(1);
            //休眠1s后重新链接，直到链接成功
            continue;
        }
        LOGE("链接成功");
        break;
    }

}