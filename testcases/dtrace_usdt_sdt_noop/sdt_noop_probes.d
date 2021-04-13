provider sdt_noop {
	probe no_args();
	probe with_args(int, int, int);
	probe with_global_arg(int);
	probe with_volatile_arg(int);
	probe with_many_args(int, int, int, int64_t, int64_t, int64_t, int64_t, int64_t);
	probe with_computed_arg(int);
};
