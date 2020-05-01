#include "svc.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void *svc_init(void) {
    helper_struct *helper = (helper_struct *) malloc(sizeof(helper_struct));
    helper->checkout_num = 1;
    helper->commit_num = 0;
    helper->file_num = 0;
    // checkout
    // 默认生成master branch
    // 工作区内容在checkout中修改
    helper->init_checkout = (checkout *) malloc(sizeof(checkout));
    helper->init_checkout->branch_name = "master";
    helper->init_checkout->next = NULL;
    helper->init_checkout->index = (index_struct *) malloc(sizeof(index_struct));
    helper->init_checkout->index->file_num=0;
    // commit
    // 初始没有commit
    helper->init_commit = NULL;

    // file
    helper->init_file=NULL;

    // head_checkout
    helper->head_checkout = helper->init_checkout;

    return helper;
}

void clean_commit(commit *c) {
    // clean children
    commit **children = c->child_list;
    if (children != NULL) {
        for (int i = 0; i < c->child_num; i++) {
            clean_commit(children[i]);
        }
    }
    free(c->child_list);

    free(c);
}

void cleanup(void *helper) {
    helper_struct *h = (helper_struct *) helper;
    if (h != NULL) {
        // commit
        commit *c = h->init_commit;
        if (c != NULL) {
            clean_commit(c);
        }

        // checkout
        checkout *co = h->init_checkout;
        checkout *co_next = NULL;
        while (co != NULL) {
            co_next = co->next;
            index_struct *index = co->index;
            for (int i = 0; i < index->file_num; i++) {
                free(index->file_message + i);
            }
            free(index);
            free(co);
            co = co_next;
        }

        // file
        file_struct *f = h->init_file;
        file_struct *f_next = NULL;
        while (f != NULL) {
            f_next = f->next;
            free(f);
            f = f_next;
        }
        free(h);
    }
}

int hash_file(void *helper, char *file_path) {
    FILE *fp = NULL;
    // 判断文件是否存在
    if (file_path == NULL) {
        return -1;
    }
    if (!(fp = fopen(file_path, "r"))) {
        return -2;
    }
    // TODO 判断helper是否存在
    if (helper == NULL) {
        return -1;
    }

    // hash算法
    int hash = 0;
    int len_name = (int) strlen(file_path);
    for (int i = 0; i < len_name; i++) {
        unsigned char byte = *(file_path + i);
//        printf("%commit_ptr\n", byte);
        hash = (hash + byte) % 1000;
    }
    //每次读取一个字节，直到读取完毕
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        unsigned char byte = ch;
//        printf("%commit_ptr\n", byte);
        hash = (hash + byte) % 2000000000;
    }
    fclose(fp);

    return hash;
}

char *svc_commit(void *helper, char *message) {
    // TODO: Implement
    if (message == NULL) {
        return NULL;
    }
    // TODO 判断helper是否存在
//    if (helper == NULL) {
//        return NULL;
//    }
    // 生成新的commit
    commit *c = (commit *) malloc(sizeof(commit));
    c->message = message;
    c->child_num = 0;
    c->child_list = NULL;
    // TODO CHANGE_TYPE 判断

    // TODO commit id 算法
    int id = 0;
    int len_name = (int) strlen(message);
    for (int i = 0; i < len_name; i++) {
        unsigned char byte = *(message + i);
//        printf("%commit_ptr\n", byte);
        id = (id + byte) % 1000;
    }

    // TODO 插入commit

    return NULL;
}

void *get_commit(void *helper, char *commit_id) {
    // TODO: Implement
    return NULL;
}

char **get_prev_commits(void *helper, void *commit, int *n_prev) {
    // TODO: Implement
    return NULL;
}

void print_commit(void *helper, char *commit_id) {
    // TODO: Implement
}

int svc_branch(void *helper, char *branch_name) {
    // TODO: Implement
    return 0;
}

int svc_checkout(void *helper, char *branch_name) {
    // TODO: Implement
    return 0;
}

char **list_branches(void *helper, int *n_branches) {
    // TODO: Implement
    return NULL;
}

int svc_add(void *helper, char *file_name) {
    // TODO: Implement
    helper_struct *h = (helper_struct *) helper;
    checkout *co = h->head_checkout;

    return 0;
}

int svc_rm(void *helper, char *file_name) {
    // TODO: Implement
    return 0;
}

int svc_reset(void *helper, char *commit_id) {
    // TODO: Implement
    return 0;
}

char *svc_merge(void *helper, char *branch_name, struct resolution *resolutions, int n_resolutions) {
    // TODO: Implement
    return NULL;
}

