inline
BOOL is_induced (gc_reason reason)
{
	return ((reason == reason_induced) ||
			(reason == reason_induced_noforce) ||
			(reason == reason_lowmemory) ||
			(reason == reason_lowmemory_blocking) ||
			(reason == reason_induced_compacting));
}