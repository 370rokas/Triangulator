	.586p
	ifdef ??version
	if    ??version GT 500H
	.mmx
	endif
	endif
	model flat
	ifndef	??version
	?debug	macro
	endm
	endif
	?debug	S "D:\proj\Triangulator\TriangulatorDLL\LZO\lzo_decompress.cpp"
	?debug	T "D:\proj\Triangulator\TriangulatorDLL\LZO\lzo_decompress.cpp"
_TEXT	segment dword public use32 'CODE'
_TEXT	ends
_DATA	segment dword public use32 'DATA'
_DATA	ends
_BSS	segment dword public use32 'BSS'
_BSS	ends
$$BSYMS	segment byte public use32 'DEBSYM'
$$BSYMS	ends
$$BTYPES	segment byte public use32 'DEBTYP'
$$BTYPES	ends
$$BNAMES	segment byte public use32 'DEBNAM'
$$BNAMES	ends
$$BROWSE	segment byte public use32 'DEBSYM'
$$BROWSE	ends
$$BROWFILE	segment byte public use32 'DEBSYM'
$$BROWFILE	ends
DGROUP	group	_BSS,_DATA
_TEXT	segment dword public use32 'CODE'
@lzo1x_decompress_asm_fast$qpucuit1puit1	segment virtual
@@lzo1x_decompress_asm_fast$qpucuit1puit1	proc	near
?live16385@0:
	?debug L 2
	?debug L 7
@1:
	push	    ebp
	?debug L 8
	push	    edi
	?debug L 9
	push	    esi
	?debug L 10
	push	    ebx
	?debug L 11
	push	    ecx
	?debug L 12
	push	    edx
	?debug L 13
	sub	     esp,0000000cH
	?debug L 14
	cld	
	?debug L 15
	mov	     esi,[esp + 28H]
	?debug L 16
	mov	     edi,[esp + 30H]
	?debug L 17
	mov	     ebp,00000003H
	?debug L 18
	xor	     eax,eax
	?debug L 19
	xor	     ebx,ebx
	?debug L 20
	lodsb	
	?debug L 21
	cmp	     al,11H
	?debug L 22
	jbe       @2
	?debug L 23
	sub	     al,0eH
	?debug L 24
	jmp       @3
	?debug L 25
@4:
L3:
             	add	     eax,000000ffH
	?debug L 26
@5:
L4:
             	mov	     bl,[esi]
	?debug L 27
	inc	     esi
	?debug L 28
	or	      bl,bl
	?debug L 29
	je        @4
	?debug L 30
	lea	     eax,[eax + ebx + 15H]
	?debug L 31
	jmp       @3
	?debug L 32
	mov	     esi,esi
	?debug L 33
@6:
L5:
             	mov	     al,[esi]
	?debug L 34
	inc	     esi
	?debug L 35
@2:
L6:
             	cmp	     al,10H
	?debug L 36
	jae       @7
	?debug L 37
	or	      al,al
	?debug L 38
	je        @5
	?debug L 39
	add	     eax,00000006H
	?debug L 40
@3:
L7:
             	mov	     ecx,eax
	?debug L 41
	xor	     eax,ebp
	?debug L 42
	shr	     ecx,02H
	?debug L 43
	and	     eax,ebp
	?debug L 44
@8:
L8:
             	mov	     edx,[esi]
	?debug L 45
	add	     esi,00000004H
	?debug L 46
	mov	     [edi],edx
	?debug L 47
	add	     edi,00000004H
	?debug L 48
	dec	     ecx
	?debug L 49
	jne       @8
	?debug L 50
	sub	     esi,eax
	?debug L 51
	sub	     edi,eax
	?debug L 52
	mov	     al,[esi]
	?debug L 53
	inc	     esi
	?debug L 54
	cmp	     al,10H
	?debug L 55
	jae       @7
	?debug L 56
	shr	     eax,02H
	?debug L 57
	mov	     bl,[esi]
	?debug L 58
	lea	     edx,[edi - 801H]
	?debug L 59
	lea	     eax,[eax + ebx*4]
	?debug L 60
	inc	     esi
	?debug L 61
	sub	     edx,eax
	?debug L 62
	mov	     ecx,[edx]
	?debug L 63
	mov	     [edi],ecx
	?debug L 64
	add	     edi,ebp
	?debug L 65
	jmp       @9
	?debug L 66
@7:
L9:
             	cmp	     al,40H
	?debug L 67
	jb        @10
	?debug L 68
	mov	     ecx,eax
	?debug L 69
	shr	     eax,02H
	?debug L 70
	lea	     edx,[edi - 1H]
	?debug L 71
	and	     eax,00000007H
	?debug L 72
	mov	     bl,[esi]
	?debug L 73
	shr	     ecx,05H
	?debug L 74
	lea	     eax,[eax + ebx*8]
	?debug L 75
	inc	     esi
	?debug L 76
	sub	     edx,eax
	?debug L 77
	add	     ecx,00000004H
	?debug L 78
	cmp	     eax,ebp
	?debug L 79
	jae       @11
	?debug L 80
	jmp       @12
	?debug L 81
@13:
L10:
            	add	     eax,000000ffH
	?debug L 82
@14:
L11:
            	mov	     bl,[esi]
	?debug L 83
	inc	     esi
	?debug L 84
	or	      bl,bl
	?debug L 85
	je        @13
	?debug L 86
	lea	     ecx,[eax + ebx + 24H]
	?debug L 87
	xor	     eax,eax
	?debug L 88
	jmp       @15
	?debug L 89
	nop	
	?debug L 90
@10:
L12:
            	cmp	     al,20H
	?debug L 91
	jb        @16
	?debug L 92
	and	     eax,0000001fH
	?debug L 93
	je        @14
	?debug L 94
	lea	     ecx,[eax + 5H]
	?debug L 95
@15:
L13:
            	mov	     ax,[esi]
	?debug L 96
	lea	     edx,[edi - 1H]
	?debug L 97
	shr	     eax,02H
	?debug L 98
	add	     esi,00000002H
	?debug L 99
	sub	     edx,eax
	?debug L 100
	cmp	     eax,ebp
	?debug L 101
	jb        @12
	?debug L 102
@11:
L14:
            	lea	     eax,[edi + ecx - 3H]
	?debug L 103
	shr	     ecx,02H
	?debug L 104
@17:
L15:
            	mov	     ebx,[edx]
	?debug L 105
	add	     edx,00000004H
	?debug L 106
	mov	     [edi],ebx
	?debug L 107
	add	     edi,00000004H
	?debug L 108
	dec	     ecx
	?debug L 109
	jne       @17
	?debug L 110
	mov	     edi,eax
	?debug L 111
	xor	     ebx,ebx
	?debug L 112
@9:
L16:
            	mov	     al,[esi - 2H]
	?debug L 113
	and	     eax,ebp
	?debug L 114
	je        @6
	?debug L 115
	mov	     edx,[esi]
	?debug L 116
	add	     esi,eax
	?debug L 117
	mov	     [edi],edx
	?debug L 118
	add	     edi,eax
	?debug L 119
	mov	     al,[esi]
	?debug L 120
	inc	     esi
	?debug L 121
	jmp       @7
	?debug L 122
	lea	     esi,[esi + 0H]
	?debug L 123
@12:
L17:
            	xchg	    edx,esi
	?debug L 124
	sub	     ecx,ebp
	?debug L 125
	repe    movsb	
	?debug L 126
	mov	     esi,edx
	?debug L 127
	jmp       @9
	?debug L 128
@18:
L18:
            	add	     ecx,000000ffH
	?debug L 129
@19:
L19:
            	mov	     bl,[esi]
	?debug L 130
	inc	     esi
	?debug L 131
	or	      bl,bl
	?debug L 132
	je        @18
	?debug L 133
	lea	     ecx,[ebx + ecx + 0cH]
	?debug L 134
	jmp       @20
	?debug L 135
	lea	     esi,[esi + 0H]
	?debug L 136
@16:
L20:
            	cmp	     al,10H
	?debug L 137
	jb        @21
	?debug L 138
	mov	     ecx,eax
	?debug L 139
	and	     eax,00000008H
	?debug L 140
	shl	     eax,0dH
	?debug L 141
	and	     ecx,00000007H
	?debug L 142
	je        @19
	?debug L 143
	add	     ecx,00000005H
	?debug L 144
@20:
L21:
            	mov	     ax,[esi]
	?debug L 145
	add	     esi,00000002H
	?debug L 146
	lea	     edx,[edi - 4000H]
	?debug L 147
	shr	     eax,02H
	?debug L 148
	je        @22
	?debug L 149
	sub	     edx,eax
	?debug L 150
	jmp       @11
	?debug L 151
	lea	     esi,[esi + 0H]
	?debug L 152
@21:
L22:
            	shr	     eax,02H
	?debug L 153
	mov	     bl,[esi]
	?debug L 154
	lea	     edx,[edi - 1H]
	?debug L 155
	lea	     eax,[eax + ebx*4]
	?debug L 156
	inc	     esi
	?debug L 157
	sub	     edx,eax
	?debug L 158
	mov	     al,[edx]
	?debug L 159
	mov	     [edi],al
	?debug L 160
	mov	     bl,[edx + 1H]
	?debug L 161
	mov	     [edi +1H ],bl
	?debug L 162
	add	     edi,00000002H
	?debug L 163
	jmp       @9
	?debug L 164
@22:
L23:
            	cmp	     ecx,00000006H
	?debug L 165
	setne	   al
	?debug L 166
	mov	     edx,[esp + 28H]
	?debug L 167
	add	     edx,[esp + 2cH]
	?debug L 168
	cmp	     esi,edx
	?debug L 169
	ja        @23
	?debug L 170
	jb        @24
	?debug L 171
@25:
L24:
            	sub	     edi,[esp + 30H]
	?debug L 172
	mov	     edx,[esp + 34H]
	?debug L 173
	mov	     [edx],edi
	?debug L 174
	neg	     eax
	?debug L 175
	add	     esp,0000000cH
	?debug L 176
	pop	     edx
	?debug L 177
	pop	     ecx
	?debug L 178
	pop	     ebx
	?debug L 179
	pop	     esi
	?debug L 180
	pop	     edi
	?debug L 181
	pop	     ebp
	?debug L 182
	ret	
	?debug L 183
	mov	     eax,00000001H
	?debug L 184
	jmp       @25
	?debug L 185
@24:
L25:
            	mov	     eax,00000008H
	?debug L 186
	jmp       @25
	?debug L 187
@23:
L26:
            	mov	     eax,00000004H
	?debug L 188
	jmp       @25
	?debug L 189
	nop	
	?debug L 191
@26:
	?debug L 0
@@lzo1x_decompress_asm_fast$qpucuit1puit1	endp
@lzo1x_decompress_asm_fast$qpucuit1puit1	ends
_TEXT	ends
$$BSYMS	segment byte public use32 'DEBSYM'
	db	2
	db	0
	db	0
	db	0
	dw	87
	dw	517
	dw	0
	dw	0
	dw	0
	dw	0
	dw	0
	dw	0
	dd	?patch1
	dd	?patch2
	dd	?patch3
	df	@@lzo1x_decompress_asm_fast$qpucuit1puit1
	dw	0
	dw	4096
	dw	0
	dw	1
	dw	0
	dw	0
	dw	0
	db	40
	db	64
	db	108
	db	122
	db	111
	db	49
	db	120
	db	95
	db	100
	db	101
	db	99
	db	111
	db	109
	db	112
	db	114
	db	101
	db	115
	db	115
	db	95
	db	97
	db	115
	db	109
	db	95
	db	102
	db	97
	db	115
	db	116
	db	36
	db	113
	db	112
	db	117
	db	99
	db	117
	db	105
	db	116
	db	49
	db	112
	db	117
	db	105
	db	116
	db	49
	dw	18
	dw	512
	dw	8
	dw	0
	dw	1056
	dw	0
	dw	2
	dw	0
	dw	0
	dw	0
	dw	18
	dw	512
	dw	12
	dw	0
	dw	117
	dw	0
	dw	3
	dw	0
	dw	0
	dw	0
	dw	18
	dw	512
	dw	16
	dw	0
	dw	1056
	dw	0
	dw	4
	dw	0
	dw	0
	dw	0
	dw	18
	dw	512
	dw	20
	dw	0
	dw	1141
	dw	0
	dw	5
	dw	0
	dw	0
	dw	0
	dw	18
	dw	512
	dw	24
	dw	0
	dw	1056
	dw	0
	dw	6
	dw	0
	dw	0
	dw	0
?patch1	equ	@26-@@lzo1x_decompress_asm_fast$qpucuit1puit1
?patch2	equ	0
?patch3	equ	@26-@@lzo1x_decompress_asm_fast$qpucuit1puit1
	dw	2
	dw	6
$$BSYMS	ends
_TEXT	segment dword public use32 'CODE'
_TEXT	ends
$$BSYMS	segment byte public use32 'DEBSYM'
	dw	?patch4
	dw	1
	db	5
	db	1
	db	0
	db	24
	db	11
	db	66
	db	67
	db	67
	db	51
	db	50
	db	32
	db	53
	db	46
	db	53
	db	46
	db	49
?patch4	equ	18
$$BSYMS	ends
$$BTYPES	segment byte public use32 'DEBTYP'
	db        2,0,0,0,14,0,8,0,116,0,0,0,0,0,5,0
	db        1,16,0,0,24,0,1,2,5,0,32,4,0,0,117,0
	db        0,0,32,4,0,0,117,4,0,0,32,4,0,0
$$BTYPES	ends
$$BNAMES	segment byte public use32 'DEBNAM'
	db	25,'lzo1x_decompress_asm_fast'
	db	3,'src'
	db	7,'src_len'
	db	3,'dst'
	db	7,'dst_len'
	db	6,'wrkmem'
$$BNAMES	ends
	?debug	D "D:\proj\Triangulator\TriangulatorDLL\LZO\lzo_decompress.cpp" 12183 35546
	end
