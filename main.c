#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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

    // 判断文件存在
//    if (!access("/run/media/gx/0226E34626E338F5/GX/Study/HW/2020/4_30/TestDir/123", 0)) {
//        printf("file_list exist\n");
//    } else {
//        printf("file_list not exist\n");
//    }

//    char *a="123";
//    char *b ="122";
//    printf("%d\n", strcmp(a,b));
//    printf("%d\n", diy_strcmp("ab","abcd"));
//    printf("%d\n", diy_strcmp("ab","AB"));
//    printf("%d\n", diy_strcmp("ab","ABc"));
//    printf("%d\n", diy_strcmp("ab","CD"));
//    printf("%d\n", diy_strcmp("cd","AB"));

    // 执行测试
//    void *helper = svc_init();
//    svc_add(helper, "Tests/test1.in");
//    svc_add(helper, "sample.txt");
//    svc_add(helper, "hello.py");
//    cleanup(helper);

// 执行测试
    test1();
    test2();

    return 0;
}
