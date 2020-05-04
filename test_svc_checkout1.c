#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "svc.h"
#include "testcases.h"

int test_svc_checkout1() {
    // 写入hello.py
    // hash=2027
    FILE *fp = fopen("hello.py", "w");
    fprintf(fp, "print(\"Hello\")\n");
    fclose(fp);

    void *helper = svc_init();

    char *temp_str;

    // 1)add
    temp_str = deepcopy_str("hello.py");
    svc_add(helper,temp_str);
    assert(hash_file(helper, temp_str) == 2027);
    free(temp_str);

    // 2)commit
    temp_str = deepcopy_str("Initial commit");
    char *commit_id = svc_commit(helper, temp_str);
    free(temp_str);
    assert(strcmp(commit_id, "00006f")==0);

    // 3）branch
    temp_str = deepcopy_str("Initial commit");
    int temp_int = svc_branch(helper, "test");
    free(temp_str);
    printf("%d\n",temp_int);

    // 4）checkout
    temp_str = deepcopy_str("Initial commit");
    temp_int = svc_checkout(helper, "test");
    free(temp_str);
    printf("%d\n",temp_int);


    cleanup(helper);
    return 1;
}