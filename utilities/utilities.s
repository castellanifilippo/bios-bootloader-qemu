/* ************* *
 *      OUT      * 
 * ************* */

/*====== 1 Byte ======*/
.globl outb
.type outb, @function

outb:
  push %ebp
  mov %esp, %ebp
  mov 8(%ebp),  %al
  mov 12(%ebp), %dx
  outb %al, %dx
  pop %ebp
  ret

/*====== 2 Byte ======*/
.globl outw
.type outw, @function

outw:
  push %ebp
  mov  %esp, %ebp
  movw 8(%ebp), %ax
  mov  12(%ebp), %dx
  outw %ax, %dx
  pop  %ebp
  ret

/*====== 4 Byte ======*/
.globl outl
.type  outl, @function

outl:
  push   %ebp
  mov    %esp, %ebp
  mov    8(%ebp), %eax
  mov    12(%ebp), %dx
  outl   %eax, %dx
  pop    %ebp
  ret

/* ************* *
 *      IN       *
 * ************* */

/*====== 1 Byte ======*/
.globl inb
.type inb, @function

inb:
  push %ebp
  mov %esp, %ebp
  xor %eax, %eax
  mov 8(%ebp), %dx
  inb %dx, %al
  pop %ebp
  ret

/*====== 2 Byte ======*/
.globl inw
.type inw, @function

inw:
    push %ebp
    mov  %esp, %ebp
    xor  %eax, %eax
    mov  8(%ebp), %dx
    inw  %dx, %ax
    pop  %ebp
    ret

/*====== 4 Byte ======*/
.globl inl
.type inl, @function

inl:
    push %ebp
    mov  %esp, %ebp
    xor  %eax, %eax
    mov  8(%ebp), %dx
    inl  %dx, %eax
    pop  %ebp
    ret

