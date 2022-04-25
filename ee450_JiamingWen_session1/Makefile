all: serverM.c serverA.c serverB.c serverC.c clientA.c clientB.c
		gcc -o serverM serverM.c
		gcc -o serverA serverA.c
		gcc -o serverB serverB.c
		gcc -o serverC serverC.c 
		gcc -o clientA clientA.c
		gcc -o clientB clientB.c

serverM:
		./serverM

serverA:
		./serverA

serverB:
		./serverB

serverC:
		./serverC

.PHONY: serverA serverB serverC serverM
