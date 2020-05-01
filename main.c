#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "testcases.h"
#include "svc.h"

int main() {
    // 输出工作路径
    char buf[1024];
    char *cwd = getcwd(buf, sizeof(buf));
    if (NULL == cwd) {
        perror("Get cerrent working directory fail.\n");
        exit(-1);
    } else {
        printf("Current working directory is : %s\n", cwd);
    }
    //

//    if (!access("/run/media/gx/0226E34626E338F5/GX/Study/HW/2020/4_30/TestDir/123", 0)) {
//        printf("file exist\n");
//    } else {
//        printf("file not exist\n");
//    }


// 执行测试
    void *helper = svc_init();
    hash_file(helper, "hello.py");
    cleanup(helper);

// 执行测试
    //test1();

    return 0;
}