#include "include/parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Parses an if statement, including its condition, true branch,
 * and optional else or else-if branch.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope used to parse the condition and branches.
 *
 * Returns:
 *   The AST node representing the if statement.
 */

ast_T* parserParseIfStatement(parser_T* parser, scope_T* scope) {
    ast_T* ast = astInit(AST_IF_STATEMENT);
    ast->scope = scope;

    // Consume "if".
    parserEat(parser, TOKEN_ID);

    // Parse the condition: if (condition)
    parserEat(parser, TOKEN_LEFTPAREN);
    ast->ifCondition = parserParseComparison(parser, scope);
    parserEat(parser, TOKEN_RIGHTPAREN);

    // Parse the true branch.
    parserEat(parser, TOKEN_LEFTBRACE);
    ast->ifBody = parserParseStatements(parser, scope);
    parserEat(parser, TOKEN_RIGHTBRACE);

    // Parse an optional else or else-if branch.
    if (
        parser->currentToken->type == TOKEN_ID &&
        strcmp(parser->currentToken->value, "else") == 0
    ) {
        parserEat(parser, TOKEN_ID);

        if (
            parser->currentToken->type == TOKEN_ID &&
            strcmp(parser->currentToken->value, "if") == 0
        ) {
            // An else-if is another if statement.
            ast->elseBody = parserParseIfStatement(parser, scope);
        }
        else {
            // A regular else block.
            parserEat(parser, TOKEN_LEFTBRACE);
            ast->elseBody = parserParseStatements(parser, scope);
            parserEat(parser, TOKEN_RIGHTBRACE);
        }
    }

    return ast;
}

/*
 * Parses a while loop, including its condition and body.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope used to parse the condition and loop body.
 *
 * Returns:
 *   The AST node representing the while loop.
 */

ast_T* parserParseWhileLoop(parser_T* parser, scope_T* scope) {
	ast_T* ast = astInit(AST_WHILE_LOOP);
	ast->scope = scope;
	
	parserEat(parser, TOKEN_ID);	// consume while
	
	// Parse the condition: while (condition)
    parserEat(parser, TOKEN_LEFTPAREN);
    ast->whileCondition = parserParseComparison(parser, scope);
    parserEat(parser, TOKEN_RIGHTPAREN);

    // Parse the while condition body.
    parserEat(parser, TOKEN_LEFTBRACE);
    ast->whileBody = parserParseStatements(parser, scope);
    parserEat(parser, TOKEN_RIGHTBRACE);
    
    return ast;
}

/*
 * Parses a for loop, including its initializer, condition,
 * update expression, and body.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Parent scope of the loop's dedicated scope.
 *
 * Returns:
 *   The AST node representing the for loop.
 */

ast_T* parserParseForLoop(parser_T* parser, scope_T* scope) {
    ast_T* ast = astInit(AST_FOR_LOOP);
    ast->scope = scope;

    // Create a scope dedicated to this loop.
    scope_T* loopScope = scopeInitChild(scope);
	ast->forScope = loopScope;

    // Consume "for".
    parserEat(parser, TOKEN_ID);
    parserEat(parser, TOKEN_LEFTPAREN);

    // Initializer: int i = 0
    ast->forInitializer = parserParseVariableDefinition(
        parser,
        loopScope
    );
    parserEat(parser, TOKEN_SEMI);

    // Condition: i < 5
    ast->forCondition = parserParseComparison(
        parser,
        loopScope
    );
    parserEat(parser, TOKEN_SEMI);

    // Update: i = i + 1
    ast->forUpdate = parserParseAssignment(
        parser,
        loopScope
    );
    parserEat(parser, TOKEN_RIGHTPAREN);

    // Loop body.
    parserEat(parser, TOKEN_LEFTBRACE);
    ast->forBody = parserParseStatements(parser, loopScope);
    parserEat(parser, TOKEN_RIGHTBRACE);

    return ast;
}
