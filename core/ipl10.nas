;haribote-ipl
;TAB=4
CYLS			EQU			10
	ORG				0x7c00					;start address
	JMP				entry
	DB				0x90
	DB				"HARIBOTE"				;boot section name 
	DW				512						;sector size must 512bytes
	DB				1						;cluster size must 1
	DW				1						;FAT start
	DB				2						;FAT count must 2
	DW				224						;root dir size usually 2
	DW				2880					;floppy size 2880 sectors
	DB				0xf0						;disk type
	DW				9						;FAT length must 9
	DW				18						; 1 track = 18 sectors
	DW				2						;heads must 2
	DD				0						;
	DD				2880					;
	DB				0,0,0x29					;
	DD				0xffffffff					;
	DB				"HARIBOTEOS "				;disk name 
	DB				"FAT12   "				;disk format 
	RESB			18						;
	
;key code
entry:
	MOV				AX,0						;init reg
	MOV				SS,AX
	MOV				SP,0X7C00
	MOV				DS,AX
;
	MOV				AX,0X0820
	MOV				ES,AX
	MOV				CH,0						;
	MOV				DH,0						; 
	MOV				CL,2
readloop:
	MOV				SI,0
retry:										;read 1 sector and try error
	MOV				AH,0x02
	MOV				AL,1
	MOV				BX,0
	MOV				DL,0X00
	INT				0X13
	JNC				next
	ADD				SI,1
	CMP				SI,5
	JAE				error
	MOV				AH,0X00
	MOV				DL,0X00
	INT				0X13
	JMP				retry
next:
	MOV				AX,ES
	ADD				AX,0X0020				;0x0020<<4=0x0200 512 bytes
	MOV				ES,AX					;read 18 sectors
	ADD				CL,1
	CMP				CL,18
	JBE				readloop
	MOV				CL,1						;18sectors then reverse side
	ADD				DH,1
	CMP				DH,2
	JB				readloop
	MOV				DH,0
	ADD				CH,1
	CMP				CH,CYLS
	JB				readloop
;	
	MOV				[0X0FF0],CH
	JMP				0XC200
error:
	MOV				SI,msg

putloop:
	MOV				AL,[SI]
	ADD				SI,1
	CMP				AL,0
	JE				fin
	MOV				AH,0X0E					;display
	MOV				BX,15
	INT				0X10					;call graphics card
	JMP				putloop
fin:
	HLT
	JMP				fin
msg:
	DB				0X0A,0X0A
	DB				"load error"
	DB				0X0A
	DB				0
	RESB			0X7DFE-$
	DB				0X55,0XAA
;	DB				0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
;	RESB			4600
;	DB				0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
;	RESB			1469432