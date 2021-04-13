provider sdt_noop {
	probe no_args();
	probe with_args(int arg1, int arg2, int arg3);
	probe with_global_arg(int arg1);
	probe with_volatile_arg(int arg1);
	probe with_many_args(int arg1, int arg2, int arg3, int64_t arg4, int64_t arg5, int64_t arg6, int64_t arg7, int64_t arg8);
	probe with_computed_arg(int arg1);
};
