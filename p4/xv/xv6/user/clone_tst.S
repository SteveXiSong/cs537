
clone_tst.o:     file format elf32-i386


Disassembly of section .text:

00000000 <do_sth>:
   0:	55                   	push   %ebp
   1:	89 e5                	mov    %esp,%ebp
   3:	83 ec 18             	sub    $0x18,%esp
   6:	8b 45 08             	mov    0x8(%ebp),%eax
   9:	8b 00                	mov    (%eax),%eax
   b:	89 44 24 08          	mov    %eax,0x8(%esp)
   f:	c7 44 24 04 00 00 00 	movl   $0x0,0x4(%esp)
  16:	00 
  17:	c7 04 24 01 00 00 00 	movl   $0x1,(%esp)
  1e:	e8 fc ff ff ff       	call   1f <do_sth+0x1f>
  23:	c9                   	leave  
  24:	c3                   	ret    

00000025 <main>:
  25:	55                   	push   %ebp
  26:	89 e5                	mov    %esp,%ebp
  28:	83 e4 f0             	and    $0xfffffff0,%esp
  2b:	83 ec 20             	sub    $0x20,%esp
  2e:	c7 44 24 10 ff ff 00 	movl   $0xffff,0x10(%esp)
  35:	00 
  36:	c7 44 24 14 64 00 00 	movl   $0x64,0x14(%esp)
  3d:	00 
  3e:	8d 44 24 14          	lea    0x14(%esp),%eax
  42:	89 04 24             	mov    %eax,(%esp)
  45:	e8 fc ff ff ff       	call   46 <main+0x21>
  4a:	c7 44 24 18 00 00 00 	movl   $0x0,0x18(%esp)
  51:	00 
  52:	8b 44 24 18          	mov    0x18(%esp),%eax
  56:	89 44 24 08          	mov    %eax,0x8(%esp)
  5a:	c7 44 24 04 0e 00 00 	movl   $0xe,0x4(%esp)
  61:	00 
  62:	c7 04 24 01 00 00 00 	movl   $0x1,(%esp)
  69:	e8 fc ff ff ff       	call   6a <main+0x45>
  6e:	c7 44 24 14 c8 00 00 	movl   $0xc8,0x14(%esp)
  75:	00 
  76:	8d 44 24 10          	lea    0x10(%esp),%eax
  7a:	89 44 24 08          	mov    %eax,0x8(%esp)
  7e:	8d 44 24 14          	lea    0x14(%esp),%eax
  82:	89 44 24 04          	mov    %eax,0x4(%esp)
  86:	c7 04 24 00 00 00 00 	movl   $0x0,(%esp)
  8d:	e8 fc ff ff ff       	call   8e <main+0x69>
  92:	89 44 24 1c          	mov    %eax,0x1c(%esp)
  96:	83 7c 24 1c 00       	cmpl   $0x0,0x1c(%esp)
  9b:	75 2d                	jne    ca <main+0xa5>
  9d:	8b 44 24 14          	mov    0x14(%esp),%eax
  a1:	83 c0 01             	add    $0x1,%eax
  a4:	89 44 24 14          	mov    %eax,0x14(%esp)
  a8:	8d 44 24 14          	lea    0x14(%esp),%eax
  ac:	89 04 24             	mov    %eax,(%esp)
  af:	e8 fc ff ff ff       	call   b0 <main+0x8b>
  b4:	c7 44 24 04 20 00 00 	movl   $0x20,0x4(%esp)
  bb:	00 
  bc:	c7 04 24 01 00 00 00 	movl   $0x1,(%esp)
  c3:	e8 fc ff ff ff       	call   c4 <main+0x9f>
  c8:	eb 2b                	jmp    f5 <main+0xd0>
  ca:	8b 44 24 14          	mov    0x14(%esp),%eax
  ce:	83 c0 01             	add    $0x1,%eax
  d1:	89 44 24 14          	mov    %eax,0x14(%esp)
  d5:	8d 44 24 14          	lea    0x14(%esp),%eax
  d9:	89 04 24             	mov    %eax,(%esp)
  dc:	e8 fc ff ff ff       	call   dd <main+0xb8>
  e1:	c7 44 24 04 27 00 00 	movl   $0x27,0x4(%esp)
  e8:	00 
  e9:	c7 04 24 01 00 00 00 	movl   $0x1,(%esp)
  f0:	e8 fc ff ff ff       	call   f1 <main+0xcc>
  f5:	e8 fc ff ff ff       	call   f6 <main+0xd1>
  fa:	e8 fc ff ff ff       	call   fb <main+0xd6>
