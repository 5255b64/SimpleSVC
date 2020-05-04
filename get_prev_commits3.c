#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "svc.h"
#include "testcases.h"

int get_prev_commits3() {
    // 写入hello.py
    // hash=2027
    FILE *fp = fopen("hello.py", "w");
    fprintf(fp, "print(\"Hello\")\n");
    fclose(fp);
    // 写入view.sh
    // hash=2027
    fp = fopen("view.sh", "w");
    fprintf(fp, "#!/usr/bin/env bash\n");
    fprintf(fp, "\n");
    fprintf(fp, "hugo --i18n-warnings server\n");
    fclose(fp);


    void *helper = svc_init();

    char *temp_str;

    // 1)add
    temp_str = deepcopy_str("hello.py");
    svc_add(helper,temp_str);
    assert(hash_file(helper, temp_str) == 2027);
    free(temp_str);

    // 2)add
    temp_str = deepcopy_str("view.sh");
    svc_add(helper,temp_str);
    int hash = hash_file(helper, temp_str);
    assert(hash == 4871);
    free(temp_str);

    // 3)commit
    temp_str = deepcopy_str("Initial commit");
    char *commit_id = svc_commit(helper, temp_str);
    free(temp_str);
    assert(strcmp(commit_id, "2e64a9")==0);

    // 4)add
    temp_str = deepcopy_str("hello.py");
    svc_add(helper,temp_str);
    assert(hash_file(helper, temp_str) == 2027);
    free(temp_str);

    // 5)commit
    temp_str = deepcopy_str("Changed hello to hi");
    commit_id = svc_commit(helper, temp_str);
    free(temp_str);
    assert(commit_id!=NULL);
    assert(strcmp(commit_id, "855a49")==0);

    // 6)hash
    temp_str = deepcopy_str("view.sh");
    assert(hash_file(helper, temp_str) == 4871);
    free(temp_str);

    // 7)rm
    temp_str = deepcopy_str("view.sh");
    svc_rm(helper,temp_str);
    assert(hash_file(helper, temp_str) == 4871);
    free(temp_str);

    // 8)commit
    temp_str = deepcopy_str("More complex hello and removed viewer");
    commit_id = svc_commit(helper, temp_str);
    free(temp_str);
    assert(strcmp(commit_id, "855a49")==0);



    cleanup(helper);
    return 1;
}