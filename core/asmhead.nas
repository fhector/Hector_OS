; haribote-os boot asm
; TAB=4

BOTPAK	EQU		0x00280000		; bootpack�̃��[�h��
DSKCAC	EQU		0x00100000		; �f�B�X�N�L���b�V���̏ꏊ
DSKCAC0	EQU		0x00008000		; �f�B�X�N�L���b�V���̏ꏊ�i���A�����[�h�j

; BOOT_INFO�֌W
CYLS	EQU		0x0ff0			; �u�[�g�Z�N�^���ݒ肷��
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; �F���Ɋւ�����B���r�b�g�J���[���H
SCRNX	EQU		0x0ff4			; �𑜓x��X
SCRNY	EQU		0x0ff6			; �𑜓x��Y
VRAM	EQU		0x0ff8			; �O���t�B�b�N�o�b�t�@�̊J�n�Ԓn

		ORG		0xc200			; ���̃v���O�������ǂ��ɓǂݍ��܂��̂�

; ��ʃ��[�h��ݒ�

		MOV		AL,0x13			; VGA�O���t�B�b�N�X�A320x200x8bit�J���[
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	; ��ʃ��[�h����������iC���ꂪ�Q�Ƃ���j
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000

; �L�[�{�[�h��LED��Ԃ�BIOS�ɋ����Ă��炤

		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

; PIC����؂̊��荞�݂��󂯕t���Ȃ��悤�ɂ���
;	AT�݊��@�̎d�l�ł́APIC�̏�����������Ȃ�A
;	������CLI�O�ɂ���Ă����Ȃ��ƁA���܂Ƀn���O�A�b�v����
;	PIC�̏������͂��Ƃł��

		MOV		AL,0xff
		OUT		0x21,AL			; Prohibit master PIC interrupt
		NOP						; some machine can not out then out,so add nop opration
		OUT		0xa1,AL			; Prohibit slave PIC interrupt

		CLI						; Prohibit the CPU interrupt
; equivalence
;io_out(	PIC0_IMR,	0XFF)
;io_out(	PIC0_IMR,	0XFF);
;io_cli();


; for CPU can access mm > 1MB, set A20GATE

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

;Switch to protected mode

[INSTRSET "i486p"]				; Want to use the 486 instructions eg. LGDT,EAX,CR0

		LGDT	[GDTR0]			; set temp GDT
		MOV		EAX,CR0			; Control Register 0
		AND		EAX,0x7fffffff		; bit 31 =0
		OR		EAX,0x00000001	; set bit 0 =1 for switch to protected mode
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			; WR segment 32bit    0x0008
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX			; except CS,  0x0008 means "gtd+1" segment

;transfer bootpack

		MOV		ESI,bootpack		; source
		MOV		EDI,BOTPAK		; destination
		MOV		ECX,512*1024/4
		CALL	memcpy

;Disk data eventually transferred to its original position

;First of all, starting from the boot sector

		MOV		ESI,0x7c00		; source
		MOV		EDI,DSKCAC		; destination
		MOV		ECX,512/4
		CALL	memcpy

; All remaining

		MOV		ESI,DSKCAC0+512	;
		MOV		EDI,DSKCAC+512	; 
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; From the cylinder number transform for the number of bytes/4
		SUB		ECX,512/4		; minus IPL
		CALL	memcpy

; equivalence
;memcpy(bootpack,	BOTPAK, 		512*512/4	);
;memcpy(0x7c00,		BOTPAK, 		512/4		);
;memcpy(DSKCAC0,	DSKCAC+512, CYLS*512*18*2/4-512/4);




; the work must do by asmhead have been finished.
; Now to bootpack to deal with

; bootpack starts

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip				; noting to transmit
		MOV		ESI,[EBX+20]		; source
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]		; destination
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]		; stack initial value
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; AND result if not 0, jump waitkbdout
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 
		RET
; memcpy

		ALIGNB	16
GDT0:
		RESB	8				; NULL selector  (the number 0 segment)
		DW		0xffff,0x0000,0x9200,0x00cf	; Read&Write segment 32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; Execute segment 32bit (for bootpack  use)

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
