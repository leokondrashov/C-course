#ifndef DSL_H

#define DSL_H

#define L node->left
#define R node->right

#define dL differentiate(L)
#define dR differentiate(R)
#define cL copy(L)
#define cR copy(R)

#define CONST(val) createNode(CONST, val, NULL, NULL)
#define VAR(val) createNode(VAR, val, NULL, NULL)
#define PLUS(n1, n2) createNode(OP, '+', n1, n2)
#define SUB(n1, n2) createNode(OP, '-', n1, n2)
#define MUL(n1, n2) createNode(OP, '*', n1, n2)
#define DIV(n1, n2) createNode(OP, '/', n1, n2)
#define SIN(n) createNode(OP, 's', NULL, n)
#define COS(n) createNode(OP, 'c', NULL, n)
#define SQRT(n) createNode(OP, 'S', NULL, n)
#define LN(n) createNode(OP, 'l', NULL, n)
#define EXP(n) createNode(OP, 'e', NULL, n)

#define IS_OP(n, value) (((n)->val->type == OP) && ((char) (n)->val->val == (value)))
#define IS_CONST_VAL(n, value) (((n)->val->type == CONST) && ((n)->val->val == (value)))
#define IS_CONST(n) ((((n)->left == NULL) || ((n)->left->val->type == CONST)) && \
((((n)->right == NULL) || ((n)->right->val->type == CONST)) && ((n)->val->type != VAR)))

#endif
