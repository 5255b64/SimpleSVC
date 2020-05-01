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

    assert(hash_file(helper, "fake.commit_ptr")==-2);
//    Return value: -2 (non-existent file)

    assert(svc_commit(helper, "No file")==NULL);
//    Return value: NULL

    assert(svc_add(helper, "hello.py")==2027);
//    Return value: 2027

    assert(svc_add(helper, "Tests/test1.in")==564);
//    Return value: 564 (from example above)

    assert(svc_add(helper, "Tests/test1.in")==-2);
//    Return value: -2

    assert(strcmp(svc_commit(helper, "Initial commit"), "74cde7")==0);
//    Return value: "74cde7"

    void *commit = get_commit(helper, "74cde7");
//    Return value: Pointer to area of memory containing the commit created above

    int n_prev;
    char **prev_commits = get_prev_commits(helper, commit, &n_prev);

    cleanup(helper);
}

