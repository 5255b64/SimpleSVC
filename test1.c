#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "svc.h"
#include "testcases.h"

int test1() {
    void *helper = svc_init();
    cleanup(helper);
    helper = svc_init();
    assert(helper!=NULL);
//    Return value: helper

    // 写入hello.py
    // hash=2027
    FILE *fp = fopen("hello.py", "w");
    fprintf(fp, "print(\"Hello\")\n");
    fclose(fp);

    // 写入sample.txt
    // hash=1178
    fp = fopen("sample.txt", "w");
    fprintf(fp, "Hello, world\n");
    fclose(fp);

    // 写入Tests/diff.txt
    // hash=385
    fp = fopen("Tests/diff.txt", "w");
    fclose(fp);

    // 写入Tests/test1.in
    // hash=564
    fp = fopen("Tests/test1.in", "w");
    fprintf(fp, "5 3 2\n");
    fclose(fp);

//    char *tmp_char = NULL;
    assert(hash_file(helper, "hello.py")==2027);
//    Return value: 2027 (from example above)
//    assert(hash_file(helper, "sample.txt")==1178);
//    assert(hash_file(helper, "Tests/diff.txt")==385);
//    assert(hash_file(helper, "Tests/test1.in")==564);

    assert(hash_file(helper, "fake.c")==-2);
//    Return value: -2 (non-existent file_list)

    assert(svc_commit(helper, "No changes")==NULL);
//    Return value: NULL
    char *char_2_free;
    char_2_free=deepcopy_str("hello.py");
    assert(svc_add(helper, char_2_free)==2027);
    free(char_2_free);
//    Return value: 2027
//    print_file(helper);

    char_2_free=deepcopy_str("Tests/test1.in");
    assert(svc_add(helper, "Tests/test1.in")==564);
    free(char_2_free);
//    Return value: 564 (from example above)
//    print_file(helper);

    char_2_free=deepcopy_str("Tests/test1.in");
    assert(svc_add(helper, "Tests/test1.in")==-2);
    free(char_2_free);
//    Return value: -2
//    print_file(helper);
//    remove("hello.py");   // TODO delete
    char_2_free=deepcopy_str("Initial commit");
    char *commit_id = svc_commit(helper, char_2_free);
    free(char_2_free);
    assert(commit_id!=NULL);
    assert(strcmp(commit_id, "74cde7")==0);
//    Return value: "74cde7"

    void *commit = get_commit(helper, commit_id);
    assert(strcmp(((commit_struct*)commit)->commit_id, "74cde7")==0);
//    Return value: Pointer to area of memory containing the commit_struct created above

    int n_prev;
    char **prev_commits = get_prev_commits(helper, commit, &n_prev);
    assert(prev_commits==NULL);
    assert(n_prev==0);
    for(int i=0;i<n_prev;i++){
        printf("prev_commits[%d]=%s\n",i, prev_commits[i]);
        free(prev_commits[i]);
    }
    free(prev_commits);

    char_2_free=deepcopy_str("74cde7");
    print_commit(helper, char_2_free);
    free(char_2_free);

    // TODO delete
//    int tmp_int = svc_branch(helper, "newBranch");
//    printf("svc_branch=%d\n", tmp_int);
//    svc_rm(helper, "hello.py");
//    char *commit_id_1 = svc_commit(helper, "Test commit");
//    printf("commit_id=%s\n", commit_id_1);
//
//    tmp_int = svc_checkout(helper, "newBranch");
//    printf("svc_checkout=%d\n", tmp_int);
//    svc_rm(helper, "Tests/test1.in");
//    char *commit_id_2 = svc_commit(helper, "Test commit");
//    printf("commit_id=%s\n", commit_id_2);
//
//    print_commit(helper, commit_id_2);

    int n;
    char **branches = list_branches(helper, &n);
    assert(branches!=NULL);
//    assert(strcmp((char*)branches, "master")==0);
//    assert(n==1);
    if(branches!=NULL) {
        for (int i = 0; i < n; i++) {
            printf("%s\n",branches[i]);
//            free(branches[i]);
        }
        free(branches);
    }
//    Output: master

    cleanup(helper);
    return 1;
}

