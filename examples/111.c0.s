.constants:
	0 S "main"
	1 I 2
	2 I 4
	3 I 3
	4 I 5
	5 I 0
	6 I 10

.start:

.functions:
	0 0 0 1

.F0:
	0 loadc 1
	1 je 12
	2 loadc 2
	3 iprint
	4 loadc 3
	5 loadc 1
	6 isub
	7 jg 11
	8 loadc 4
	9 iprint
	10 jmp 11
	11 jmp 12
	12 loadc 5
	13 iprint
	14 loadc 6
	15 iret
	16 popn 0
	17 ret
