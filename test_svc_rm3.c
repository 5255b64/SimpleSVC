#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "svc.h"
#include "testcases.h"

int test_svc_rm3() {
    void *helper = svc_init();

    int temp_int;
//    char *temp_char = NULL;

    // 写入hello.py
    // hash=2027
    FILE *fp = fopen("hello.py", "w");
    fprintf(fp, "print(\"Hello\")\n");
    fclose(fp);


//    temp_int = hash_file(helper, "hello.py");
    temp_int = svc_add(helper, "hello.py");
    assert(temp_int == 2027);

    temp_int = svc_rm(helper, "hello.py");
    assert(temp_int == 2027);


    svc_add(helper, "hello.py");


    cleanup(helper);
    return 1;
}