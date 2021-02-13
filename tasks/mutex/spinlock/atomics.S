#if(APPLE)
  #define FUNCTION_NAME(name) _##name
#else
  #define FUNCTION_NAME(name) name
#endif

.globl FUNCTION_NAME(AtomicLoad)
.globl FUNCTION_NAME(AtomicStore)
.globl FUNCTION_NAME(AtomicExchange)

# Solution starts here

FUNCTION_NAME(AtomicLoad):
    # Your asm code goes here
    movq (%rdi), %rax
    retq

FUNCTION_NAME(AtomicStore):
    # Your asm code goes here
    movq %rsi, (%rdi)
    retq

FUNCTION_NAME(AtomicExchange):
    # Your asm code goes here
    movq (%rdi), %rax  # Load
    movq %rsi, (%rdi)  # Store
    retq
