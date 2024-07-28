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
	?debug	S "D:\proj\Triangulator\TriangulatorDLL\FLZ\FLZDecoder.cpp"
	?debug	T "D:\proj\Triangulator\TriangulatorDLL\FLZ\FLZDecoder.cpp"
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
_BSS	segment dword public use32 'BSS'
	align	4
$mobkbaka	label	dword
	db	4	dup(?)
	align	4
$ecckbaka	label	dword
	db	4	dup(?)
	align	4
$mfckbaka	label	dword
	db	4	dup(?)
	align	4
$ejckbaka	label	dword
	db	4	dup(?)
	align	4
$mmckbaka	label	dword
	db	4	dup(?)
	align	4
$eadkbaka	label	dword
	db	4	dup(?)
	align	4
$mddkbaka	label	dword
	db	4	dup(?)
	align	4
$ehdkbaka	label	dword
	db	4	dup(?)
	align	4
$mkdkbaka	label	dword
	db	4	dup(?)
_BSS	ends
_DATA	segment dword public use32 'DATA'
	align	4
$eodkbaka	label	dword
	dd	1026
	dd	1283
	dd	1284
_DATA	ends
_TEXT	segment dword public use32 'CODE'
@FLZDecompressMemory$qpuct1ii	segment virtual
@@FLZDecompressMemory$qpuct1ii	proc	near
?live16385@0:
	?debug L 4
	push      ebp
	mov       ebp,esp
	add       esp,-8
	push      ebx
	push      esi
	push      edi
	?debug L 13
@1:
	mov       dword ptr [$mobkbaka],4
	?debug L 14
	mov       dword ptr [$ecckbaka],2
	?debug L 15
	mov       dword ptr [$mfckbaka],8
	?debug L 16
	xor       eax,eax
	mov       dword ptr [$ejckbaka],eax
	?debug L 17
	xor       edx,edx
	mov       dword ptr [$mmckbaka],edx
	?debug L 18
	mov       ecx,dword ptr [ebp+20]
	mov       dword ptr [$eadkbaka],ecx
	?debug L 19
	mov       eax,dword ptr [ebp+12]
	mov       dword ptr [$mddkbaka],eax
	?debug L 20
	mov       edx,dword ptr [ebp+8]
	mov       dword ptr [$ehdkbaka],edx
	?debug L 22
	mov       ecx,dword ptr [$ehdkbaka]
	mov       al,byte ptr [ecx]
	mov       byte ptr [ebp-1],al
	inc       dword ptr [$ehdkbaka]
	?debug L 24
	test      byte ptr [ebp-1],1
	je        short @2
	?debug L 26
	mov       edx,dword ptr [$ehdkbaka]
	mov       ecx,dword ptr [edx]
	mov       eax,dword ptr [$ehdkbaka]
	add       ecx,eax
	mov       dword ptr [$mkdkbaka],ecx
	?debug L 27
	add       dword ptr [$ehdkbaka],4
	?debug L 28
@3:
	mov       edx,dword ptr [$mkdkbaka]
	xor       ecx,ecx
	mov       cl,byte ptr [edx]
	inc       ecx
	mov       dword ptr [ebp-8],ecx
	inc       dword ptr [$mkdkbaka]
	?debug L 29
@4:
	jmp       short @5
	?debug L 31
@2:
	xor       eax,eax
	mov       dword ptr [$mkdkbaka],eax
	?debug L 33
@5:
	test      byte ptr [ebp-1],2
	je        short @6
	?debug L 34
	mov       dword ptr [$eodkbaka+4],1283
	jmp       short @7
	?debug L 36
@6:
	mov       dword ptr [$eodkbaka+4],1027
	?debug L 38
@7:
	test      byte ptr [ebp-1],4
	je        short @8
	?debug L 40
	mov       dword ptr [$eodkbaka],1027
	?debug L 41
	mov       dword ptr [$ecckbaka],3
	?debug L 42
	jmp       short @9
	?debug L 45
@8:
	mov       dword ptr [$eodkbaka],1026
	?debug L 46
	mov       dword ptr [$ecckbaka],2
	?debug L 50
@9:
	push		ebp
	?debug L 51
	mov		esi,dword ptr $ehdkbaka
	?debug L 52
	mov		edi,dword ptr $mddkbaka
	?debug L 53
	mov		ebx,[esi]
	?debug L 54
	add		dword ptr $ehdkbaka,4
	?debug L 55
	mov		ebp,[esi+4]
	?debug L 56
	add		dword ptr $ehdkbaka,4
	?debug L 57
	bswap		ebx
	?debug L 58
	bswap		ebp
	?debug L 59
	xor		edx,edx
	?debug L 60
	cld	
	?debug L 62
@10:
MainLoop:
	?debug L 67
	mov		ecx,edx			
	?debug L 68
	shr		ebx,cl
	?debug L 69
	shld		ebx,ebp,cl
	?debug L 70
	shr		edx,8
	?debug L 71
	shl		ebp,cl
	?debug L 72
	add		dl,cl
	?debug L 74
	cmp		edx,32
	?debug L 75
	jb        @11
	?debug L 77
	mov		eax,dword ptr $ehdkbaka
	?debug L 78
	mov		ebp,[eax]
	?debug L 79
	add		dword ptr $ehdkbaka,4
	?debug L 80
	bswap		ebp
	?debug L 82
	sub		edx,32
	?debug L 83
	mov		ecx,edx
	?debug L 84
	shr		ebx,cl
	?debug L 85
	shld		ebx,ebp,cl
	?debug L 86
	shl		ebp,cl
	?debug L 87
@11:
_LoadData1:
	?debug L 88
	shl		edx,8
	?debug L 91
	inc		edx
	?debug L 92
	shl		ebx,1 
	?debug L 93
	jae       @12
	?debug L 96
	mov		ecx,dword ptr $mobkbaka
	?debug L 97
	add		edx,ecx
	?debug L 98
	mov		eax,ebx
	?debug L 99
	shl		ebx,cl
	?debug L 100
	sub		ecx,32
	?debug L 101
	neg		ecx
	?debug L 102
	shr		eax,cl
	?debug L 103
	mov		ecx,eax
	?debug L 106
	xor		esi,esi
	?debug L 107
	test		ecx,ecx
	?debug L 108
	je        short @13
	?debug L 109
	dec		ecx
	?debug L 110
	inc		esi
	?debug L 111
	shld		esi,ebx,cl
	?debug L 112
	shl		ebx,cl
	?debug L 113
	add		edx,ecx
	?debug L 114
@13:
M_1:
	?debug L 119
	mov		ecx,edx			
	?debug L 120
	shr		ebx,cl
	?debug L 121
	shld		ebx,ebp,cl
	?debug L 122
	shr		edx,8
	?debug L 123
	shl		ebp,cl
	?debug L 124
	add		dl,cl
	?debug L 126
	cmp		edx,32
	?debug L 127
	jb        @14
	?debug L 129
	mov		eax,dword ptr $ehdkbaka
	?debug L 130
	mov		ebp,[eax]
	?debug L 131
	add		dword ptr $ehdkbaka,4
	?debug L 132
	bswap		ebp
	?debug L 134
	sub		edx,32
	?debug L 135
	mov		ecx,edx
	?debug L 136
	shr		ebx,cl
	?debug L 137
	shld		ebx,ebp,cl
	?debug L 138
	shl		ebp,cl
	?debug L 139
@14:
_LoadData2:
	?debug L 140
	shl		edx,8
	?debug L 143
	cmp		esi,2
	?debug L 144
	je        @15
	?debug L 145
	cmp		esi,3
	?debug L 146
	je        @15
	?debug L 147
	cmp		esi,4
	?debug L 148
	je        @15
	?debug L 151
	cmp		esi,1
	?debug L 152
	je        @16
	?debug L 155
	mov		ecx,dword ptr $ecckbaka
	?debug L 156
	add		edx,ecx
	?debug L 157
	mov		eax,ebx
	?debug L 158
	shl		ebx,cl
	?debug L 159
	sub		ecx,32
	?debug L 160
	neg		ecx
	?debug L 161
	shr		eax,cl
	?debug L 162
	mov		ecx,eax
	?debug L 165
	xor		eax,eax
	?debug L 166
	test		ecx,ecx
	?debug L 167
	je        short @17
	?debug L 168
	dec		ecx
	?debug L 169
	inc		eax
	?debug L 170
	shld		eax,ebx,cl
	?debug L 171
	shl		ebx,cl
	?debug L 172
	add		edx,ecx
	?debug L 173
@17:
M_2:
	?debug L 176
	cmp		esi,5
	?debug L 177
	je        @18
	?debug L 180
	cmp		esi,0
	?debug L 181
	jne       @19
	?debug L 184
	xor		ecx,ecx
	?debug L 185
	add		edx,3
	?debug L 186
	shld		ecx,ebx,3
	?debug L 187
	shl		ebx,3
	?debug L 190
	xor		esi,esi
	?debug L 191
	test		ecx,ecx
	?debug L 192
	je        short @20
	?debug L 193
	dec		ecx
	?debug L 194
	inc		esi
	?debug L 195
	shld		esi,ebx,cl
	?debug L 196
	shl		ebx,cl
	?debug L 197
	add		edx,ecx
	?debug L 198
@20:
M_3:
	?debug L 199
	mov		ecx,dword ptr $mmckbaka
	?debug L 200
	shl		ecx,7
	?debug L 201
	add		esi,ecx
	?debug L 203
@19:
LZ:
	?debug L 205
	mov		ecx,esi
	?debug L 206
	shr		ecx,7
	?debug L 207
	mov		dword ptr $mmckbaka,ecx
	?debug L 209
	add		eax,3
	?debug L 210
	sub		esi,6
	?debug L 211
	add		esi,eax
	?debug L 213
	mov		ecx,eax
	?debug L 214
	sub		esi,edi
	?debug L 215
	neg		esi
	?debug L 217
	add		ecx,3
	?debug L 218
	push		eax
	?debug L 219
	shr		ecx,2
	?debug L 220
@21:
_movsd:
	?debug L 221
	mov		eax,[esi]
	?debug L 222
	mov		[edi],eax
	?debug L 223
	add		esi,4
	?debug L 224
	add		edi,4
	?debug L 225
	dec		ecx
	?debug L 226
	jne       @21
	?debug L 228
	pop		eax
	?debug L 229
	mov		ecx,eax
	?debug L 230
	add		ecx,3
	?debug L 231
	add		edi,eax
	?debug L 232
	and		ecx,0FFFFFFFCH
	?debug L 233
	sub		edi,ecx
	?debug L 235
	sub		dword ptr $eadkbaka,eax
	?debug L 236
	jne       @10
	?debug L 237
	jmp       @22
	?debug L 239
@18:
Rle:
	?debug L 241
	add		eax,1
	?debug L 242
	mov		ecx,eax
	?debug L 244
	push		eax
	?debug L 245
	inc		ecx
	?debug L 246
	mov		al,[edi-1]
	?debug L 247
	shr		ecx,1
	?debug L 248
	mov		ah,al
	?debug L 249
@23:
_stosw:
	?debug L 250
	mov		[edi],eax
	?debug L 251
	inc		edi
	?debug L 252
	inc		edi
	?debug L 253
	dec		ecx
	?debug L 254
	jne       @23
	?debug L 256
	pop		eax
	?debug L 257
	mov		ecx,eax
	?debug L 258
	and		ecx,1
	?debug L 259
	sub		edi,ecx
	?debug L 261
	sub		dword ptr $eadkbaka,eax
	?debug L 262
	jne       @10
	?debug L 263
	jmp       @22
	?debug L 265
@12:
RawByte:
	?debug L 267
	xor		eax,eax
	?debug L 268
	mov		ecx,dword ptr $mfckbaka
	?debug L 269
	shld		eax,ebx,cl
	?debug L 270
	add		edx,ecx
	?debug L 271
	shl		ebx,cl
	?debug L 272
	add		eax,dword ptr $ejckbaka
	?debug L 273
	mov		esi,dword ptr $mkdkbaka
	?debug L 274
	test		esi,esi
	?debug L 275
	je        short @24
	?debug L 276
	add		esi,eax
	?debug L 277
	mov		al,[esi]
	?debug L 278
@24:
_putRaw:
	?debug L 279
	mov		[edi],al
	?debug L 280
	inc		edi
	?debug L 282
	dec		dword ptr $eadkbaka
	?debug L 283
	jne       @10
	?debug L 284
	jmp       @22
	?debug L 286
@16:
RawSwitch:
	?debug L 288
	mov		ecx,4
	?debug L 289
	add		edx,ecx
	?debug L 290
	mov		eax,ebx
	?debug L 291
	shl		ebx,cl
	?debug L 292
	sub		ecx,32
	?debug L 293
	neg		ecx
	?debug L 294
	shr		eax,cl
	?debug L 295
	mov		ecx,eax
	?debug L 298
	xor		eax,eax
	?debug L 299
	test		ecx,ecx
	?debug L 300
	je        short @25
	?debug L 301
	dec		ecx
	?debug L 302
	inc		eax
	?debug L 303
	shld		eax,ebx,cl
	?debug L 304
	shl		ebx,cl
	?debug L 305
	add		edx,ecx
	?debug L 306
@25:
M_4:
	?debug L 308
	push		edx
	?debug L 309
	xor		edx,edx
	?debug L 310
	mov		ecx,9
	?debug L 311
	div		ecx
	?debug L 312
	mov		dword ptr $ejckbaka,eax
	?debug L 313
	mov		dword ptr $mfckbaka,edx
	?debug L 314
	pop		edx
	?debug L 315
	jmp       @10
	?debug L 318
@15:
CntSwitch:
	?debug L 319
	mov		eax,esi
	?debug L 320
	sub		eax,2
	?debug L 321
	lea		esi,dword ptr $eodkbaka
	?debug L 322
	mov		eax,[esi+eax*4]
	?debug L 323
	mov		byte ptr dword ptr $mobkbaka,ah
	?debug L 324
	mov		byte ptr dword ptr $ecckbaka,al
	?debug L 325
	jmp       @10
	?debug L 327
@22:
Exit:
	?debug L 328
	pop		ebp
	?debug L 330
	mov       al,1
	?debug L 331
@27:
@26:
	pop       edi
	pop       esi
	pop       ebx
	pop       ecx
	pop       ecx
	pop       ebp
	ret 
	?debug L 0
@@FLZDecompressMemory$qpuct1ii	endp
@FLZDecompressMemory$qpuct1ii	ends
_TEXT	ends
$$BSYMS	segment byte public use32 'DEBSYM'
	db	2
	db	0
	db	0
	db	0
	dw	76
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
	df	@@FLZDecompressMemory$qpuct1ii
	dw	0
	dw	4096
	dw	0
	dw	1
	dw	0
	dw	0
	dw	0
	db	29
	db	64
	db	70
	db	76
	db	90
	db	68
	db	101
	db	99
	db	111
	db	109
	db	112
	db	114
	db	101
	db	115
	db	115
	db	77
	db	101
	db	109
	db	111
	db	114
	db	121
	db	36
	db	113
	db	112
	db	117
	db	99
	db	116
	db	49
	db	105
	db	105
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
	dw	1056
	dw	0
	dw	3
	dw	0
	dw	0
	dw	0
	dw	18
	dw	512
	dw	16
	dw	0
	dw	116
	dw	0
	dw	4
	dw	0
	dw	0
	dw	0
	dw	18
	dw	512
	dw	20
	dw	0
	dw	116
	dw	0
	dw	5
	dw	0
	dw	0
	dw	0
	dw	18
	dw	512
	dw	65535
	dw	65535
	dw	32
	dw	0
	dw	6
	dw	0
	dw	0
	dw	0
	dw	22
	dw	513
	df	$eodkbaka
	dw	0
	dw	4098
	dw	0
	dw	7
	dw	0
	dw	0
	dw	0
	dw	22
	dw	513
	df	$mkdkbaka
	dw	0
	dw	1056
	dw	0
	dw	8
	dw	0
	dw	0
	dw	0
	dw	22
	dw	513
	df	$ehdkbaka
	dw	0
	dw	1056
	dw	0
	dw	9
	dw	0
	dw	0
	dw	0
	dw	22
	dw	513
	df	$mddkbaka
	dw	0
	dw	1056
	dw	0
	dw	10
	dw	0
	dw	0
	dw	0
	dw	22
	dw	513
	df	$eadkbaka
	dw	0
	dw	116
	dw	0
	dw	11
	dw	0
	dw	0
	dw	0
	dw	22
	dw	513
	df	$mmckbaka
	dw	0
	dw	116
	dw	0
	dw	12
	dw	0
	dw	0
	dw	0
	dw	22
	dw	513
	df	$ejckbaka
	dw	0
	dw	116
	dw	0
	dw	13
	dw	0
	dw	0
	dw	0
	dw	22
	dw	513
	df	$mfckbaka
	dw	0
	dw	116
	dw	0
	dw	14
	dw	0
	dw	0
	dw	0
	dw	22
	dw	513
	df	$ecckbaka
	dw	0
	dw	116
	dw	0
	dw	15
	dw	0
	dw	0
	dw	0
	dw	22
	dw	513
	df	$mobkbaka
	dw	0
	dw	116
	dw	0
	dw	16
	dw	0
	dw	0
	dw	0
	dw	24
	dw	519
	dw	0
	dw	0
	dw	0
	dw	0
	dd	?patch4
	df	@3
	dw	0
	dw	0
	dw	18
	dw	512
	dw	65528
	dw	65535
	dw	116
	dw	0
	dw	17
	dw	0
	dw	0
	dw	0
?patch4	equ	@4-@3
	dw	2
	dw	6
?patch1	equ	@27-@@FLZDecompressMemory$qpuct1ii+7
?patch2	equ	0
?patch3	equ	@27-@@FLZDecompressMemory$qpuct1ii
	dw	2
	dw	6
	dw	8
	dw	531
	dw	7
	dw	65516
	dw	65535
$$BSYMS	ends
_TEXT	segment dword public use32 'CODE'
_TEXT	ends
$$BSYMS	segment byte public use32 'DEBSYM'
	dw	?patch5
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
?patch5	equ	18
$$BSYMS	ends
$$BTYPES	segment byte public use32 'DEBTYP'
	db        2,0,0,0,14,0,8,0,48,0,0,0,0,0,4,0
	db        1,16,0,0,20,0,1,2,4,0,32,4,0,0,32,4
	db        0,0,116,0,0,0,116,0,0,0,18,0,3,0,117,0
	db        0,0,17,0,0,0,0,0,0,0,12,0,3,0
$$BTYPES	ends
$$BNAMES	segment byte public use32 'DEBNAM'
	db	19,'FLZDecompressMemory'
	db	7,'_InData'
	db	8,'_OutData'
	db	6,'_InLen'
	db	7,'_OutLen'
	db	5,'Flags'
	db	6,'TabCnt'
	db	8,'Alphabet'
	db	6,'InData'
	db	7,'OutData'
	db	6,'OutLen'
	db	7,'BaseOff'
	db	10,'RawByteAdd'
	db	11,'RawByteBits'
	db	10,'ExpLenBits'
	db	10,'ExpOffBits'
	db	6,'Alphas'
$$BNAMES	ends
	?debug	D "D:\proj\Triangulator\TriangulatorDLL\FLZ\FLZCommon.h" 12415 38513
	?debug	D "D:\proj\Triangulator\TriangulatorDLL\FLZ\FLZDecoder.cpp" 12418 35218
	end
