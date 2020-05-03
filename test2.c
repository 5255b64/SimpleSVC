#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "svc.h"
#include "testcases.h"

int test2() {
    void *helper = svc_init();
    assert(helper != NULL);
//    Return value: helper
    int temp_int;
    char *temp_char = NULL;

    FILE *fp = NULL;

    // 写入COMP2017/svc.c
    // 1) hash=5007
    fp = fopen("COMP2017/svc.c", "w");
    fprintf(fp, "#include \"svc.h\"\n");
    fprintf(fp, "void *svc_init(void) {\n");
    fprintf(fp, "   // TODO : implement\n");
    fprintf(fp, "}\n");
    fclose(fp);
    // 2)hash = 4798
//    fp = fopen("COMP2017/svc.c", "w");
//    fprintf(fp, "#include \"svc.h\"\n");
//    fprintf(fp, "void *svc_init(void) {\n");
//    fprintf(fp, "    return NULL;\n");
//    fprintf(fp, "}\n");
//    fclose(fp);

    // 写入COMP2017/svc.h
    fp = fopen("COMP2017/svc.h", "w");
    fprintf(fp, "#ifndef svc_h\n");
    fprintf(fp, "#define svc_h\n");
    fprintf(fp, "void *svc_init(void);\n");
    fprintf(fp, "#endif\n");
    fclose(fp);

    // 1）加入文件"COMP2017/svc.h"
    temp_int = svc_add(helper, "COMP2017/svc.h");
//    printf("%d\n", temp_int);
    assert(temp_int == 5007);
//    Return value: 5007

    // 2）加入文件"COMP2017/svc.c"
    temp_int = svc_add(helper, "COMP2017/svc.c");
//    printf("%d\n", temp_int);
    assert(temp_int == 5217);
//    Return value: 5217

    // 3）commit "Initial commit"
    temp_char = svc_commit(helper, "Initial commit");
    printf("%s\n", temp_char);
    assert(strcmp(temp_char, "7b3e30") == 0);
//    Return value: "7b3e30"

    // 4）print
    print_commit(helper, "7b3e30");

    // 5）branch "random_branch"
    temp_int = svc_branch(helper, "random_branch");
    assert(temp_int == 0);
//    Return value: 0

    // 6）checkout "random_branch"
    temp_int = svc_checkout(helper, "random_branch");
    assert(temp_int == 0);
//    Return value: 0

//    Next, the file COMP2017/svc.c is changed to have the following contents
//    # include " svc . h " \ n
//    void * svc_init ( void ) {\ n
//        return NULL ;\ n
//    }\ n
//    which has the hash 4798.
    // 7）修改文件"COMP2017/svc.c"
    fp = fopen("COMP2017/svc.c", "w");
    fprintf(fp, "#include \"svc.h\"\n");
    fprintf(fp, "void *svc_init(void) {\n");
    fprintf(fp, "    return NULL;\n");
    fprintf(fp, "}\n");
    fclose(fp);

    // 8）删除文件"COMP2017/svc.h"
    temp_int = svc_rm(helper, "COMP2017/svc.h");
    printf("%d\n", temp_int);
    assert(temp_int == 5007);
//    Return value: 5007

    // 9）commit "Implemented svc_init"
    temp_char = svc_commit(helper, "Implemented svc_init");
    assert(temp_char != NULL);
    printf("%s\n", temp_char);
    assert(strcmp(temp_char, "73eacd") == 0);
//    Return value: "73eacd"

    // 10）reset "7b3e30" "Initial commit"(3)
//    You realise you accidentally deleted COMP2017/svc.h and want to revert to the initial commit
    temp_int = svc_reset(helper, "7b3e30");
    assert(temp_int == 0);
//    Return value: 0
//    Then, the file COMP2017/svc.c is changed again to have the contents shown above.
    // 10-2）修改文件"COMP2017/svc.c"
    fp = fopen("COMP2017/svc.c", "w");
    fprintf(fp, "#include \"svc.h\"\n");
    fprintf(fp, "void *svc_init(void) {\n");
    fprintf(fp, "    return NULL;\n");
    fprintf(fp, "}\n");
    fclose(fp);

    // 11）commit "Implemented svc_init"
    temp_char = svc_commit(helper, "Implemented svc_init");
    assert(temp_char != NULL);
    printf("%s\n", temp_char);
    assert(strcmp(temp_char, "24829b") == 0);
//    Return value: "24829b"


    // 12）get_commit "24829b" "Implemented svc_init"(11)
    void *commit = get_commit(helper, "24829b");
    assert(strcmp(((commit_struct *) commit)->commit_id, "24829b") == 0);
//    Return value: Pointer to area of memory containing the commit created above

    // 13）get_prev_commits
    int n_prev;
    char **prev_commits = get_prev_commits(helper, commit, &n_prev);
    assert(strcmp(*prev_commits, "7b3e30") == 0);
    assert(n_prev == 1);
//    Return value: Pointer to an array of length one, containing "7b3e30"
//    Afterwards, n_prev = 1

    // 14）svc_checkout "master"
    temp_int = svc_checkout(helper, "master");
    assert(temp_int == 0);
//    Return value: 0

//    The test framework creates a file resolutions/svc.c with the contents
//      # include " svc . h " \ n
//      void * svc_init ( void ){\ n
//          return NULL ;\ n
//      }\ n
    // 15）新建文件"resolutions/svc.c"
    fp = fopen("resolutions/svc.c", "w");
    fprintf(fp, "#include \"svc.h\"\n");
    fprintf(fp, "void *svc_init(void) {\n");
    fprintf(fp, "    return NULL;\n");
    fprintf(fp, "}\n");
    fclose(fp);

//        The following code is then executed to perform a merge:
//    Resolution ( s ) are created by the test framework
    // 16）svc_merge
    resolution *resolutions = malloc(sizeof(resolution));
    resolutions[0].file_name = "COMP2017/svc.c";
    resolutions[0].resolved_file = "resolutions/svc.c ";
    // Call to merge function
    temp_char = svc_merge(helper, "random_branch", resolutions, 1);
    assert(temp_char!=NULL);
    assert(strcmp(temp_char, "48eac3") == 0);
    // The test framework will free the memory
    free(resolutions);
//    Return value: "48eac3"

//    The commit message is Merged branch random_branch. The conflicts array indicates that only
//    the change is to be kept, following the merge rules described above. This means that from the
//    perspective of the master branch, the only change to be committed is this modification.
    // 17）get_commit （16）
    commit = get_commit(helper, "48eac3");
    prev_commits = get_prev_commits(helper, commit, &n_prev);
//    Return value: Pointer to an array of length two, containing "7b3e30" and "24829b" in that
//    order. Afterwards, n_prev = 2
    assert(strcmp(*prev_commits, "7b3e30") == 0);
    assert(strcmp(*(prev_commits + 1), "24829b") == 0);
    assert(n_prev == 2);


    cleanup(helper);
    return 1;
}

