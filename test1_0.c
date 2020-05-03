#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "svc.h"
#include "testcases.h"

int test1() {
    void *helper = svc_init();
//    Return value: helper

    svc_commit(helper, "No file_list");
//    Return value: NULL

    printf("%d\n", svc_add(helper, "hello.py"));
//    Return value: 2027
//    print_file(helper);

    printf("%d\n", svc_add(helper, "Tests/test1.in"));
//    Return value: 564 (from example above)
//    print_file(helper);

    printf("%d\n", svc_add(helper, "Tests/test1.in"));
//    Return value: -2
//    print_file(helper);
    printf("%s\n", svc_commit(helper, "Initial commit_struct"));
//    Return value: "74cde7"

    void *commit = get_commit(helper, "74cde7");
//    Return value: Pointer to area of memory containing the commit_struct created above

    int n_prev;
    char **prev_commits = get_prev_commits(helper, commit, &n_prev);
    for(int i=0;i<n_prev;i++){
        printf("prev_commits[%d]=%s\n",i, prev_commits[i]);
        free(prev_commits[i]);
    }
    free(prev_commits);

    print_commit(helper, "74cde7");

    int n;
    char **branches = list_branches(helper, &n);
    for(int i=0;i<n;i++){
        free(branches[i]);
    }
    free(branches);
//    Output: master

    cleanup(helper);
    return 1;
}

