#include "svc.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#define IS_DEBUG 1

void *svc_init(void) {
    helper_struct *helper = construct_helper();

    // checkout_struct
    // 默认生成master branch
    // 工作区内容在checkout中修改
    checkout_struct *co = construct_checkout("master", NULL);
    helper_add_checkout(&helper, co);

    return helper;
}

void cleanup(void *helper) {
    helper_struct *h = (helper_struct *) helper;
    if (h != NULL) {
        free_file(h->init_file);
        free_commit(h->init_commit);
        free_checkout(h->init_checkout);
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
    // 判断helper是否存在
    if (helper == NULL) {
        return -1;
    }

    // hash算法
    int hash = 0;
    int len_name = (int) strlen(file_path);
    for (int i = 0; i < len_name; i++) {
        unsigned char byte = *(file_path + i);
//        printf("%commit_pre\n", byte);
        hash = (hash + byte) % 1000;
    }
    //每次读取一个字节，直到读取完毕
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        unsigned char byte = ch;
//        printf("%commit_pre\n", byte);
        hash = (hash + byte) % 2000000000;
    }
    fclose(fp);

    return hash;
}

char *svc_commit(void *helper, char *message) {
    if (message == NULL) {
        return NULL;
    }
    // 判断helper是否存在
    if (helper == NULL) {
        return NULL;
    }
    helper_struct *h = (helper_struct *) helper;
    // 生成新的commit
    commit_struct *new_commit = construct_commit();
    new_commit->message = message;
    // 深拷贝 checkout_struct 的 file track
    new_commit->file_track = deepcopy_file_track(h->head_checkout->file_track);
    // 文件排序（根据file_name排序）
    file_list_sort(&(new_commit->file_track->file_list));
    // 将存储器中的文件与new_commit->file_track中的文件做对比
    check_disk_file(&(new_commit->file_track), h);

    // CHANGE_TYPE 判断
    // 将前一个commit与当前checkout的file track做对比 存储change_list
    // 如过前一个commit为NULL 则所有文件均为ADDITION
    // 文件缺失 则为DELETION
    // 存在的文件 计算其哈希值 与file track中的哈希值做对比
    checkout_struct *co = h->head_checkout;
    commit_struct *commit_pre = co->commit_pre;    // 当前checkout工作区的前一个commit
    change_list_struct *new_change_list = new_commit->file_track->change_list;;
    if (commit_pre == NULL) {
        // 当前工作区没有commit 所有文件均为ADDITION
        copy_file_2_change(&new_change_list, new_commit->file_track->file_list);
        set_all_change(&new_change_list, ADDITION);
    } else {
        // 当前工作区存在旧commit 将其与新commit做对比 为change_list赋值
        file_track_struct *new_ft = new_commit->file_track;
        file_track_struct *old_ft = commit_pre->file_track;
        // 对比文件差异 并且排序
        get_new_change_list(
                &new_change_list,
                old_ft->file_list,
                new_ft->file_list
        );
    }
    new_commit->file_track->change_list = new_change_list;

    // commit_struct id 算法 (需要文件排序)
    int id = 0;
    int len_name = (int) strlen(message);
    // 处理commit.message
    for (int i = 0; i < len_name; i++) {
        unsigned char byte = *(message + i);
//        printf("%commit_pre\n", byte);
        id = (id + byte) % 1000;
    }
    // 处理commit.changes
    change_list_struct *cl_ptr = new_change_list;
    if (cl_ptr == NULL) {
        // 与上个版本相比 没有change
        free_commit(new_commit);
        return NULL;
    }
    while (cl_ptr != NULL) {
        switch (cl_ptr->ptr->change_type) {
            case ADDITION:
                id += 376591;
                break;
            case DELETION:
                id += 85973;
                break;
            case MODIFICATION:
                id += 9573681;
                break;
            default:
                break;
        }
        len_name = (int) strlen(cl_ptr->ptr->file_name);
        for (int i = 0; i < len_name; i++) {
            unsigned char byte = *(cl_ptr->ptr->file_name + i);
            id = (id * (byte % 37)) % 15485863 + 1;
        }
        cl_ptr = cl_ptr->next;
    }


    // 将id转为16进制字符串
    char *result = int2hash(id);

    // commit_struct id 赋值
    new_commit->commit_id = result;

    // 向commit链表中 插入commit
    new_commit->branch_name = co->branch_name;
    helper_add_commit(&h, new_commit);
    co->commit_pre = new_commit;

    return result;
}

void *get_commit(void *helper, char *commit_id) {
    helper_struct *h = (helper_struct *) helper;
    commit_struct *c_ptr = h->init_commit;
    while (c_ptr != NULL) {
        if (strcmp(c_ptr->commit_id, commit_id) == 0) {
            break;
        }
    }
    return c_ptr;
}

char **get_prev_commits(void *helper, void *commit, int *n_prev) {
    if (helper == NULL) {
        return NULL;
    }
    if (n_prev == NULL) {
        return NULL;
    }
    if (commit == NULL) {
        *n_prev = 0;
        return NULL;
    }

    commit_struct *c = (commit_struct *) commit;
    int parent_num = c->parent_num;
    *n_prev = parent_num;
    if (parent_num == 0) {
        return NULL;
    }
    char **result = (char **) malloc(sizeof(char *) * parent_num);
    commit_link_list *c_ptr = c->parent_list;
    for (int i = 0; i < parent_num; i++) {
        result[i] = (char *) malloc(sizeof(char) * strlen(c_ptr->ptr->commit_id));
        strcpy(result[i], c_ptr->ptr->commit_id);
        c_ptr = c_ptr->next;
    }
    return result;
}

void print_commit(void *helper, char *commit_id) {
    if (commit_id == NULL || helper == NULL) {
        printf("Invalid commit id\n");
        return;
    }
    commit_struct *commit = helper_find_commit((helper_struct *) helper, commit_id);
    if (commit == NULL) {
        printf("Invalid commit id\n");
        return;
    }
    // print commit信息
    char *message = commit->message;
    if (message == NULL) {
        message = "";
    }
    printf("%s [%s]: %s\n", commit->commit_id, commit->branch_name, message);
    change_list_struct *cl = commit->file_track->change_list;
    while (cl != NULL) {
        char *type = NULL;
        char *file_name = cl->ptr->file_name;
        switch (cl->ptr->change_type) {
            case ADDITION:
                type = "+";
                break;
            case DELETION:
                type = "-";
                break;
            case MODIFICATION:
                type = "/";
                break;
            default:
                break;
        }
        printf("\t%s %s", type, file_name);
        if (cl->ptr->change_type == MODIFICATION) {
            printf("\t[%d --> %d]\n", cl->ptr->file_hash_pre, cl->ptr->file_hash);
        } else {
            printf("\n");
        }
        cl = cl->next;
    }
    printf("\n");
    printf("\tTracked files (%d):\n", commit->file_track->file_num);
    cl = commit->file_track->change_list;
    while (cl != NULL) {
        printf("\t[%10d] %s\n", cl->ptr->file_hash, cl->ptr->file_name);
        cl = cl->next;
    }
}

int svc_branch(void *helper, char *branch_name) {
    // branch_name格式检查不通过 返回-1
    // branch_name已存在 返回-2
    // 存在尚未commit的改动 返回-3
    // 成功branch 返回0

    helper_struct *h = (helper_struct *) helper;
    // 格式检查
    int name_len = strlen(branch_name);
    for (int i = 0; i < name_len; i++) {
        char c = *(branch_name + i);
        if (!(c >= 48 && c <= 57)   // 数字
            && !(c >= 65 && c <= 90)    // 大写字母
            && !(c >= 97 && c <= 122)   // 小写字母
            && c != 95 && c != 47 && c != 45) // 下划线_ 斜杠/ 减号-
        {// 格式检查不通过
            return -1;
        }
    }

    // 检查branch_name是否已存在
    checkout_struct *co_ptr = h->init_checkout;
    while (co_ptr != NULL) {
        if (strcmp(co_ptr->branch_name, branch_name) == 0) {
            return -2;
        }
        co_ptr = co_ptr->next;
    }
    // 检查是否有未commit的改动
    int result = check_uncommit_change(h);
    if (result != 1) {
        return -3;
    }

    // branch
    checkout_struct *new_co = construct_checkout(branch_name, h->head_checkout->commit_pre);
    helper_add_checkout(&h, new_co);

    return 0;
}

// 检查是否有未commit的change
// 有则返回-1
// 没有则返回1
int check_uncommit_change(helper_struct *helper) {
    // 检索disk上的文件 与commit_pre做对比
    file_track_struct *ft_temp = deepcopy_file_track(helper->head_checkout->file_track);   // 深拷贝
    check_disk_file(&ft_temp, helper);
    // 存在的文件 计算其哈希值 与file track中的哈希值做对比
    checkout_struct *co = helper->head_checkout;
    commit_struct *commit_pre = co->commit_pre;    // 当前checkout工作区的前一个commit
    change_list_struct *new_change_list = ft_temp->change_list;
    if (commit_pre == NULL) {
        // 当前工作区没有commit 所有文件均为ADDITION
        copy_file_2_change(&new_change_list, ft_temp->file_list);
        set_all_change(&new_change_list, ADDITION);
    } else {
        // 当前工作区存在旧commit 将其与新commit做对比 为change_list赋值
        file_track_struct *new_ft = ft_temp;
        file_track_struct *old_ft = commit_pre->file_track;
        get_new_change_list(
                &new_change_list,
                old_ft->file_list,
                new_ft->file_list
        );
    }
    // 检查change_list 若存在非UNCHANGED的change 则返回-3
    int flag = 1;
    change_list_struct *cl_ptr = new_change_list;
    while (cl_ptr != NULL) {
        if (cl_ptr->ptr->change_type != UNCHANGED) {
            flag = -1;
            break;
        }
        cl_ptr = cl_ptr->next;
    }
    // 释放临时对象ft_temp
    free_file_track(ft_temp);
    if (flag == 1) {
        return 1;
    } else {
        return -1;
    }
}

// checkout不改变工作空间 需要将file_track拷贝一份
int svc_checkout(void *helper, char *branch_name) {
    if (helper == NULL) {
        return -1;
    }
    if (branch_name == NULL) {
        return -1;
    }

    helper_struct *h = (helper_struct *) helper;
    // 检查是否有未commit的change
    int result = check_uncommit_change(h);
    if (result != 1) {
        // 存在未完成的commit
        return -2;
    }
    // 查找branch_name
    checkout_struct *co_ptr = h->init_checkout;

    while (co_ptr != NULL) {
        if (strcmp(co_ptr->branch_name, branch_name) == 0) {
            // 找到目标分支
            // 删除原有工作空间
            free_file_track(co_ptr->file_track);
            // 拷贝工作空间file_track
            co_ptr->file_track = deepcopy_file_track(h->head_checkout->file_track);
            // head重定向
            h->head_checkout = co_ptr;
            co_ptr->commit_pre->branch_name = co_ptr->branch_name;
            return 0;
        }
        co_ptr = co_ptr->next;
    }

    // 不存在分支branch_name
    return -1;
}

char **list_branches(void *helper, int *n_branches) {
    helper_struct *h = (helper_struct *) helper;
    int branch_num = h->checkout_num;
    *n_branches = branch_num;
    if (branch_num == 0) {
        return NULL;
    }
    char **result = (char **) malloc(sizeof(char *) * branch_num);
    checkout_struct *co_ptr = h->init_checkout;
    for (int i = 0; i < branch_num; i++) {
        result[i] = (char *) malloc(sizeof(char) * strlen(co_ptr->branch_name));
        strcpy(result[i], co_ptr->branch_name);
        co_ptr = co_ptr->next;
    }
    return result;
}

int svc_add(void *helper, char *file_name) {
    // TODO 内存泄漏
    // 文件添加至helper_struct->init_file中
    // 并且链接到checkout
    FILE *fp = NULL;
    int hash;
    // 判断文件是否存在
    if (file_name == NULL) {
        return -1;
    }
    if (!(fp = fopen(file_name, "r"))) {
        return -3;
    }
    //
    helper_struct *h = (helper_struct *) helper;
    checkout_struct *co = h->head_checkout;
    file_track_struct *ft = co->file_track;
    // 检索当前工作区的文件
    file_list_struct *file_list = ft->file_list;
    while (file_list != NULL) {
        if (strcmp(file_list->ptr->file_name, file_name) == 0) {
            // 文件file_name已存在于当前工作区
            return -2;
        }
        file_list = file_list->next;
    }
    // 新建文件
    file_struct *new_file = construct_file();
    new_file->file_name = file_name;
    new_file->file_content = read_file(file_name);
    new_file->file_hash = hash_file(helper, file_name);
    // 保存哈希值
    hash = new_file->file_hash;
    // 向文件列表中添加文件
    helper_add_file(&h, new_file);

    // 向工作区添加文件
    // 添加至file_list
    checkout_add_file(&h, new_file);

    return hash;
}

int svc_rm(void *helper, char *file_name) {
    if (helper == NULL) {
        return -1;
    }
    if (file_name == NULL) {
        return -1;
    }
    // 检索file_track
    helper_struct *h = (helper_struct *) helper;
    file_track_struct *ft = h->head_checkout->file_track;
    file_list_struct *fl_head = construct_file_list(NULL);  // 临时对象 指向链表头
    fl_head->next = ft->file_list;
    file_list_struct *fl_pre = fl_head;
    file_list_struct *fl_next = fl_head->next;
    int hash = -1;
    while (fl_next != NULL) {
        // 检索目标文件 删除 返回其hash值
        if (strcmp(fl_next->ptr->file_name, file_name) == 0) {
            // 存储hash
            hash = fl_next->ptr->file_hash;
            // 对链表内第一个元素做特殊处理
            if (fl_next == fl_head->next) {
                ft->file_list = fl_next->next;
            }
            // 从file_track中删除目标文件
            fl_pre->next = fl_next->next;
            // 释放目标文件
            fl_next->next = NULL;
            free_file_list_single(fl_next);
            // file_tracker文件计数
            ft->file_num--;
            break;
        }
        fl_pre = fl_pre->next;
        fl_next = fl_next->next;
    }

    free_file_list_single(fl_head);    // 释放临时对象

    if (hash != -1) {
        return hash;
    } else {
        // 不存在文件file_name 返回-2
        return -2;
    }
}

int svc_reset(void *helper, char *commit_id) {
    if (helper == NULL) {
        return -1;
    }
    if (commit_id == NULL) {
        return -1;
    }
    helper_struct *h = (helper_struct *) helper;
    // 判断commit_id是否存在
    // 若不存在 则返回-2
    // 从head开始往回找
    // 考虑branch时 考虑当前节点的branch_name 对其直接父节点有效
    char *branch_name = h->head_checkout->branch_name;
    commit_struct *c_ptr = h->head_checkout->commit_pre;
    while (c_ptr != NULL) {
        if (strcmp(c_ptr->commit_id, commit_id) == 0) {
            break;
        }
        // 当前commit脱离branch
        if (strcmp(c_ptr->branch_name, branch_name) != 0) {
            c_ptr = NULL;
            break;
        }
        // 指向前一个commit
        if (c_ptr->parent_list != NULL) {
            c_ptr = c_ptr->parent_list->ptr;    // 注：保证有且仅有一个parent
        } else {
            c_ptr = NULL;
        }
    }
    // commit_id不存在 则返回-2
    if (c_ptr == NULL) {
        return -2;
    }

    // reset 忽略所有变更
    // 将commit中的所有文件都输出到disk
    h->head_checkout->commit_pre = c_ptr;
    commit_write_all_file(c_ptr);

    // reset之后 部分commit需要被释放
    free_commit_link_list(c_ptr->child_list);
    c_ptr->child_list = NULL;

    return 0;
}

char *svc_merge(void *helper, char *branch_name, struct resolution *resolutions, int n_resolutions) {
    // TODO: Implement

    if (helper == NULL) {
        return 0;
    }
    if (branch_name == NULL) {
        return 0;
    }
    if (resolutions == NULL) {
        return 0;
    }
    if (n_resolutions == -1) {
        return 0;
    }
    return NULL;
}

// 读取文件的全部内容
// 该方法使用了malloc 需要主动释放
char *read_file(char *path) {
    int *length = (int *) malloc(sizeof(int));
    FILE *pfile;
    char *data;

    pfile = fopen(path, "rb");
    if (pfile == NULL) {
        return NULL;
    }
    fseek(pfile, 0, SEEK_END);
    *length = ftell(pfile);
    data = (char *) malloc((*length + 1) * sizeof(char));
    rewind(pfile);
    *length = fread(data, 1, *length, pfile);
    data[*length] = '\0';
    fclose(pfile);
    free(length);
    return data;
}

void print_file(void *helper) {
    helper_struct *h = (helper_struct *) helper;
    file_struct *file = h->init_file;
    printf("%d\n", h->file_num);
    while (file != NULL) {
        printf("%s\n", file->file_name);
        file = file->next;
    }
}

// 深拷贝 file track
file_track_struct *deepcopy_file_track(file_track_struct *ft) {
    if (ft == NULL) {
        return NULL;
    }
    file_track_struct *new_ft = construct_file_track();
    new_ft->change_list = NULL;
    new_ft->file_list = NULL;
    new_ft->file_num = ft->file_num;
    // 深拷贝 file_list
    if (ft->file_list == NULL) {
        new_ft->file_list = NULL;
    } else {
        file_list_struct *ft_fl = ft->file_list;
        file_list_struct *new_ft_fl_head = construct_file_list(ft_fl->ptr);
        file_list_struct *new_ft_fl_pre = new_ft_fl_head;
        file_list_struct *new_ft_fl_next;
        ft_fl = ft_fl->next;
        while (ft_fl != NULL) {
            new_ft_fl_next = construct_file_list(ft_fl->ptr);
            new_ft_fl_pre->next = new_ft_fl_next;
            new_ft_fl_pre = new_ft_fl_next;
            ft_fl = ft_fl->next;
        }
        new_ft->file_list = new_ft_fl_head;
    }

    // 深拷贝 change_list
    if (ft->change_list != NULL) {
        change_list_struct *new_ft_cl_head = construct_change_list(NULL);
        change_list_struct *new_ft_cl_pre = new_ft_cl_head;
        change_list_struct *new_ft_cl_next;
        change_list_struct *ft_cl = ft->change_list;
        change_struct *new_change = construct_change();
        new_change->change_type = ft_cl->ptr->change_type;
        new_change->file_name = ft_cl->ptr->file_name;
        new_change->file_hash = ft_cl->ptr->file_hash;
        new_ft_cl_head->ptr = new_change;
        ft_cl = ft_cl->next;
        while (ft_cl != NULL) {
            new_ft_cl_next = construct_change_list(new_change);
            new_change = (change_struct *) malloc(sizeof(change_struct));
            new_change->change_type = ft_cl->ptr->change_type;
            new_change->file_name = ft_cl->ptr->file_name;
            new_change->file_hash = ft_cl->ptr->file_hash;
            new_ft_cl_pre->next = new_ft_cl_next;
            new_ft_cl_pre = new_ft_cl_next;
            ft_cl = ft_cl->next;
        }
    }
    return new_ft;
}

void file_list_add_file(file_list_struct **fl_output, file_struct *file) {
    file_list_struct *fl = *fl_output;
    if (fl == NULL) {
        fl = construct_file_list(file);
        fl->next = NULL;
        *fl_output = fl;
    } else {
        while (fl->next != NULL) {
            fl = fl->next;
        }
        fl->next = construct_file_list(file);
    }
}

//void change_list_add_file(change_list_struct *change_list, file_struct *file) {
//    if (change_list == NULL) {
//        change_list = (change_list_struct *) malloc(sizeof(change_list_struct));
//        change_list->ptr = (change_struct *) malloc(sizeof(change_struct));
//        change_list->next = NULL;
//        change_list->ptr->file_name = file->file_name;
//        change_list->ptr->change_type = ADDITION;
//    } else {
//        while (change_list->next != NULL) {
//            change_list = change_list->next;
//        }
//        change_list->next = (change_list_struct *) malloc(sizeof(change_list_struct));
//        change_list->next->next = NULL;
//        change_list->next->ptr = (change_struct *) malloc(sizeof(change_struct));
//        change_list->next->ptr->file_name = file->file_name;
//        change_list->next->ptr->change_type = ADDITION;
//    }
//}

// 将change_list中的所有change_type置为type
void set_all_change(change_list_struct **change_list_output, change_type type) {
    change_list_struct *change_list = *change_list_output;
    change_list_struct *cl = change_list;
    while (cl != NULL) {
        cl->ptr->change_type = type;
        cl = cl->next;
    }
}

// 将file_list 中的内容 拷贝至 change_list
void copy_file_2_change(change_list_struct **change_list_output, file_list_struct *file_list) {
    change_list_struct *change_list = *change_list_output;
    assert(change_list == NULL);
    if (file_list != NULL) {
        change_list_struct *cl_head = construct_change_list(construct_change());
        change_list_struct *cl_pre = cl_head;
        change_list_struct *cl_next;
        cl_pre->ptr->file_name = file_list->ptr->file_name;
        cl_pre->ptr->file_hash = file_list->ptr->file_hash;
        while (file_list->next != NULL) {
            cl_next = construct_change_list(construct_change());
            cl_next->ptr->file_name = file_list->next->ptr->file_name;
            cl_next->ptr->file_hash = file_list->next->ptr->file_hash;
            cl_pre->next = cl_next;
            cl_pre = cl_pre->next;
            file_list = file_list->next;
        }
        change_list = cl_head;
    }
    *change_list_output = change_list;
}

void helper_add_commit(helper_struct **helper_output, commit_struct *new_commit) {
    helper_struct *helper = *helper_output;
    commit_struct *commit_ptr = helper->head_checkout->commit_pre;
    if (commit_ptr == NULL) {
        helper->init_commit = new_commit;
    } else {
        commit_add_commit(&commit_ptr, new_commit);
    }
    helper->commit_num++;
}

void helper_add_file(helper_struct **helper_output, file_struct *new_file) {
    helper_struct *h = *helper_output;
    h->file_num++;
    if (h->init_file == NULL) {
        h->init_file = new_file;
    } else {
        file_struct *file = h->init_file;
        while (file->next != NULL) {
            file = file->next;
        }
        new_file->next = NULL;
        file->next = new_file;
    }
}

void helper_add_checkout(helper_struct **helper_output, checkout_struct *new_checkout) {
    helper_struct *helper = *helper_output;
    checkout_struct *co = helper->init_checkout;

    if (co == NULL) {
        helper->init_checkout = new_checkout;
        helper->head_checkout = new_checkout;
    } else {
        while (co->next != NULL) {
            co = co->next;
        }
        co->next = new_checkout;
    }
    helper->checkout_num++;
}

// checkout构造方法
checkout_struct *construct_checkout(char *branch_name, commit_struct *commit_pre) {
    checkout_struct *co = (checkout_struct *) malloc(sizeof(checkout_struct));
    co->branch_name = branch_name;
    co->next = NULL;
    co->commit_pre = commit_pre;
    co->file_track = construct_file_track();

    return co;
}

change_list_struct *construct_change_list(change_struct *cg) {
    change_list_struct *cl = (change_list_struct *) malloc(sizeof(change_list_struct));
    cl->next = NULL;
    cl->ptr = cg;
    return cl;
}

file_list_struct *construct_file_list(file_struct *file) {
    file_list_struct *fl = (file_list_struct *) malloc(sizeof(file_list_struct));
    fl->next = NULL;
    fl->ptr = file;
    return fl;
}

file_track_struct *construct_file_track() {
    file_track_struct *ft = (file_track_struct *) malloc(sizeof(file_track_struct));
    ft->file_list = NULL;
    ft->change_list = NULL;
    ft->file_num = 0;
    return ft;
}

commit_struct *construct_commit() {
    commit_struct *c = (commit_struct *) malloc(sizeof(commit_struct));
    c->message = NULL;
    c->child_num = 0;
    c->parent_num = 0;
    c->child_list = NULL;
    c->parent_list = NULL;
    c->commit_id = NULL;
    c->file_track = NULL;
    c->branch_name = NULL;
    return c;
}

change_struct *construct_change() {
    change_struct *cg = (change_struct *) malloc(sizeof(change_struct));
    cg->file_name = NULL;
    cg->file_hash = -1;
    cg->file_hash = -1;
    cg->change_type = UNCHANGED;
    return cg;
}

commit_link_list *construct_commit_link_list() {
    commit_link_list *ccl = (commit_link_list *) malloc(sizeof(commit_link_list));
    ccl->next = NULL;
    ccl->ptr = NULL;
    return ccl;
}

void free_checkout(checkout_struct *co) {
    if (co != NULL) {
        free_checkout(co->next);
        free_file_track(co->file_track);
//        free(co->file_track);
        free(co);
    } else {
        if (IS_DEBUG == 1) {
            fprintf(stderr, "ERROR free_checkout(checkout_struct *co)==NULL\n");
        }
    }
}

void free_change_list(change_list_struct *cl) {
    if (cl != NULL) {
        free_change_list(cl->next);
        free_change(cl->ptr);
        free(cl);
    } else {
        if (IS_DEBUG == 1) {
            fprintf(stderr, "ERROR free_change_list(change_list_struct *cl)==NULL\n");
        }
    }
}

void free_file_list(file_list_struct *fl) {
    if (fl != NULL) {
        free_file_list(fl->next);
        free(fl);
    } else {
        if (IS_DEBUG == 1) {
            fprintf(stderr, "ERROR free_file_list(file_list_struct *fl)==NULL\n");
        }
    }
}

void free_file_list_single(file_list_struct *fl) {
    if (fl != NULL) {
        free(fl);
    } else {
        if (IS_DEBUG == 1) {
            fprintf(stderr, "ERROR free_file_list_single(file_list_struct *fl)==NULL\n");
        }
    }
}

void free_file_track(file_track_struct *ft) {
    if (ft != NULL) {
        free_change_list(ft->change_list);
        free_file_list(ft->file_list);
        free(ft);
    } else {
        if (IS_DEBUG == 1) {
            fprintf(stderr, "ERROR free_file_track(file_track_struct *ft)==NULL\n");
        }
    }
}

void free_change(change_struct *cg) {
    if (cg != NULL) {
        free(cg);
    } else {
        if (IS_DEBUG == 1) {
            fprintf(stderr, "ERROR free_change(change_struct *cg)==NULL\n");
        }
    }
}

// 释放commit_child_list中的所有comiit
void free_commit_link_list(commit_link_list *ccl) {
    if (ccl != NULL) {
        free_commit_link_list(ccl->next);
        free_commit(ccl->ptr);
        free(ccl);
    } else {
        if (IS_DEBUG == 1) {
            fprintf(stderr, "ERROR free_commit_link_list(commit_link_list *ccl)==NULL\n");
        }
    }
}

void free_commit(commit_struct *c) {
    if (c != NULL) {
        free_commit_link_list(c->child_list);
//        free_commit_link_list(c->parent_list);
        free_file_track(c->file_track);
//        if(c->branch_name!=NULL){
//            free(c->branch_name);
//        }
        free(c->commit_id);
        free(c);
    } else {
        if (IS_DEBUG == 1) {
            fprintf(stderr, "ERROR free_commit(commit_struct* c)==NULL\n");
        }
    }
}

// 将新的commit 链接为旧commit的儿子
void commit_add_commit(commit_struct **c_parent_output, commit_struct *c_child) {
    commit_struct *c_parent = *c_parent_output;
    commit_link_list_add_commit(&(c_parent->child_list), c_child);
    commit_link_list_add_commit(&(c_child->parent_list), c_parent);
    c_parent->child_num++;
    c_child->parent_num++;
}

void commit_link_list_add_commit(commit_link_list **ccl_output, commit_struct *c) {
    commit_link_list *ccl = *ccl_output;
    if (ccl == NULL) {
        ccl = construct_commit_link_list();
        ccl->ptr = c;
    } else {
        while (ccl->next != NULL) {
            ccl = ccl->next;
        }
        ccl->next = construct_commit_link_list();
        ccl->next->ptr = c;
    }
    *ccl_output = ccl;
}

helper_struct *construct_helper() {
    helper_struct *helper = (helper_struct *) malloc(sizeof(helper_struct));

    // 初始没有commit
    helper->init_commit = NULL;
    helper->commit_num = 0;

    // file_list
    helper->init_file = NULL;
    helper->file_num = 0;

    // checkout_struct
    helper->init_checkout = NULL;
    helper->checkout_num = 0;

    // head_checkout
    helper->head_checkout = NULL;
    return helper;
}

void free_file(file_struct *f) {
    if (f != NULL) {
        free_file(f->next);
        free(f->file_content);
        free(f);
    } else {
        if (IS_DEBUG == 1) {
            fprintf(stderr, "ERROR free_file(file_struct *f)==NULL\n");
        }
    }
}

file_struct *construct_file() {
    file_struct *file = (file_struct *) malloc(sizeof(file_struct));
    file->file_hash = -1;
    file->file_name = NULL;
    file->file_content = NULL;
    file->next = NULL;
    return file;
}

// 拷贝构造函数
file_struct *copy_construct_file(helper_struct **helper_output, file_struct *file) {
    file_struct *new_file = (file_struct *) malloc(sizeof(file_struct));
    new_file->file_hash = file->file_hash;
    new_file->file_name = file->file_name;
    new_file->file_content = file->file_content;
    new_file->next = NULL;
//    helper_add_file(helper_output, new_file);
    return new_file;
}

// 在磁盘上检查file_track_in_out中的文件 删去不存在的文件 标记改变的文件
void check_disk_file(file_track_struct **file_track_in_out, helper_struct *h) {
    file_track_struct *file_track = *file_track_in_out;
    if (file_track == NULL) {
        return;
    }
    file_list_struct *fl_head = construct_file_list(NULL);  // 链表头指针 不存储数据
    fl_head->next = file_track->file_list;
    file_list_struct *fl_mid = file_track->file_list;;
    file_list_struct *fl_pre = fl_head;
    file_list_struct *fl_next = NULL;
    file_list_struct *temp = NULL;
    if (fl_mid != NULL) {
        fl_next = fl_mid->next;
        while (fl_mid != NULL) {
            //在磁盘上搜索fl_mid的文件
            int hash = hash_file(h, fl_mid->ptr->file_name);
            switch (hash) {
                case -1:
                    // 文件hash出错
                    if (IS_DEBUG) {
                        fprintf(stderr, "ERROR check_disk_file hash_file file_path == NULL\n");
                    }
                    // 处理下一个文件
                    temp = fl_mid->next;
                    fl_pre = fl_mid;
                    fl_mid = fl_next;
                    fl_next = temp;
                    break;
                case -2:
                    // 文件不存在 从list中删除
                    fl_pre->next = fl_next;
                    fl_mid->next = NULL;
                    free_file_list(fl_mid);
                    // 处理下一个文件
                    fl_mid = fl_next;
                    if (fl_mid != NULL) {
                        fl_next = fl_mid->next;
                    }
                    break;
                default:
                    // 文件存在 比较hash值
                    if (hash != fl_mid->ptr->file_hash) {
                        // hash不相等 构建新对象
                        file_struct *new_file = copy_construct_file(&h, fl_mid->ptr);
                        new_file->file_content = read_file(fl_mid->ptr->file_name);
                        new_file->file_hash = hash;
                        // 使用新对象替代file track中的对象
                        fl_mid->ptr = new_file;
                        // 向helper中加入新建对象
                        helper_add_file(&h, new_file);
                    }
                    // 处理下一个文件
                    temp = fl_mid->next;
                    fl_pre = fl_mid;
                    fl_mid = fl_next;
                    fl_next = temp;
                    break;
            }
        }
    }
    free_file_list_single(fl_head);
}

// 对比两个file_track 生成 change_list_struct
void get_new_change_list(change_list_struct **cl_output, file_list_struct *fl_old, file_list_struct *fl_new) {
    change_list_struct *cl = *cl_output;
    if (cl != NULL) {
        fprintf(stderr, "get_new_change_list(change_list_struct **cl_output == NULL\n");
        free_change_list(cl);
    }
    // 文件排序（根据file_name排序）
    file_list_sort(&fl_old);
    file_list_sort(&fl_new);
    // 对比两个有序文件序列 检查新增、减少、修改情况
    // 声明两个指针
    file_list_struct *fl_old_ptr = fl_old;
    file_list_struct *fl_new_ptr = fl_new;
    while (fl_old_ptr != NULL || fl_new_ptr != NULL) {
        // 指针指向链表末尾 才结束循环
        int cmp_ressult;
        if (fl_old_ptr != NULL && fl_new_ptr != NULL) {
            cmp_ressult = strcmp(
                    fl_old_ptr->ptr->file_name,
                    fl_new_ptr->ptr->file_name
            );
        } else if (fl_old_ptr != NULL) {
            cmp_ressult = -1;
        } else {
            cmp_ressult = 1;
        }
        if (cmp_ressult > 0) {
            // old比new大
            // new中存在新增的文件
            // new的指针后移
            change_struct *cg = construct_change();
            cg->change_type = ADDITION;
            cg->file_name = fl_new_ptr->ptr->file_name;
            cg->file_hash = fl_new_ptr->ptr->file_hash;
            change_list_struct_add_change(&cl, cg);

            fl_new_ptr = fl_new_ptr->next;
        } else if (cmp_ressult < 0) {
            // old比new小
            // new中存在删除的文件
            // old的指针后移
            change_struct *cg = construct_change();
            cg->change_type = DELETION;
            cg->file_name = fl_old_ptr->ptr->file_name;
            cg->file_hash = fl_old_ptr->ptr->file_hash;
            change_list_struct_add_change(&cl, cg);

            fl_old_ptr = fl_old_ptr->next;
        } else {
            // 文件名相同
            // 比较hash 判断是否有文件修改
            // 指针同时后移
            if (fl_old_ptr->ptr->file_hash != fl_new_ptr->ptr->file_hash) {
                // 文件hash不一致 发生更改
                change_struct *cg = construct_change();
                cg->change_type = MODIFICATION;
                cg->file_name = fl_new_ptr->ptr->file_name;
                cg->file_hash = fl_new_ptr->ptr->file_hash;
                cg->file_hash_pre = fl_new_ptr->ptr->file_hash;
                change_list_struct_add_change(&cl, cg);
            }

            fl_old_ptr = fl_old_ptr->next;
            fl_new_ptr = fl_new_ptr->next;
        }
    }
    *cl_output = cl;
}

// 对file_list进行排序
void file_list_sort(file_list_struct **fl_output) {
    file_list_struct *fl = *fl_output;
    file_list_struct *fl_head = construct_file_list(NULL);  // 链表头指针 不存储数据
    fl_head->next = fl;
//    file_list_struct *fl_pre = fl_head;
    file_list_struct *fl_mid = fl;
    file_list_struct *fl_next = NULL;
//    file_list_struct *fl_temp = NULL;
//     *f_temp = NULL;
    if (fl_mid != NULL) {
        fl_next = fl_mid->next;
        while (fl_mid != NULL) {
            // 将 mid和next进行比较
            if (fl_next != NULL) {
                // 修改比较算法 字母不区分大小写 其余字符按照ascii排序
                // “aaa.txt">"AAA.txt";
                int cmp_ressult = diy_strcmp(
                        fl_mid->ptr->file_name,
                        fl_next->ptr->file_name
                );
//                if (cmp_ressult < 0) {
                if (cmp_ressult > 0) {
                    // mid比next大时 交换顺序 以升序
                    file_struct *f_temp = fl_mid->ptr;
                    fl_mid->ptr = fl_next->ptr;
                    fl_next->ptr = f_temp;
                }
            }
            // 处理下一个文件
            fl_mid = fl_mid->next;
            if (fl_next != NULL) {
                fl_next = fl_next->next;
            }
//            fl_temp = fl_mid->next;
////            fl_pre = fl_mid;
//            fl_mid = fl_next;
//            fl_next = fl_temp->next;
        }
    }
    free_file_list_single(fl_head);
}


void change_list_struct_add_change(change_list_struct **cl_output, change_struct *cg) {
    change_list_struct *cl = *cl_output;
    if (cl == NULL) {
        cl = construct_change_list(cg);
        *cl_output = cl;
    } else {
        while (cl->next != NULL) {
            cl = cl->next;
        }
        cl->next = construct_change_list(cg);
    }
}

void checkout_add_file(helper_struct **h_output, file_struct *file) {
    helper_struct *h = *h_output;
    file_track_struct *ft = h->head_checkout->file_track;
    ft->file_num++;
    file_list_struct *fl = ft->file_list;
    file_list_add_file(&fl, file);
    ft->file_list = fl;
}


// 递归搜索
commit_struct *find_commit(commit_struct *commit, char *commit_id) {
    if (commit == NULL) {
        return NULL;
    }
    if (strcmp(commit->commit_id, commit_id) == 0) {
        return commit;
    } else {
        commit_link_list *cl_ptr = commit->child_list;
        while (cl_ptr != NULL) {
            commit_struct *find_child_commit = find_commit(cl_ptr->ptr, commit_id);
            if (find_child_commit != NULL) {
                return find_child_commit;
            }
        }
        return NULL;
    }
}

commit_struct *helper_find_commit(helper_struct *helper, char *commit_id) {
    if (helper == NULL) {
        return NULL;
        if (IS_DEBUG) {
            fprintf(stderr, "ERROR commit_struct *helper_find_commit helper==NULL\n");
        }
    }
    if (commit_id == NULL) {
        return NULL;
        if (IS_DEBUG) {
            fprintf(stderr, "ERROR commit_struct *helper_find_commit commit_id==NULL\n");
        }
    }
    return find_commit(helper->init_commit, commit_id);
}

// 将int类型 转换为6位16进制字符串
// 注：返回值需要被释放
char *int2hash(int id) {
    char *result = (char *) malloc(sizeof(char) * 6);
    for (int i = 5; i >= 0; i--) {
        char x = id % 16;
        if (x < 10) {
            // 处理0-9
            x += 48;
        } else {
            // 处理a-f
            x += 87;
        }
        id /= 16;
        result[i] = x;
    }
    return result;
}

// 自定义的字符串比较函数
// 当字符为字母时 无视大小写（小写字符ascii-32）
// 当字符为非字母时 使用ascii比较
// 当两个字符串完全相等时 考虑大小写
// 大小写也相等时 输出0
// 若str1>str2 输出1
// 若str1<str2 输出-1
int diy_strcmp(char *str1, char *str2) {
    int len_str1 = strlen(str1);
    int len_str2 = strlen(str2);
    int ptr_str1 = 0;
    int ptr_str2 = 0;
    int result = 0;       // 不考虑大小写的结果
    int result_case = 0;  // 考虑大小写的结果

    while (ptr_str1 < len_str1 && ptr_str2 < len_str2) {
        char c1 = *(str1 + ptr_str1);
        char c2 = *(str2 + ptr_str2);
        // 将小写字母转换为大写字母
        if (97 <= c1 && c1 <= 122) {
            c1 -= 32;
        }
        if (97 <= c2 && c2 <= 122) {
            c2 -= 32;
        }
        // 比较大小
        if (c1 > c2) {
            result = 1;
            break;
        } else if (c1 < c2) {
            result = -1;
            break;
        } else {
            // 若相等 比较大小写
            if (result_case == 0) {
                c1 = *(str1 + ptr_str1);
                c2 = *(str2 + ptr_str2);
                if (c1 > c2) {
                    result_case = 1;
                } else if (c1 < c2) {
                    result_case = -1;
                }
            }
        }
        ptr_str1++;
        ptr_str2++;
    }
    // 某一字符串执行到底仍相等 根据字符串长度来判断
    if (result == 0) {
        if (len_str1 > len_str2) {
            result = 1;
        } else if (len_str1 < len_str2) {
            result = -1;
        }
    }
    //
    if (result == 0) {
        return result_case;
    } else {
        return result;
    }
}

// 将commit_struct *commit中的所有文件 都输出到disk中
void commit_write_all_file(commit_struct *commit) {
    file_list_struct *fl = commit->file_track->file_list;
    while (fl != NULL) {
        char *file_name = fl->ptr->file_name;
        char *file_content = fl->ptr->file_content;
        // 写入文件
        FILE *fp = fopen(file_name, "w");
        fprintf(fp, "%s", file_content);
        fclose(fp);
        // 指向下一个
        fl = fl->next;
    }
}