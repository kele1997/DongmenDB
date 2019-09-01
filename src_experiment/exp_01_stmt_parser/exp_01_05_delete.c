//
// Created by Sam on 2018/2/13.
//

#include <parser/statement.h>

/**
 * 在现有实现基础上，实现delete from子句
 *
 *  支持的delete语法：
 *
 *  DELETE FROM <table_nbame>
 *  WHERE <logical_expr>
 *
 * 解析获得 sql_stmt_delete 结构
 */

sql_stmt_delete *parse_sql_stmt_delete(ParserT *parser){
    char *tablename=new_id_name();
    sql_stmt_delete* deleter=calloc(1,sizeof(sql_stmt_delete));
    //sql_stmt_update* deleter=malloc(sizeof(sql_stmt_update));

    //printf("%x\n",deleter);
    TokenT *token=parseNextToken(parser);
    SRA_t *where=NULL;


    if(!matchToken(parser,TOKEN_RESERVED_WORD,"delete"))
    {
        strcpy(parser->parserMessage,"缺少 delete ！");
        return NULL;
    }

    /*
    if(!matchToken(parser,TOKEN_RESERVED_WORD,"from"))
    {
        strcpy(parser->parserMessage,"缺少 from ！");
        return NULL;
    }*/

    token=parseNextToken(parser);
    if(token->type==TOKEN_WORD){
        strcpy(tablename,token->text);
        deleter->tableName=tablename;
    }

    Expression *expr=NULL;
    token=parseEatAndNextToken(parser);
    if(token!=NULL&&matchToken(parser,TOKEN_RESERVED_WORD,"where")){
        expr=parseExpressionRD(parser);
    }

    TableReference_t *ref=TableReference_make(tablename,NULL);
    where=SRATable(ref);
    if(expr!=NULL){//其实不用检查也可以？，因为expr有可能为NULL
        where=SRASelect(where,expr);
    }

    deleter->tableName=tablename;
    deleter->where=where;
    return deleter;
};