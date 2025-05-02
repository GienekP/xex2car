;-----------------------------------------------------------------------
;
; XEX2CAR
; (c)2025 GienekP
;
;-----------------------------------------------------------------------
BUF		= $2C
POS		= $2E
;-----------------------------------------------------------------------
CASINI  = $02
BOOTQ   = $09
DOSVEC  = $0A
DOSINI  = $0C
CRITIC  = $42
RAMTOP  = $6A
DMACTLS	= $022F
COLDST  = $0244
RUNAD   = $02E0
INITAD  = $02E2
MEMTOP  = $02E5
MEMLO	= $02E7
BASICF  = $03F8
GINTLK  = $03FA
RAMPROC = $0700
PORTB   = $D301
CASBUF	= $0400
TRIG3   = $D013
CONSOL  = $D01F
DMACTL	= $D400
VCOUNT	= $D40B
RESETCD = $E477
EDOPN   = $EF94
;-----------------------------------------------------------------------
; CARTRIDGE BANK 0

		OPT h-f+
;-----------------------------------------------------------------------
		ORG $A000
		
		dta $FF
;-----------------------------------------------------------------------
		ORG $BE00
		
STARTLD
.local SIOINT,RAMPROC
;----------------
		jsr THEEND
;----------------
		lda #<RETURN
		sta RUNAD
		lda #>RETURN
		sta RUNAD+1
;----------------
; START READ NEW DOS BLOCK
NEWBLK	ldx #$03
@		lda BUF,x
		sta STORE,x
		dex
		bpl @-
		lda #<RETURN
		sta INITAD
		lda #>RETURN
		sta INITAD+1
;----------------
; READ START & STOP
		jsr GETBYTE
		sta START
		sta POS
		jsr GETBYTE
		sta START+1
		sta POS+1
		jsr GETBYTE
		sta STOP
		jsr GETBYTE
		sta STOP+1
		inc STOP
		bne @+
		inc STOP+1	
;----------------
; CHECK EOF
@		ldx #$03
@		lda START,X
		cmp #$FF
		bne LOADBLK
		dex
		bpl	@-
;----------------
; RUNAD
		lda #>RESETCD
		pha
		lda #<RESETCD-1
		pha
		jsr RESTORE
		jmp (RUNAD)
;----------------
; READ DATA
LOADBLK	jsr GETBYTE
		ldy #$00
		sta (POS),y
		inc POS
		bne @+
		inc POS+1
@		lda POS
		cmp STOP
		bne LOADBLK
		lda POS+1
		cmp STOP+1
		bne LOADBLK
		lda #>NEWBLK
		pha
		lda #<NEWBLK-1
		pha
		jsr RESTORE
		jmp (INITAD)
;----------------
; RESTORE
RESTORE	ldx #$03
@		lda STORE,x
		sta BUF,x
		dex
		bpl @-
		rts
;----------------
; GET BYTE
GETBYTE	ldy PTR
		cpy #$80
		bne READYB
		lda #$00
		sta PTR
		lda #$72
@		cmp VCOUNT
		bne @-
		inc CRITIC
		ldx BANK
		sta $D500,x
		lda SECTOR
		sta BUF
		lda SECTOR+1
		sta BUF+1
		ldy #$00
@		lda (BUF),y
		sta CASBUF,y
		iny
		bpl @-
		sta $D5FF
		dec CRITIC
		lda #<CASBUF
		sta BUF
		lda #>CASBUF
		sta BUF+1
		ldy PTR
		lda SECTOR
		cmp #$00
		bne @+
		lda #$80
		sta SECTOR
		bmi READYB
@		lda #$00
		sta SECTOR
		inc SECTOR+1
		lda BANK
		bne @+
		lda SECTOR+1
		cmp #$BE
		bne READYB
		beq NEXTB
@		lda SECTOR+1
		cmp #$C0
		bne READYB
NEXTB	lda #$A0
		sta SECTOR+1
		inc BANK
READYB	lda (BUF),y
		inc PTR
RETURN	rts
;----------------
START	dta $FF,$FF
STOP	dta $FF,$FF
BANK	dta $00
SECTOR	dta $00,$A0
PTR		dta $80
STORE	dta $00,$00,$00,$00
;-----------------------------------------------------------------------
THEEND	sta $D5FF
		lda TRIG3
		sta GINTLK		
CLPRS	lda #$A0
		sta POS+1
		lda #$00
		sta POS
		ldy #$00
NEWPAG	lda #$00
@		sta (POS),Y
		iny
		bne @-
		inc POS+1
		lda POS+1
		cmp #$C0
		bne NEWPAG
		lda #<THEEND
		sta MEMLO
		lda #>THEEND
		sta MEMLO+1
		lda #$22
		sta DMACTLS
		jsr EDOPN
;----------------
; HASZOWNIK :)
		lda #$03
		sta $BFD4
		lda #$00
		sta $BC42
		sta COLDST
		lda #$10
@		cmp VCOUNT
		bne @-		
		rts
;-----------------------------------------------------------------------
.end
ENDLD
;-----------------------------------------------------------------------		
BEGIN	ldx #$FF
		txs
		lda #$00
		sta DMACTLS
		sta DMACTL
		sta COLDST
		lda PORTB
		ora #$02
		sta PORTB
		lda #$01
		sta BASICF
		lda #$1F
		sta MEMTOP
		lda #$BC
		sta MEMTOP+1
		lda #$C0
		sta RAMTOP
		lda #<RESETCD
		sta DOSVEC
		sta DOSINI
		sta CASINI	
		lda #>RESETCD
		sta DOSVEC+1
		sta DOSINI+1
		sta CASINI+1
		lda #$03
		sta BOOTQ
		lda #<STARTLD
		sta POS
		lda #>STARTLD
		sta POS+1
		lda #<RAMPROC
		sta BUF
		lda #>RAMPROC
		sta BUF+1
		ldy #$00
LOOP	lda (POS),y
		sta (BUF),y
		inc BUF
		bne @+
		inc BUF+1
@		inc POS
		bne @+
		inc POS+1
@		lda POS
		cmp #<ENDLD
		bne LOOP
		lda POS+1
		cmp #>ENDLD
		bne LOOP
		lda #$10
@		cmp VCOUNT
		bne @-
		lda #<CASBUF
		sta BUF
		lda #>CASBUF
		sta BUF+1
		clc
		jmp RAMPROC
;-----------------------------------------------------------------------		
; INITCART ROUTINE

		ORG $BFDB
			
INIT	lda CONSOL
		and #$02
		bne CONTIN
		ldx #(CONTIN-STANDR-1)
@		lda STANDR,X
		sta RAMPROC,x
		dex
		bpl @-
		jmp RAMPROC
STANDR  sta $D5FF
		jmp RESETCD
CONTIN	sta $D500
		rts
;-----------------------------------------------------------------------
; CARTRIDGE HEADER

		ORG $BFFA
		
		dta <BEGIN, >BEGIN, $00, $04, <INIT, >INIT
;-----------------------------------------------------------------------
