; copyright Caz (2002) rgb12[4:4:4] to rgb16[5:6:5]

global dolinergb12
REDS dd 0x000f000f, 0x000f000f
GREENS dd 0x00f000f0, 0x00f000f0
BLUES dd 0x0f000f00, 0x0f000f00
MASK dd 0x00f800f8, 0x00f800f8

dolinergb12:

push ebp
mov ebp, esp
pushad

mov esi, [ebp+8]
mov edi, [ebp+12]
mov ecx, [ebp+16]
shr ecx, 2
xor eax, eax

next:
movq mm0, [esi+eax*8]
movq mm1, mm0

pand mm0, [REDS]
pslld mm0, 12

movq mm2, mm1

pand mm1, [GREENS]
pslld mm1, 3

por mm0, mm1

pand mm2, [BLUES]
psrld mm2, 7
				  
por mm0, mm2
movq [edi+eax*8], mm0

inc eax
cmp ecx, eax
ja next
;emms call emms after all lines have been done
popad

mov esp, ebp
pop ebp
ret