//
// Created by Sam on 2018/2/13.
//

#include <parser/statement.h>
#include <parser/expression.h>

/**
 * 在现有实现基础上，实现update from子句
 *
 * 支持的update语法：
 *
 * UPDATE <table_name> SET <field1> = <expr1>[, <field2 = <expr2>, ..]
 * WHERE <logical_expr>
 *
 * 解析获得 sql_stmt_update 结构
 */

/*TODO: parse_sql_stmt_update， update语句解析*/
sql_stmt_update *parse_sql_stmt_update(ParserT *parser){
    //fprintf(stderr, "TODO: update is not implemented yet. in parse_sql_stmt_update \n");
    arraylist *fieldName=arraylist_create();
    arraylist *fieldexpr=arraylist_create();
    sql_stmt_update *update=malloc(sizeof(sql_stmt_update));


    TokenT *token=parseNextToken(parser);

    //update
    if(!matchToken(parser,TOKEN_RESERVED_WORD,"update")){
        return NULL;
    }
    token=parseNextToken(parser);


    //tablename
    if(token->type==TOKEN_WORD) {
        update->tableName = new_id_name();
        strcpy(update->tableName, token->text);
        token=parseEatAndNextToken(parser);
    }else{
        strcpy(parser->parserMessage,"invalid sql:missing table name.");
        return NULL;
    }

    //别名为空
    TableReference_t *ref1 =   TableReference_make(update->tableName, NULL);
    SRA_t *where =  SRATable(ref1);


    //set
    if(!matchToken(parser,TOKEN_RESERVED_WORD,"set"))
    {
        strcpy(parser->parserMessage,"invalid sql: missing set.");
        return  NULL;
    }

    //解析字段,直到遇到where保留字
    token=parseNextToken(parser);
    while(token->type ==TOKEN_WORD) {

        //fieldname
        arraylist_add(fieldName,token->text);
        token=parseEatAndNextToken(parser);


        if(!matchToken(parser,TOKEN_EQ,"="))
        {
            strcpy(parser->parserMessage,"invalid sql:missing =!");
            return NULL;
        }

        Expression *expr=parseExpressionRD(parser);
        arraylist_add(fieldexpr,expr);
        token=parseNextToken(parser);

        if(token!=NULL)
        {
            if(matchToken(parser,TOKEN_COMMA,","))
            {
                token=parseNextToken(parser);
                continue;
            }
        }
        else
        {
            break;//到达语句末尾
        }

        token=parseNextToken(parser);

    }


    Expression *whereExpr;
    if(token!=NULL&&matchToken(parser,TOKEN_RESERVED_WORD,"where")){
        //strcpy(parser->parserMessage,"invalid sql:missing where.");
        //return NULL;
        token = parseNextToken(parser);
        whereExpr = parseExpressionRD(parser); //where 后面的条件表达式
        where=SRASelect(where,whereExpr);
        if (parser->parserStateType == PARSER_WRONG) {
            return NULL;
        }
    }




    update->fields=fieldName;
    update->fieldsExpr=fieldexpr;
    SRA_print(where);
    update->where=where;
    return update;

};


