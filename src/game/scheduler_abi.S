# PS2-NANA-decomp — custom-entry stubs for scheduler functions.
# STATUS: PARTIAL / TARGET-VERIFICATION PENDING
#
# Task__QueuePending originally receives p5, callback and callback_arg in
# $t0-$t2. This stub forwards those registers to the portable C helper using
# the stack locations expected by a seven-argument C function.
    .set    noreorder
    .section .text
    .globl  Task__QueuePending
    .ent    Task__QueuePending
Task__QueuePending:
    addiu   $sp, $sp, -16
    sw      $t0, 0($sp)
    sw      $t1, 4($sp)
    sw      $t2, 8($sp)
    jal     Task__QueuePendingImpl
    nop
    addiu   $sp, $sp, 16
    jr      $31
    nop
    .end    Task__QueuePending