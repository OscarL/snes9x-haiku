# copyright Caz (2002)

.globl normal_blit
normal_blit:
pushl %ebp
movl %esp, %ebp
pushal
movl 8(%ebp), %esi
movl 12(%ebp), %edi
movl 16(%ebp), %ecx
shrl $2, %ecx
xorl %eax, %eax

.align 16
next:
movq (%esi, %eax, 8), %mm0
movq %mm0, (%edi, %eax, 8)
incl %eax
cmpl %eax, %ecx
ja next

popal
movl %ebp, %esp
popl %ebp
ret

.globl double_blit
double_blit:
pushl %ebp
movl %esp, %ebp
pushal
movl 8(%ebp), %esi
movl 12(%ebp), %edi
movl 16(%ebp), %ecx
shrl $3, %ecx
xorl %eax, %eax

.align 16
next_double:
movq (%esi), %mm0
movq %mm0, %mm1
punpcklwd %mm0, %mm0
punpckhwd %mm1, %mm1
movq %mm0, (%edi)
movq %mm1, 8(%edi)
addl $8, %esi
addl $16, %edi
decl %ecx
jnz next_double

popal
movl %ebp, %esp
popl %ebp
ret

.globl lcd_blit_standard
lcd_blit_standard:
pushl %ebp
movl %esp, %ebp
pushal
movl 8(%ebp), %esi
movl 12(%ebp), %edi
movl 16(%ebp), %ecx
shrl $2, %ecx
xorl %eax, %eax

.align 16
next_lcd_standard:
movq (%esi, %eax, 4), %mm0
punpcklwd %mm0, %mm0
pslld $16, %mm0
movq %mm0, (%edi, %eax, 8)
incl %eax
cmpl %eax, %ecx
ja next_lcd_standard

popal
movl %ebp, %esp
popl %ebp
ret

.globl lcd_blit_extend
lcd_blit_extend:
pushl %ebp
movl %esp, %ebp
pushal
movl 8(%ebp), %esi
movl 12(%ebp), %edi
movl 16(%ebp), %ecx
shrl $2, %ecx
xorl %eax, %eax

.align 16
next_lcd_extend:
movq (%esi, %eax, 8), %mm0
pslld $16, %mm0
movq %mm0, (%edi, %eax, 8)
incl %eax
cmpl %eax, %ecx
ja next_lcd_extend

popal
movl %ebp, %esp
popl %ebp
ret
