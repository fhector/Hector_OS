; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 	
[BITS 32]						; 


; 

[FILE "naskfunc.nas"]				;

		GLOBAL	_io_hlt			; define func


; function body

[SECTION .text]					; section .text

_io_hlt:							; void io_hlt(void);
		HLT
		RET
