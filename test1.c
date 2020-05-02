#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "svc.h"
#include "testcases.h"

int test1() {
    void *helper = svc_init();
    assert(helper!=NULL);
//    Return value: helper

    assert(hash_file(helper, "hello.py")==2027);
//    Return value: 2027 (from example above)
    assert(hash_file(helper, "sample.txt")==1178);
    assert(hash_file(helper, "Tests/diff.txt")==385);
    assert(hash_file(helper, "Tests/test1.in")==564);

    assert(hash_file(helper, "fake.commit_pre")==-2);
//    Return value: -2 (non-existent file_list)

    assert(svc_commit(helper, "No file_list")==NULL);
//    Return value: NULL

    assert(svc_add(helper, "hello.py")==2027);
//    Return value: 2027
//    print_file(helper);

    assert(svc_add(helper, "Tests/test1.in")==564);
//    Return value: 564 (from example above)
//    print_file(helper);

    assert(svc_add(helper, "Tests/test1.in")==-2);
//    Return value: -2
//    print_file(helper);

    assert(strcmp(svc_commit(helper, "Initial commit_struct"), "74cde7")==0);
//    Return value: "74cde7"

    void *commit = get_commit(helper, "74cde7");
    assert(strcmp(((commit_struct*)commit)->commit_id, "74cde7")==0);
//    Return value: Pointer to area of memory containing the commit_struct created above

    int n_prev;
    char **prev_commits = get_prev_commits(helper, commit, &n_prev);
    assert(prev_commits==NULL);
    assert(n_prev==0);

    print_commit(helper, "74cde7");

    int n;
    char **branches = list_branches(helper, &n);
    assert(branches!=NULL);
    assert(strcmp(*branches, "master")==0);
    assert(n==1);
//    Output: master

    cleanup(helper);
    return 1;
}

