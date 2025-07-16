# void matmul_i8_rvm(int8_t *a, int8_t *b, int32_t *c, int m, int n, int k)
# a0: a, a1: b, a2: c, a3: m, a4: n, a5: k
.section .text
.global matmul_i8_rvm
matmul_i8_rvm:
  addi sp, sp, -32
	sd ra, 8(sp)
  sd s0, 16(sp)
	sd s1, 24(sp)

  # set CSR tmdata (addr 0x803) to int8 (0x03)
  li s0, 0x030303
  csrw 0x803, s0
  # set CSR tmsize (addr 0x804) according to m, n, k
  andi s0, a3, 0x3ff
  andi s1, a4, 0x3ff
  sll s1, s1, 10
  or s0, s0, s1
  andi s1, a5, 0x3ff
  sll s1, s1, 20
  or s0, s0, s1
  csrw 0x804, s0

  # Insert 10 NOPs after csrw instruction
  addi x0, x0, 0  # NOP 1
  addi x0, x0, 0  # NOP 2
  addi x0, x0, 0  # NOP 3
  addi x0, x0, 0  # NOP 4
  addi x0, x0, 0  # NOP 5
  addi x0, x0, 0  # NOP 6
  addi x0, x0, 0  # NOP 7
  addi x0, x0, 0  # NOP 8
  addi x0, x0, 0  # NOP 9
  addi x0, x0, 0  # NOP 10

  # tm0 <- a; tm1 <- b; tm2 <- c
  # tmld: |31 ... 26|25|24 ... 20|19 ... 15|14 ... 12|11 ... 7|6 ... 0|
  #       | funct6  |vm|         |   rs1   | funct3  |   tmd  |0001011|

  .word 0x0005000B # tmlda tm0, (a0); 0b000000_0_00000_01010_000_00000_0001011
  .word 0x0805808B # tmldb tm1, (a1); 0b000010_0_00000_01011_000_00001_0001011
  .word 0x0c06010B # tmldc tm2, (a2); 0b000011_0_00000_01100_000_00010_0001011

  # c <- a * b + c
  # tmm: |31 ... 26|25|24 ... 20|19 ... 15|14 ... 12|11 ... 7|6 ... 0|
  #      |  000001 |  |   tms2  |   tms1  |   000   |   tmd  |0001011|
  .word 0x410010B # tmm tm2, tm1, tm0; 0b000001_0_00001_00000_000_00010_0001011

  # tm0 -> c
  # tmst: |31 ... 26|25|24 ... 20|19 ... 1|14 ... 12|11 ... 7|6 ... 0|
  #       |         |  |         |   rs1  |   000    |  tms1  |0001011|
  .word 0x1806010B # tmstc tm2, (a2); 0b000110_0_00000_01100_000_00010_0001011

  ld ra, 8(sp)
	ld s0, 16(sp)
	ld s1, 24(sp)
	addi sp, sp, 32

  ret
