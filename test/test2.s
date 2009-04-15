# FUNC PTR|PROC|INT
   # NAME PTR|PROC|INT name="main", scope = 2, offset=0
   # LIST 
      # CNST INT value=0
      # CNST INT value=4
	.text
	.globl	main
	.type	main, @function
main:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$12,%esp

# LABEL  label=1
.L1:

# = INT
   # NAME INT name="i", scope = 0, offset=4
   # CNST INT value=10
	leal	-4(%ebp),%eax
	pushl	%eax
	pushl	$10
	popl	%eax
	popl	%edx
	movl	%eax,(%edx)
	pushl	%eax
	popl	%eax

# LABEL  label=2
.L2:

# CALL INT
   # NAME PTR|PROC|INT name="printf", scope = 0, offset=0
   # LIST 
      # STRG PTR| value=i is %d\n
      # @ INT
         # NAME INT name="i", scope = 0, offset=4
	leal	-4(%ebp),%eax
	pushl	%eax
	popl	%eax
	movl	(%eax),%edx
	pushl	%edx
	.data
L01:	.asciz	"i is %d\n"
	.text
	leal	L01,%eax
	pushl	%eax
	call	printf
	addl	$8,%esp
	pushl	%eax

# LABEL  label=3
.L3:

# = INT
   # NAME INT name="i", scope = 0, offset=4
   # + INT
      # @ INT
         # NAME INT name="i", scope = 0, offset=4
      # CNST INT value=3
	leal	-4(%ebp),%eax
	pushl	%eax
	leal	-4(%ebp),%eax
	pushl	%eax
	popl	%eax
	movl	(%eax),%edx
	pushl	%edx
	pushl	$3
	popl	%eax
	popl	%edx
	addl	%edx,%eax
	pushl	%eax
	popl	%eax
	popl	%edx
	movl	%eax,(%edx)
	pushl	%eax
	popl	%eax

# LABEL  label=4
.L4:

# CALL INT
   # NAME PTR|PROC|INT name="foo", scope = 0, offset=0
   # @ INT
      # NAME INT name="i", scope = 0, offset=4
	leal	-4(%ebp),%eax
	pushl	%eax
	popl	%eax
	movl	(%eax),%edx
	pushl	%edx
	call	foo
	addl	$8,%esp
	pushl	%eax

# LABEL  label=5
.L5:

# RET INT
   # CNST INT value=0
	pushl	$0
	popl	%eax
	movl	%ebp,%esp
	popl	%ebp
	ret

# FEND PTR|PROC|INT
   # NAME PTR|PROC|INT name="main", scope = 2, offset=0
	movl	%ebp,%esp
	popl	%ebp
	ret

# FUNC PTR|PROC|INT
   # NAME PTR|PROC|INT name="foo", scope = 2, offset=0
   # LIST 
      # CNST INT value=0
      # CNST INT value=4
	.text
	.globl	foo
	.type	foo, @function
foo:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$12,%esp

# LABEL  label=6
.L6:

# CALL INT
   # NAME PTR|PROC|INT name="printf", scope = 0, offset=0
   # LIST 
      # STRG PTR| value=j is %d\n
      # @ INT
         # NAME INT name="j", scope = 0, offset=4
	leal	-4(%ebp),%eax
	pushl	%eax
	popl	%eax
	movl	(%eax),%edx
	pushl	%edx
	.data
L02:	.asciz	"j is %d\n"
	.text
	leal	L02,%eax
	pushl	%eax
	call	printf
	addl	$8,%esp
	pushl	%eax

# LABEL  label=7
.L7:

# RET INT
   # CNST INT value=0
	pushl	$0
	popl	%eax
	movl	%ebp,%esp
	popl	%ebp
	ret

# FEND PTR|PROC|INT
   # NAME PTR|PROC|INT name="foo", scope = 2, offset=0
	movl	%ebp,%esp
	popl	%ebp
	ret

