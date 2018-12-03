DEF_OP(+, '+',
PLUS(dL, dR
))
DEF_OP(-, '-',
SUB(dL, dR
))
DEF_OP(*, '*',
PLUS(MUL(dL, cR), MUL(cL, dR)
))
DEF_OP(/, '/',
DIV(SUB(MUL(dL, cR), MUL(cL, dR)
),
MUL(cR, cR
)))
DEF_OP(sin,
's',
MUL(dR, COS(cR)
))
DEF_OP(cos,
'c',
MUL (CONST(
-1),
MUL(dR, SIN(cR)
)))
DEF_OP(sqrt,
'S',
DIV(dR, MUL(CONST(
2),
SQRT(cR)
)))
DEF_OP(ln,
'l',
DIV(dR, cR
))
DEF_OP(exp,
'e',
MUL(dR, EXP(cR)
))