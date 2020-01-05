.constants:
	0 I 42
	1 S "main"
	2 I 10
	3 I 40
	4 I 34
	5 I 100

.start:
	0 loadc 0

.functions:
	0 1 0 1

.F0:
	0 loadc 2
	1 loada 1 ,0
	2 iload
	3 loadc 3
	4 isub
	5 jle 13
	6 loadc 4
	7 iprint
	8 loada 1 ,0
	9 loada 0 ,0
	10 iload
	11 istore
	12 jmp 1
	13 loadc 5
	14 iprint
	15 loadc 2
	16 iret
	17 popn 1
	18 ret
