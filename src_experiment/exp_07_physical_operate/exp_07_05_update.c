//
// Created by sam on 2018/9/18.
//
#include "physicalplan/physicalplan.h"

/*执行 update 语句*/

/*TODO: plan_execute_update， update语句执行*/
int plan_execute_update(dongmendb *db, sql_stmt_update *sqlStmtUpdate , transaction *tx) {


    //fprintf(stderr, "TODO: update is not implemented yet. in plan_execute_update \n");
    physical_scan *plan = physical_scan_generate(db, sqlStmtUpdate->where, tx);
    int update_lines=0;

    plan->beforeFirst(plan);

    while(plan->next(plan))
    {

        for(size_t i=0;i<sqlStmtUpdate->fields->size;i++){
            char *fieldname=arraylist_get(sqlStmtUpdate->fields,i);
            variant *var=malloc(sizeof(variant));
            physical_scan_evaluate_expression(arraylist_get(sqlStmtUpdate->fieldsExpr,i),plan,var);
            if(var->type==DATA_TYPE_INT){
                plan->setInt(plan,sqlStmtUpdate->tableName,fieldname,var->intValue);
            }
            else if(var->type==DATA_TYPE_CHAR){
                plan->setString(plan,sqlStmtUpdate->tableName,fieldname,var->strValue);
            }
        }
        update_lines++;
    }
    plan->close(plan);




    return update_lines;
};