; To: kermit@columbia.edu 
; Subject: Small ASCII encoded comm program for DOS
; Date: Tue, 16 Apr 2002 16:41:37 -0600 (MDT)
; From: carruth@ece.utah.edu (Brent Carruth)
;
; I have put together ASM code that generates a small ASCII encoded comm
; program for DOS. It may prove useful to others and so I am sending it
; to you for inclusion in the MS-KERMIT software repository. Included below
; is a file which I have named TCOMTXT.ASM. If you need to rename the file,
; then a few lines in the commented source code need to reflect this change.
;
; Brent L. Carruth, Ph.D.
; ----------------------------------------------------------------------
; This program is useful in copying files to a computer using only the
; serial port and starting with only the COPY command.
;
; Use this program to generate TCOM.COM for MS-DOS with the commands:
;
;   TASM TCOMTXT
;   TLINK TCOMTXT /T
;   TCOMTXT > TCOM.COM
;
; The resulting TCOM.COM file is encoded with only printable 7-bit ASCII
; characters. It can be entered as plain text (shown below) and executed.
; TCOM is a 185 byte (with CRLFs) comm program that sets port 1 to 300
; baud and polls it. To set port 2 replace 'ss' with 'ts'. To set 2400 baud
; replace '26' with 'V'. On 25MHz+ computers 2400 baud works fine.
;
; XPHPD[0GG0G,0G51G31GB'(G+(G:u'0g?(G>(GE1G@arwIV_F*=US@<1|_,5wXNg-7muTu(4
; 1m0ss1k260s@3G1g360@3G0i7t2g3A1g350@3G2E1=0C1g350@3T2M0^\1g3>0@3T=1s2g0T
; 1g3;0@3ToN2g391g0t@3G0^F1k0s2?0@3T4
;
;
; The loader and builder portions of this program were written by
; Dan Norstedt, Stockholm, Sweden on 17 Feb 1990 who placed them in the
; public domain. He notes that they may be useful for other programs, too.
;
;
; The source code for the comm program portion of this program was modified
; from a debug listing of the TINYCOMM.COM program shown and explained in an
; article describing it (along with other related matters, including a tiny
; comm program in BASIC) in a 26911-byte file named TINYCOMM.TXT dated 13 Dec
; 1986. Unfortunately, no author is listed in this article. However, the
; author of the article is presumably Charles Petzold because, when assembled
; with debug, the TINYCOMM program described in the article is byte-for-byte
; identical to the first 49 bytes of the 103 byte TINYCOMM program dated
; 1 Jun 1988 from PC-MAG Utilities and the last 54 bytes includes the string
; ' by Charles Petzold - (c) 1988 Ziff Communications Co.'
;
;
; I have changed the source code for TINYCOMM in two trivial, but significant,
; ways related to file transfers. The first is that COM1 is initialized to 300
; baud but may be readily changed to either COM2 and/or 2400 baud. The second
; is that the program exits by pressing Esc. This makes it possible to use
; TCOM to download a file from another computer connected with a fully-wired
; null modem cable with 'TCOM > filename' and pressing Esc when done. I use
; MS-Kermit on the other computer to upload the file with the commands "set
; port 1, set speed 300, set local-echo on, set transmit line-feeds-sent on,
; set transmit prompt \0, transmit filename". To get TCOM on the first
; computer to begin with, carefully type it in using the standard DOS
; 'COPY CON TCOM.COM' command and pressing Ctrl-Z when done. MSBPCG.ASM is
; another program by Dan Norstedt that generates a 557 byte 7-bit ASCII encoded
; program named MSBPCT.COM that decodes standard Kermit bootstrap or "BOO"
; files. The first BOO file downloaded should be a small, reliable file trans-
; fer program. Thus, starting with a DOS boot disk having only the system files
; and COMMAND.COM it is possible to get other files copied to this disk using
; only the serial port and starting with only the DOS 'COPY' command.
;
;
; Brent L. Carruth, Ph.D.
; 9 April 2002

	CODE	SEGMENT PUBLIC

	ASSUME	CS:CODE,DS:CODE

	ORG	100H

START:	JMP	GENCOM

	ORG	100H+72+1	; Address where loader starts loading

CODARE:

;----------------------- beginning of PROGRAM code area -----------------------

;TCOM.ASM -- assemble with TASM v1.0
;58 byte communications program that polls COM1 at 300 baud.
;Esc key exits. Avoid Ctrl-P.
;Major portions attributable to Charles Petzold.
;
;Brent L. Carruth, Ph.D.
;18 April 2002
;
;CODE	SEGMENT	PARA
;	ASSUME	CS:CODE,DS:CODE
;	ORG	100H

BEGIN:	MOV	DX,0		; 0 FOR COM1 (1 FOR COM2)
	MOV	AX,43H		; SET PORT 300,N,8,1 (0A3H FOR 2400,N,8,1)
	INT	14H
TC_1:	MOV	AH,03		; GET COMM PORT STATUS
	INT	14H
	TEST	AH,01		; IF NO DATA READY, GO ON
	JZ	TC_2
	MOV	AH,02		; READ CHARACTER FROM COMM PORT
	INT	14H
	PUSH	DX
	MOV	DL,AL
	MOV	AH,02		; WRITE TO DISPLAY
	INT	21H
	POP	DX
	JMP	TC_1		; LOOP AROUND
TC_2:	MOV	AH,0BH		; CHECK KEYBOARD
	INT	21H
	OR	AL,AL
	JZ	TC_1
	MOV	AH,08		; IF CHAR, READ IT
	INT	21H
	CMP	AL,1BH		; IS IT ESC KEY ?
	JZ	QUIT		; YES, QUIT
	MOV	AH,01		; NO, SEND TO MODEM
	INT	14H
	JMP	TC_1		; LOOP AROUND
QUIT:	MOV	AX,4C00H	; EXIT TO DOS
	INT	21H
;CODE	ENDS
;	END	BEGIN

;-------------------------- end of PROGRAM code area --------------------------

CODEND:

;-- Loader starts here; don't touch unless you REALLY know what's going on --

FIXUP	MACRO	LABL,OFFS,DAT,DA2,DAS
	ORG	$+(OFFS)
LABL	Label	Byte
	IFNB	<DAS>
	 IFNB	<DA2>
	  DB	 (((DAS) OR 65H)-((DAS) AND 65H)) AND 0FFH,0FFH-(DA2)
	  ORG	 $-1H
	 ELSE
	  DB	 ((DAS)+65H) AND 0FFH
	 ENDIF
	ELSE
	 DB	0FFH-((DAT) AND 0FFH)
	 IFNB	<DA2>
	  DB	 0FFH-(DA2)
	  ORG	 $-1H
	 ENDIF
	ENDIF
	ORG	$-(OFFS)-1H
	ENDM

FIXBYT	MACRO	ADDR	 ; Generate "XOR [BX+ADDR-0FFH],AL" with 8-bit offset
	DB	30H,47H,(Offset ADDR-Offset LOADER+100H)-0FFH
	ENDM

FIXBYH	MACRO	ADDR	 ; Generate "XOR [BX+ADDR-0FFH],AH" with 8-bit offset
	DB	30H,67H,(Offset ADDR-Offset LOADER+100H)-0FFH
	ENDM

FIXWRD	MACRO	ADDR	 ; Generate "XOR [BX+ADDR-0FFH],AX" with 8-bit offset
	DB	31H,47H,(Offset ADDR-Offset LOADER+100H)-0FFH
	ENDM

FIXSUB	MACRO	ADDR	 ; Generate "SUB [BX+ADDR-0FFH],AL" with 8-bit offset
	DB	28H,47H,(Offset ADDR-Offset LOADER+100H)-0FFH
	ENDM

LOADER:	POP	AX
	PUSH	AX
	DEC	AX		; Load AX with 0FFFFH
	PUSH	AX		; Now FF FF 00 00 on stack
	INC	SP
	POP	BX		; Load BX with 00FF
	FIXBYT	XORBXX		; Fix up end of loader code
	FIXBYT	XORB1
	FIXBYT	XORB2
	FIXWRD	XORW1
	FIXWRD	XORW2
	DAA			; Load AL with 65H
	FIXSUB	SUBB1
	FIXSUB	SUBB2
	JNZ	J0JMP		; Break pipeline
J0DST:	FIXBYH	XORB3
	FIXSUB	SUBB3
	FIXSUB	SUBB4
	FIXWRD	XORX1

; FC
	CLD			; Set LODSB direction
	FIXUP	SUBB1,-1H,,,0FCH
; 8D7749  LEA	SI,[BX+DATA-0FFH] ; Point at 100H+72
	DB	8DH,77H,(Offset DATA-Offset LOADER+100H)-0FFH
	FIXUP	XORB1,-3H,8DH
; 56
	PUSH	SI		; Copy SI -> DI
; 5F
	POP	DI
; 46
	INC	SI
J2DST:
; 2AC2
	SUB	AL,DL		; Compute real code/data byte
; AA
	STOSB			; Save it
	FIXUP	XORW1,-2H,0C2H,0AAH
J1DST:
J3DST:
; AC
	LODSB			; Get an encoded data byte
	FIXUP	XORB2,-1H,0ACH
; 40
	INC	AX
; 3C31
	CMP	AL,31H		; Printable and >= '0'?
; 7CFA
	JL	J1DST
J1SRC:	FIXUP	SUBB2,-1H,,,J1DST-J1SRC
; 2C35
	SUB	AL,'4'+1	; Yes, '0'-'3' (or '4' = exit code) ?
; 77F3
	JA	J2DST		; No, store with current prefix code
J2SRC:	FIXUP	SUBB3,-1H,,,J2DST-J2SRC
; B102
	MOV	CL,2H
	FIXUP	XORB3,-2H,0B1H
; D2C8
	ROR	AL,CL
	FIXUP	XORX1,-3H,,0D2H,2H
; 92
	XCHG	DX,AX		; Yes, just save shifted value
	FIXUP	XORW2,-2H,0C8H,92H
; 75EF
	JNZ	J3DST		; No, contine loop
J3SRC:	FIXUP	SUBB4,-1H,,,J3DST-J3SRC
; 75D7
J0JMP:	JNZ	J0DST		; (Dummy branch used to clear prefetch queue)
J0SRC:	FIXUP	XORBXX,-1H,J0DST-J0SRC
	DB	34H		; Skip over next byte (34H = XOR AL,nn opcode)

DATA:	DB	"$"	    ; CRLF data to make sure JL J1DST taken first time

GENCOM:	PUSH	CS			; Allow use without EXE2BIN
	POP	DS
	MOV	DX,Offset LOADER	; Output LOADER code (!)
	MOV	AH,9H
	INT	21H
	CLD
	MOV	SI,Offset CODARE	; Pointer to real PROGRAM code
	XOR	BP,BP			; Reset columns left counter
	MOV	BH,17			; Assure that BH not in range '0'-'3'
BYTLOP:	MOV	AX,0C01H
	SUB	AL,[SI]			; Convert PROGRAM code to loader format
	INC	SI
	MOV	CL,2H
	SHL	AX,CL			; First byte is top 2 bits of byte+43H
	NOT	AL
	SHR	AL,CL			; Second byte is low 6 bits of -byte-3
	ADD	AL,35H			; Based value is '5' for second byte
	CMP	AH,BH			; Same prefix as previous byte?
	MOV	BH,AH
	XCHG	DX,AX
OUTBYT:	XCHG	DH,DL			; Swap output order
	JZ	OUTNHI			; Skip unnecessary prefix byte
	DEC	BP
	JG	OUTNCR			; Not 72 chars on the line yet
	PUSH	DX
	MOV	DX,Offset CRLF		; 72 chars on line, add CR LF
	MOV	AH,9H
	INT	21H
	MOV	BP,48H			; Restart line pointer
	POP	DX
OUTNCR:	MOV	AH,2H			; Output a byte
	INT	21H
OUTNHI:	XOR	DL,DL			; Clear out used code byte
	AND	DH,DH			; Anything more to print?
	JNZ	OUTBYT
	CMP	SI,Offset CODEND	; End of area?
	JNZ	BYTLOP
	MOV	DX,Offset ENDTXT	; Yes, add trailer: 34H and CR LF
	MOV	AH,9H
	INT	21H
	MOV	AH,4CH			; End GENCOM program section
	INT	21H

ENDTXT	DB	34H			; End of file marker for loader
CRLF	DB	0DH,0AH,"$"

CODE ENDS
	END	START

