#ifndef macros_h
#define macros_h

/* Array */
#define macros_array_count(Array) (sizeof(Array) / sizeof((Array)[0]))


/* Amount */
#define macros_amount_kilobytes(Value) ((Value)*1024LL)
#define macros_amount_megabytes(Value) (macros_amount_kilobytes(Value)*1024LL)
#define macros_amount_gigabytes(Value) (macros_amount_megabytes(Value)*1024LL)
#define macros_amount_terabytes(Value) (macros_amount_gigabytes(Value)*1024LL)
#define macros_align_pow2(Value, Alignment) (((Value) + ((Alignment) - 1)) & ~(((Value) - (Value)) + (Alignment) - 1))
#define macros_align_4(Value) (((Value) + 3) & ~3)
#define macros_align_8(Value) (((Value) + 7) & ~7)
#define macros_algin_16(Value) (((Value) + 15) & ~15)

#endif
