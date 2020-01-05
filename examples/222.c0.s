.constants:
	0 I 100
	1 S "main"
	2 I 3
	3 I 10000
	4 I 0
	5 I 1
	6 I 10

.start:
	0 loadc 0

.functions:
	0 1 0 1

.F0:
	0 loadc 2
	1 loadc 3
	2 ipush 0
	3 loada 0 ,0
	4 iload
	5 loadc 4
	6 isub
	7 jle 38
	8 loada 0 ,2
	9 iscan
	10 istore
	11 loada 0 ,2
	12 iload
	13 iprint
	14 loada 0 ,2
	15 iload
	16 loada 0 ,1
	17 iload
	18 isub
	19 jge 25
	20 loada 0 ,1
	21 loada 0 ,2
	22 iload
	23 istore
	24 jmp 25
	25 loada 0 ,1
	26 iload
	27 iprint
	28 loada 0 ,0
	29 loada 0 ,0
	30 iload
	31 loadc 5
	32 isub
	33 istore
	34 loada 0 ,0
	35 iload
	36 iprint
	37 jmp 3
	38 loada 0 ,1
	39 iload
	40 iprint
	41 loadc 6
	42 iret
	43 popn 3
	44 ret
