//
// Created by Sam on 2018/2/13.
//

#include <dongmensql/optimizer.h>

/**
 * 使用关于选择的等价变换规则将条件下推。
 *
 */


/*输入一个关系代数表达式，输出优化后的关系代数表达式
 * 要求：在查询条件符合合取范式的前提下，根据等价变换规则将查询条件移动至合适的位置。
 * */


int opt_expr_test(Expression *expr) {
    Expression* e=expr;
    while(!e->term){
        int order=e->opType;
        if(order<=15){
            e=e->nextexpr;
            continue;
        }else if(order==16||order==17){
            return 0;
        }
    }


    return 1;

}




void swap(int *a,int *b){
    int tmp=*a;
    *a=*b;
    *b=tmp;
}

SRA_t* break_and(Expression** expr,SRA_t *table){
    Expression* pexpr=*expr;
    Expression* ans;
    SRA_t* result;
    SRA_t* left,*right;
    if(pexpr->opType==TOKEN_AND){
        pexpr=pexpr->nextexpr;//pass the token_and
        left=break_and(&pexpr,table);
        right=break_and(&pexpr,left);
        result=right;
        *expr=pexpr;
    }else{
            int num=operators[pexpr->opType].numbers;
        Expression* tmp;
        while(num--)
            pexpr=pexpr->nextexpr;
        tmp=pexpr->nextexpr;
        pexpr->nextexpr=NULL;
        result=SRASelect(table,*expr);
        *expr=tmp;
    }

    return result;

}

char* get_table_by_field(record_file *fcatFile,char* columnName){
    record_file_moveto(fcatFile,0);
    char* fieldname1=new_id_name();
    char* tablename=new_id_name();
    for(int i=0;i<7;i++) record_file_next(fcatFile);
    while(record_file_next(fcatFile)){

        record_file_get_string(fcatFile,"fieldname",fieldname1);
        if(strcmp(fieldname1,columnName)==0)
        {
            record_file_get_string(fcatFile,"tablename",tablename);

            break;
        }
        //printf("%s\n",tablename);
    }
    free(fieldname1);
    return tablename;
}

int find_place(SRA_t *table,char *table1,arraylist* list) {
    if (strcmp(table1, "") == 0)
        return 0;

    if (table->t == SRA_TABLE) {
        arraylist_add(list, table);
        if (strcmp(table1, table->table.ref->table_name) == 0) {
            return 1;
        }
        arraylist_remove_by_element(list, table);
        return 0;
    } else if (table->t == SRA_JOIN) {
        arraylist_add(list, table);
        int flag1 = find_place(table->join.sra1, table1, list);
        int flag2 = find_place(table->join.sra2, table1, list);

        if (!flag1 && !flag2) {
            arraylist_remove_by_element(list, table);
            return 0;
        }
        return 1;
    } else if (table->t == SRA_SELECT) {
        arraylist_add(list, table);
        int cnt=1;
        while(table->select.sra->t==SRA_SELECT){
            arraylist_add(list,table->select.sra);
            cnt++;
            table=table->select.sra;
        }
        int flag = find_place(table->select.sra, table1, list);
        if (flag) return 1;
        else {
            //arraylist_remove_by_element(list, table);
            while(cnt--){
                arraylist_remove(list,list->size-1);
            }
            return 0;
        }
    }else if(table->t==SRA_PROJECT){
        arraylist_add(list,table);
        int flag=find_place(table->project.sra,table1,list);
        if(flag) return 1;
        else{
            arraylist_remove_by_element(list,table);
            return 0;
        }
    }
}


int find_common_point(arraylist* path1,arraylist* path2){
    if(path2->size==0){
        return path1->size-2;
    }else{
        int least=path1->size>path2->size?path2->size:path1->size;
        for(int i=0;i<least;i++){
            if(arraylist_get(path1,i)!=arraylist_get(path2,i))
                return i-2;
        }
    }
}


void arraylist_getall(arraylist* list,size_t size){
    for(int i=0;i<size;i++)
        printf("%x ",(SRA_t*)arraylist_get(list,i));
}

void conncat_sra(SRA_t* sra1,SRA_t* sra2,SRA_t* after){
    if(sra1->t==SRA_SELECT) sra1->select.sra=sra2;
    else if(sra1->t==SRA_PROJECT) sra1->select.sra=sra2;
    else if(sra1->t==SRA_JOIN) {
        if(sra1->join.sra1==after) sra1->join.sra1=sra2;
        else sra1->join.sra2=sra2;
    }
}


SRA_t *dongmengdb_algebra_optimize_condition_pushdown(SRA_t *sra,table_manager *tableManager,transaction *tx){

    /*初始关系代数语法树sra由三个操作构成：SRA_PROJECT -> SRA_SELECT -> SRA_JOIN，即对应语法树中三个节点。*/

    /*第一步：.等价变换：将SRA_SELECT类型的节点进行条件串接*/

    /*1.1 在sra中找到每个SRA_Select节点 */
    /*1.2 检查每个SRA_Select节点中的条件是不是满足串接条件：多条件且用and连接*/
    /*1.3 若满足串接条件则：创建一组新的串接的SRA_Select节点，等价替换当前的SRA_Select节点*/

    /*第二步：等价变换：条件交换*/
    /*2.1 在sra中找到每个SRA_Select节点*/
    /*2.2 对每个SRA_Select节点做以下处理：
     * 在sra中查找 SRA_Select 节点应该在的最优位置：
     *     若子操作也是SRA_Select，则可交换；
     *     若子操作是笛卡尔积，则可交换，需要判断SRA_Select所包含的属性属于笛卡尔积的哪个子操作
     * 最后将SRA_Select类型的节点移动到语法树的最优位置。
     * */
   //printf("\nbefore:\n");
    //SRA_print(sra);
    //int flag=opt_condition_test(sra);
    //if(flag==0)
      //  return sra;
    SRA_t *project;
    SRA_t *select;
    SRA_t *table;
    Expression* expr;
    SRA_t *psra=sra;
    if(psra->t==SRA_PROJECT){
        project=psra;
        psra=psra->project.sra;
    }
    if(psra->t==SRA_SELECT){
        select=psra;
        psra=psra->select.sra;
    }
    expr=select->select.cond;

    //判断是否是合取范式，如果不是，直接返回
    if(!opt_expr_test(expr)){
        return sra;
    }

    if(psra->t==SRA_TABLE||psra->t==SRA_JOIN){
        table=psra;
    }
    SRA_t* new_select;
    new_select=break_and(&expr,psra);

    //SRA_print(new_select);
    project->project.sra=new_select;
    SRA_t*s =new_select;
    while(s->t==SRA_SELECT){
        if(s->select.sra->t==SRA_SELECT){
            s=s->select.sra;
            continue;
        }else{
            s->select.sra=table;
            break;
        }
    }


    record_file *fcatFile = (record_file *) malloc(sizeof(record_file));
    record_file_create(fcatFile, tableManager->fcatInfo, tx);
    //record_file_moveto_recordid(fcatFile,0);

    s=new_select;
    SRA_t* before=project;
    while(s->t==SRA_SELECT){
        arraylist* fieldlist=arraylist_create();
        arraylist* path1=arraylist_create();
        arraylist* path2=arraylist_create();
        int cnt=1;
        for(Expression* e=s->select.cond;e!=NULL;e=e->nextexpr){
            if(e->term&&e->term->t==TERM_COLREF){
                if(e->term->ref->tableName){
                    arraylist_add(fieldlist,e->term->ref->tableName);
                }else{
                    arraylist_add(fieldlist,get_table_by_field(fcatFile,e->term->ref->columnName));
                }

            }
        }



        //printf("test");
        //SRA_print(project);
        conncat_sra(before,s->select.sra,NULL);
        //before=s->select.sra;
        SRA_t* tmp=s->select.sra;
        //SRA_print(project);



        int flag1=0;
        int flag2=0;
        if(fieldlist->size==1)
        {
            flag1=find_place(project,arraylist_get(fieldlist,0),path1);
            flag2=1;
        }else{
            flag1=find_place(project,arraylist_get(fieldlist,0),path1);
            flag2=find_place(project,arraylist_get(fieldlist,1),path2);
        }

        int common=0;
        if(flag1&&flag2){
            common=find_common_point(path1,path2);
        }
        SRA_t* point=arraylist_get(path1,common); //common 指向 select
        SRA_t* after=arraylist_get(path1,common+1);
        conncat_sra(s,after,NULL);
        conncat_sra(point,s,after);
        s=tmp;
        //SRA_print(project);

        //printf("test");
    }






    //printf("\nafter:\n");
    //SRA_print(project);
    return project;
}